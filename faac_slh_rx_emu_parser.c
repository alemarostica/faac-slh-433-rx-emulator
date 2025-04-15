#include "faac_slh_rx_emu_parser.h"
#include "faac_slh_rx_emu_utils.h"

/*
NOTES:
 -  I have difficulty testing the case where the seed is 0x0 and/or fix = 0x0
    The flipper locks up when I send the normal key
    I think it is because of the allow_zero_seed var in lib/subghz/protocols/faac_slh.c but I'm not sure
 -  Explanation for the editing of the Unleased firmware
     -  Suppose you have a memorized remote because the prog key was read
        A normal key will be correctly decoded, count included
        If a prog key from another remote is received the receiver will be set to decode based on the new seed received
        The original remote will not be parsed correctly anymore
        I imagine this is the intended implementation of the protocol but it does not work with this app
        Must edit the firmware as specified in README
 -  The remote sometimes sends a weird seed:
     -  If the normal seed is XXXXYYYY, sometimes YYYYXXXX is sent, no idea why
     -  Also the hop value sent by the remote after does not coincide with the sent seed
 -  Some behaviours of the receiver:
     -  If the counter in the received key is <= internal count + 0x20 then it opens
     -  If the counter in the received key is <= internal count + 0x8000 it must resync
     -  Any other value is considered past
     -  To resync the receiver must receive two sequential keys, so if count in key1 = x, count in key2 must be x + 1
     -  The second received key may be outside the internal count + 0x8000 range
     -  The internal count is not updated unless the received key is valid or the internal count is resynced
     -  If a future key is received but the next is in range the gate is opened
     -  If a future key is received the receiver will check if a past key is immediately preceding the one just received and resync if so
         -  I could not determine the number of past keys kept in memory
            Sometimes it seemed it only kept 5 while other times up to 8
            At most, based on lenghty testing, it seemed to keep a maximum of 8 past keys so I decided to use this value for the history
         - This history keeps also keys with a different fix from the memorized one
     -  The programming phase behaves similarly to the resync phase but checks at most 5 past keys
     -  The programming phase is selective of the fix value, meaning if the second key received has a different 
        fix than the first the count check will not happen BUT it still will be kept in the history
     -  If the fix value corresponds between first and second key received then the count check will happen but 
        if an invalid count is received it will now become the leading one meaning the receiver will wait for a sequential key based on this count former cunt value.
     -  I don't know is the internal counter is advanced when receiving a prog key
*/

#define DECODE_DEBOUNCE_MS           500
#define MAX_FUTURE_COUNT_OPEN        0x20
#define MAX_FUTURE_COUNT_RESYNC      0x8000
#define HISTORY_MIN_INDEX(key_index) ((key_index) > 4 ? (key_index) - 4 : 0)

uint32_t last_decode = 0;
static int key_index = 0;

/**
 * @brief   Debounce the signal.
 * @details This function checks if the last signal arrived within 500ms of the preceding signal.
 * @param   now The time.
 * @return  True if time between keys received is less than 500ms, false otherwise.
*/
bool debounce_decode(uint32_t now) {
    if(now - last_decode < furi_ms_to_ticks(DECODE_DEBOUNCE_MS)) {
        FURI_LOG_D(TAG, "Ignoring decode. Too soon.");
        last_decode = now;
        return true;
    }
    last_decode = now;
    return false;
}

/**
 * @brief   Check circular range.
 * @details This function checks if a value is in a circular range with max value 0xFFFFF.
 * @param   value   The value.
 * @param   start   The minimum value.
 * @param   end     The maximum value.
 * @return  true if in range, false otherwise.
*/
bool is_within_range(uint32_t value, uint32_t start, uint32_t end) {
    uint32_t c_end = end & 0xFFFFF;

    if(start <= c_end) {
        return (value >= start && value <= end);
    } else {
        return (value >= start || value <= end);
    }
}

void clear_history(FaacSLHRxEmuInteral** keys) {
    // Invoked when a remote is succesfully programmed, only the last received key will remain in the history
    keys[0] = keys[key_index];
    for(int i = 1; i <= key_index; i++) {
        keys[i]->count = 0x0;
        keys[i]->fix = 0x0;
    }
    key_index = 1;
}

