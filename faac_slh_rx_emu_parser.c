#include "faac_slh_rx_emu_parser.h"
#include "faac_slh_rx_emu_utils.h"
// Tutto questo dovrà essere rifatto perché non voglio che il model diventi un tramite per lo scambio dati
// Dovrebbe esserci una struttura dati che tiene tipo la last read

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
    /*
    if(furi_string_search_str(buffer, "Master") == FURI_STRING_FAILURE) {
        furi_string_printf(app->model_normal->info, "Normal key, ignoring");
        __gui_redraw();
        return;
    }
    // Maybe have different variables for the programming screen and the normal screen in the model?
    // So that when you switch between the screen no jumbled mess appears?
    // Clear previous transmission
    data->fix = 0x0;
    data->hop = 0x0;
    data->sn = 0x0;
    data->seed = 0x0;
    data->btn = 0x0;
    data->cnt = 0x0;

    // I think Ke is a typo in the lib file, should be Key
    __furi_string_extract_string(buffer, 0, "Ke:", '\r', model->key);
    data->seed = __furi_string_extract_int(buffer, "Seed:", ' ', FAILED_TO_PARSE);
    // Also si può fare in modo che quando c'è un remote memorizzato non può entrare in memorizing almeno che non sia eliminato il remote
    // Oppure semplicemente se si riavvia la memorizing mode si azzera tutto

    // This count is not the remote counter but the mCnt of the programming mode.
    data->cnt =
        __furi_string_extract_int(buffer, "mCnt:", '\0' placeholder , FAILED_TO_PARSE);
    model->count = data->cnt;
    if(data->seed != FAILED_TO_PARSE) {
        model->seed = data->seed;
    }
    */

    if(furi_string_search_str(buffer, "Master") != FURI_STRING_FAILURE) {
        furi_string_printf(app->model_normal->info, "Prog key, ignoring");
        __gui_redraw();
        return;
    }
    __furi_string_extract_string(buffer, 0, "Key:", '\r', app->model_normal->key);
    // (゜-゜)
    // Se invio un segnale normale restituisce BADCODE
    // Se invio un prog restituisce BADCODE
    // Se invio un normale dopo un prog il counter è corretto
    // Deve avere qualcosa a che fare con il decoding del protocollo che ancora non mi è chiaro
    if(app->model_normal->seed == 0x0) {
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

    __gui_redraw();
}

void parse_faac_slh_prog(void* context, FuriString* buffer) {
    UNUSED(context);
    UNUSED(buffer);
}
