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

    __gui_redraw();
}

void parse_faac_slh_prog(void* context, FuriString* buffer) {
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

    __furi_string_extract_string(buffer, 0, "Ke:", '\r', app->model_prog->key);
    app->model_prog->seed = __furi_string_extract_int(buffer, "Seed:", ' ', FAILED_TO_PARSE);
    if(app->model_normal->seed != 0x0 && app->model_normal->seed == app->model_prog->seed) {
        furi_string_printf(app->model_prog->info, "Memory full");
        return;
    } else {
        if(app->model_prog->seed == FAILED_TO_PARSE) {
            furi_string_printf(app->model_prog->info, "Failed to read seed");
        } else {
            app->model_normal->seed = app->model_prog->seed;
        }
    }
    app->model_prog->mCnt = __furi_string_extract_int(buffer, "mCnt:", '\0', FAILED_TO_PARSE);
    furi_string_printf(app->model_prog->info, "OK");
    furi_string_printf(app->model_normal->info, "Waiting");

    __gui_redraw();
}