void parse_faac_slh_normal(void* context, FuriString* buffer) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    FaacSLHRxEmuModelNormal* model = app->model_normal;
    FURI_LOG_T(TAG, "Decoding FAAC SLH...");

    if(debounce_decode(furi_get_tick())) return;

    furi_string_printf(
        app->last_transmission, "Last transmission:\n%s", furi_string_get_cstr(buffer));

    if(furi_string_search_str(buffer, "Master") != FURI_STRING_FAILURE) {
        furi_string_printf(app->model_normal->info, "Prog key, ignoring");
        __gui_redraw();
        return;
    }

    __furi_string_extract_string(buffer, 0, "Key:", '\r', app->model_normal->key);

    app->model_normal->hop = __furi_string_extract_int(buffer, "Hop:", ' ', FAILED_TO_PARSE);

    if(app->mem_status == FaacSLHRxEmuMemStatusFull ||
       model->status == FaacSLHRxEmuNormalStatusSyncFirst ||
       model->status == FaacSLHRxEmuNormalStatusSyncSecond) {
        // Memory is full OR we need to memorize new remote
        model->fix = __furi_string_extract_int(buffer, "Fix:", ' ', FAILED_TO_PARSE);
        model->count = __furi_string_extract_int(buffer, "Cnt:", '\r', FAILED_TO_PARSE);

        if(model->fix == FAILED_TO_PARSE || model->count == FAILED_TO_PARSE) {
            furi_string_printf(model->info, "Failed to parse, retry");
            __gui_redraw();
            return;
        }

        app->keys[key_index]->fix = model->fix;
        app->keys[key_index]->count = model->count;

        if(model->status == FaacSLHRxEmuNormalStatusNone) {
            // We are reading normally
            if(model->fix == app->mem_remote->fix) {
                if(model->count == app->mem_remote->count) {
                    furi_string_printf(model->info, "Replay attack");
                } else if(is_within_range(
                              model->count,
                              app->mem_remote->count + 1,
                              app->mem_remote->count + MAX_FUTURE_COUNT_OPEN)) {
                    app->mem_remote->count = model->count;
                    furi_string_printf(model->info, "Opened");
                } else if(is_within_range(
                              model->count,
                              app->mem_remote->count + MAX_FUTURE_COUNT_OPEN + 1,
                              app->mem_remote->count + MAX_FUTURE_COUNT_RESYNC)) {
                    furi_string_printf(model->info, "Key is future, resync");
                    model->status = FaacSLHRxEmuNormalStatusSyncNormal;
                } else {
                    furi_string_printf(model->info, "Key is past");
                }
            } else {
                furi_string_printf(model->info, "Unrecognized remote");
            }
        } else if(model->status == FaacSLHRxEmuNormalStatusSyncFirst) {
            // We are reading the first key after a prog key
            model->status = FaacSLHRxEmuNormalStatusSyncSecond;
            furi_string_printf(model->info, "OK, first");
        } else if(model->status == FaacSLHRxEmuNormalStatusSyncSecond) {
            // We are reading the second key after a prog key
            int min = HISTORY_MIN_INDEX(key_index);
            for(int i = key_index - 1; i >= min; i--) {
                if(model->fix == app->keys[i]->fix) {
                    if(model->count == app->keys[i]->count + 1) {
                        furi_string_printf(model->info, "Opened, programmed");
                        app->mem_remote->fix = model->fix;
                        app->mem_remote->count = model->count;
                        model->status = FaacSLHRxEmuNormalStatusNone;
                        app->mem_status = FaacSLHRxEmuMemStatusFull;
                        app->mem_seed = app->model_prog->seed;
                        clear_history(app->keys);
                        break;
                    } else {
                        furi_string_printf(model->info, "Non sequential keys");
                        break;
                    }
                } else {
                    furi_string_printf(model->info, "Unrecognized remote");
                }
            }
        }
        if(model->status == FaacSLHRxEmuNormalStatusSyncNormal) {
            // We are resyncing
            // This is not an else if because it can resync at any time the are two sequntial keys in the history
            if(model->fix == app->mem_remote->fix) {
                if(is_within_range(
                       model->count,
                       app->mem_remote->count + MAX_FUTURE_COUNT_OPEN + 1,
                       app->mem_remote->count + MAX_FUTURE_COUNT_RESYNC + 1)) {
                    for(int i = key_index - 1; i > 0; i--) {
                        if(model->fix == app->keys[i]->fix &&
                           model->count == ((app->keys[i]->count + 1) & 0xFFFFF)) {
                            furi_string_printf(model->info, "Opened, resynced");
                            model->status = FaacSLHRxEmuNormalStatusNone;
                            app->mem_remote->count = model->count;
                            break;
                        }
                    }
                } else if(is_within_range(
                              model->count,
                              app->mem_remote->count + 1,
                              app->mem_remote->count + MAX_FUTURE_COUNT_OPEN)) {
                    furi_string_printf(model->info, "Opened, in order");
                    model->status = FaacSLHRxEmuNormalStatusNone;
                    app->mem_remote->count = model->count;
                } else {
                    furi_string_printf(model->info, "Key is past");
                    model->status = FaacSLHRxEmuNormalStatusNone;
                }
            }
        }

        // Advance the key index
        if(key_index >= HISTORY_SIZE - 1) {
            for(int i = 1; i < HISTORY_SIZE; i++) {
                memmove(app->keys[i - 1], app->keys[i], sizeof(FaacSLHRxEmuInteral));
            }
            key_index = HISTORY_SIZE - 1;
            app->keys[HISTORY_SIZE - 1]->fix = 0x0;
            app->keys[HISTORY_SIZE - 1]->count = 0x0;
        } else {
            key_index += 1;
        }

    } else {
        // Memory is empty

        // Even though memory is empty a prog key might have been received so the receiver might have been set to decode the count also
        // So we must account for both cases
        if(furi_string_search_str(buffer, "Cnt:") == FURI_STRING_FAILURE) {
            app->model_normal->fix =
                __furi_string_extract_int(buffer, "Fix:", '\r', FAILED_TO_PARSE);
        } else {
            app->model_normal->fix =
                __furi_string_extract_int(buffer, "Fix:", ' ', FAILED_TO_PARSE);
        }

        app->model_normal->count = FAILED_TO_PARSE;
        furi_string_printf(app->model_normal->info, "No remote memorized");
    }

    // This should not be necessary, but I do not trust myself
    if(app->mem_remote->count > 0xFFFFF) app->mem_remote->count &= 0xFFFFF;

    __gui_redraw();
}

