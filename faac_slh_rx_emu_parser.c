#include "faac_slh_rx_emu_parser.h"
#include "faac_slh_rx_emu_utils.h"

// Ho difficoltà a testare il caso seed = 0x0 e/o fix = 0x0
// Il flipper si blocca provando ad inviare la normal key e non capisco il perché...
// Penso sia dovuto a quel allow_zero_seed nella libreria ma non ne sono sicuro...

#define DECODE_DEBOUNCE_MS         500
#define MAX_FUTURE_COUNT_OPEN      0x20
#define MAX_FUTURE_COUNT_RESYNC    0x8000
#define QUEUE_MIN_INDEX(key_index) ((key_index) > 4 ? (key_index) - 4 : 0)

uint32_t last_decode = 0;
static int key_index = 0;

bool debounce_decode(uint32_t now) {
    if(now - last_decode < furi_ms_to_ticks(DECODE_DEBOUNCE_MS)) {
        FURI_LOG_D(TAG, "Ignoring decode. Too soon.");
        last_decode = now;
        return true;
    }
    last_decode = now;
    return false;
}

bool is_within_range(uint32_t value, uint32_t start, uint32_t end) {
    uint32_t c_end = end & 0xFFFFF;

    if(start <= c_end) {
        return (value >= start && value <= end);
    } else {
        return (value >= start || value <= end);
    }
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
            model->status = FaacSLHRxEmuNormalStatusSyncSecond;
            furi_string_printf(model->info, "OK, first");
        } else if(model->status == FaacSLHRxEmuNormalStatusSyncSecond) {
            // il massimo di chiavi passate mantenute nella prog mode sembra essere univocamente 5
            int min = QUEUE_MIN_INDEX(key_index);
            for(int i = key_index - 1; i >= min; i--) {
                if(model->fix == app->keys[i]->fix && model->count == app->keys[i]->count + 1) {
                    furi_string_printf(model->info, "Opened, programmed");
                    app->mem_remote->fix = model->fix;
                    app->mem_remote->count = model->count;
                    model->status = FaacSLHRxEmuNormalStatusNone;
                    app->mem_status = FaacSLHRxEmuMemStatusFull;
                    app->mem_seed = app->model_prog->seed;
                    // Azzeriamo tutto meno che le ultime due ricezioni?
                    break;
                } else {
                    furi_string_printf(model->info, "Invalid key");
                }
            }
        }
        if(model->status == FaacSLHRxEmuNormalStatusSyncNormal) {
            // Il numero di chiavi passate mantenute durante un resync standard non mi è chiaro, in vari momenti il ricevitore si comporta in modo diverso senza apparente motivo.
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

        if(key_index >= QUEUE_SIZE - 1) {
            for(int i = 1; i < QUEUE_SIZE; i++) {
                memmove(app->keys[i - 1], app->keys[i], sizeof(FaacSLHRxEmuInteral));
            }
            key_index = QUEUE_SIZE - 1;
            app->keys[QUEUE_SIZE - 1]->fix = 0x0;
            app->keys[QUEUE_SIZE - 1]->count = 0x0;
        } else {
            key_index += 1;
        }

    } else {
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
    // se il count è massimo count + 0x20 apre
    // se è massimo 0x8000 count nel future deve resyncare
    // NOTA: per resyncare anche la seconda chiave ricevuta deve essere all'interno del futuro di 0x8000 count
    // Se un segnale è totalmente futuro il count non va avanti
    // se un segnale ricevuto è futuro ma quello dopo è accettabile si apre e basta
    // Nella fase di ascolto delle chiavi normali dopo la prog appare esserci una coda lunga massimo 5 elementi
    // C'è una coda lunga 8 per il resync, meaning che se vede una chiave lunga uno in più del più lungo counter sentito allora resynca al futuro che tiene anche i fix non riconosciuti
    // Se una chiave è furuta checka se nella queue di chiavi lunga un numero erratico ce n'è una esattamente precedente
    // La coda per il mem sembra molto erratica ma sono ABBASTANZA sicuro che sia lunga solo 5
    // Also non so veramente se quando riceve la master prog key il counter del ricevitore avanza

    // Questa roba non dovrebbe mai essere necessaria, ma non mi fido di me stesso
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
