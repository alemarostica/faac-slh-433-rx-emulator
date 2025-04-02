#include "faac_slh_rx_emu_parser.h"
#include "faac_slh_rx_emu_utils.h"

uint32_t last_decode = 0;
void parse_faac_slh_normal(void* context, FuriString* buffer) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    FURI_LOG_T(TAG, "Decoding FAAC SLH...");
    uint32_t now = furi_get_tick();
    if(now - last_decode < furi_ms_to_ticks(500)) {
        FURI_LOG_D(TAG, "Ignoring decode. Too soon.");
        last_decode = now;
        return;
    }
    last_decode = now;

    furi_string_printf(
        app->last_transmission, "Last transmission:\n%s", furi_string_get_cstr(buffer));

    __furi_string_extract_string(buffer, 0, "Key:", '\r', app->model_normal->key);

    if(furi_string_search_str(buffer, "Cnt:") == FURI_STRING_FAILURE) {
        app->model_normal->code_fix =
            __furi_string_extract_int(buffer, "Fix:", '\r', FAILED_TO_PARSE);
        app->model_normal->count = FAILED_TO_PARSE;
    } else {
        app->model_normal->code_fix =
            __furi_string_extract_int(buffer, "Fix:", ' ', FAILED_TO_PARSE);
        app->model_normal->count =
            __furi_string_extract_int(buffer, "Cnt:", '\r', FAILED_TO_PARSE);
    }
    app->model_normal->hop = __furi_string_extract_int(buffer, "Hop:", ' ', FAILED_TO_PARSE);

    if(app->model_normal->seed == 0x0) {
        furi_string_printf(app->model_normal->info, "No remote memorized");
    } else {
        furi_string_printf(app->model_normal->info, "OK");
    }

    // Si presenta un problema significativo:
    // Ipotizziamo che abbiamo un remote memorizzato perché è stata letta la prog key
    // La lettura normale sarà parsata correttamente incluso il count
    // Tuttavia se si riceve una prog key di un altro remote (o addirittura lo stesso quando invia quella versione strana flippata) il receiver sarà settato con il nuovo seed
    // Il count quindi adesso non sarà più parsato correttamente
    // Che facciamo?
    // Come faccio a fare in modo che se non è in prog mode ignori completamente una prog key?
    // Forse devo comprendere bene cosa succede con i file *_subghz
    // Most likely è il funzionamento intended dell'implementazione del protocollo, non ci posso fare più di tanto

    // SOLUZIONE TROVATA:
    // Bisogna modificare /lib/subghz/protocols/faac_slh.c così:
    // 26:  static bool already_programmed = false;
    // 464: already_programmed = false;
    // 594: if(!already_programmed) {
    // 595:     instance->seed = data_prg[5] << 24 | data_prg[4] << 16 | data_prg[3] << 8 |
    // 596:     data_prg[2];
    // 597:     already_programmed = true;
    // 698: }

    // Some behaviours:
    // se è massimo 0x20 count nel futuro apre
    // se è massimo 0x8000 count nel future deve resyncare
    // NOTA: per resyncare anche la seconda chiave ricevuta deve essere all'interno del futuro di 0x8000 count
    // Se un segnale è totalmente futuro il count non va avanti
    // se un segnale ricevuto è futuro ma quello dopo è accettabile si apre e basta

    __gui_redraw();
}

void parse_faac_slh_prog(void* context, FuriString* buffer) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    FaacSLHRxEmuModelProg* model = app->model_prog;
    FURI_LOG_T(TAG, "Decoding FAAC SLH...");
    uint32_t now = furi_get_tick();
    if(now - last_decode < furi_ms_to_ticks(500)) {
        FURI_LOG_D(TAG, "Ignoring decode. Too soon.");
        last_decode = now;
        return;
    }
    last_decode = now;

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
        model->status = FaacSLHRxEmuProgStatusWaitingForFirst;
        furi_string_printf(model->info, "OK Prog");
    } else if(model->status == FaacSLHRxEmuProgStatusWaitingForFirst) {
        // It could be argued that the receiver is simply in normal mode after having received the seed and just performs a resync to extract the count
        // The only differece with normal mode is that it appears to resync whatever the remote's count is so, in a way, it is behaving different from normal mode
        // It could be made so both resyncs always have to happen in normal mode, it would make it more accurate to the original and maybe spare some lines of code.
        // This would mean making a unified resync mode which takes a boolean (or a status) and resyncs in two different manners depending on wether we just learned a new seed or not.
        // Probabilmente renderebbe anche il codice meno complesso.
        // Ci penso domani mi sa

        // Check if the received key is normal, return without changing status if not
        if(furi_string_search_str(buffer, "Master") != FURI_STRING_FAILURE) {
            furi_string_printf(model->info, "Prog key, ignoring");
            __gui_redraw();
            return;
        }
        model->first_key->serial = __furi_string_extract_int(buffer, "Sn:", ' ', FAILED_TO_PARSE);
        model->first_key->count = __furi_string_extract_int(buffer, "Cnt:", '\r', FAILED_TO_PARSE);
        // Check if key was parsed correctly, return without changing status if not
        if(model->first_key->serial == FAILED_TO_PARSE ||
           model->first_key->count == FAILED_TO_PARSE) {
            furi_string_printf(model->info, "Failed to parse, retry");
            __gui_redraw();
            return;
        }
        // Key was received correctly
        model->status = FaacSLHRxEmuProgStatusWaitingForSecond;
        furi_string_printf(model->info, "OK First");
    } else if(model->status == FaacSLHRxEmuProgStatusWaitingForSecond) {
        // Check if the received key is normal, return without changing status if not
        if(furi_string_search_str(buffer, "Master") != FURI_STRING_FAILURE) {
            furi_string_printf(model->info, "Prog key, ignoring");
            __gui_redraw();
            return;
        }
        model->second_key->serial = __furi_string_extract_int(buffer, "Sn:", ' ', FAILED_TO_PARSE);
        model->second_key->count =
            __furi_string_extract_int(buffer, "Cnt:", '\r', FAILED_TO_PARSE);
        // Check if key was parsed correctly, return without changing status if not
        if(model->second_key->serial == FAILED_TO_PARSE ||
           model->second_key->count == FAILED_TO_PARSE) {
            furi_string_printf(model->info, "Failed to parse, retry");
            model->status = FaacSLHRxEmuProgStatusWaitingForFirst;
            __gui_redraw();
            return;
        }
        if(model->first_key->serial == model->second_key->serial &&
           model->first_key->count == model->second_key->count - 1) {
            // Two sequential keys received, remote memorized
            app->mem_remote->serial = model->second_key->serial;
            app->mem_remote->count = model->second_key->count;
            furi_string_printf(model->info, "OK, remote saved");
            model->status = FaacSLHRxEmuProgStatusNone;
        } else {
            if(model->first_key->count != model->second_key->count) {
                // Non sequential keys
                furi_string_printf(model->info, "Non sequential keys");
            }
            if(model->first_key->serial != model->second_key->serial) {
                // Serial numbers differ
                furi_string_printf(model->info, "Serial numbers differ");
            }
            // Move second key to first key, erase second key, go back to this step
            model->first_key->serial = model->second_key->serial;
            model->first_key->count = model->second_key->count;
            model->second_key->serial = 0x0;
            model->second_key->count = 0x0;
        }
    } else if(model->status == FaacSLHRxEmuProgStatusNone) {
        // At this stage a reomte has been memorized, nothing else received will be saved
        furi_string_printf(model->info, "Memory full");
        __gui_redraw();
        return;
    }

    __gui_redraw();
}