void parse_faac_slh_prog(void* context, FuriString* buffer) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    FaacSLHRxEmuModelProg* model = app->model_prog;
    FURI_LOG_T(TAG, "Decoding FAAC SLH...");

    if(debounce_decode(furi_get_tick())) return;

    furi_string_printf(
        app->last_transmission, "Last transmission:\n%s", furi_string_get_cstr(buffer));

    // Check in which phase of the programming we are
    if(model->status == FaacSLHRxEmuProgStatusWaitingForProg) {
        // Check if the received key is prog mode, return if not
        if(furi_string_search_str(buffer, "Master") == FURI_STRING_FAILURE) {
            furi_string_printf(model->info, "Normal key, ignoring");
            __gui_redraw();
            return;
        }
        // Extract data
        __furi_string_extract_string(buffer, 0, "Ke:", '\r', model->key);
        model->seed = __furi_string_extract_int(buffer, "Seed:", ' ', FAILED_TO_PARSE);
        model->mCnt = __furi_string_extract_int(buffer, "mCnt:", '\0', FAILED_TO_PARSE);
        // Check if seed was parsed succesfully, return without changing status if not
        if(model->seed == FAILED_TO_PARSE) {
            furi_string_printf(model->info, "Failed to parse seed");
            __gui_redraw();
            return;
        }
        // Prog key received succesfully
        model->status = FaacSLHRxEmuProgStatusLearned;
        app->model_normal->status = FaacSLHRxEmuNormalStatusSyncFirst;
        furi_string_printf(app->model_normal->info, "Syncing prog");
        furi_string_printf(model->info, "OK, Prog");
    } else if(model->status == FaacSLHRxEmuProgStatusLearned) {
        // At this stage a reomte has been memorized, nothing else received will be saved
        furi_string_printf(model->info, "Memory full");
        __gui_redraw();
        return;
    }

    __gui_redraw();
}
