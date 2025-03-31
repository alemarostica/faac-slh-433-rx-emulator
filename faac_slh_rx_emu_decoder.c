#include "faac_slh_rx_emu_decoder.h"
#include "faac_slh_rx_emu_utils.h"

FaacSLHData* faac_slh_data_alloc() {
    FaacSLHData* data = malloc(sizeof(FaacSLHData));
    data->fix = 0x0;
    data->hop = 0x0;
    data->sn = 0x0;
    data->seed = 0x0;
    data->btn = 0x0;
    data->cnt = 0x0;
    return data;
}

void faac_slh_data_free(FaacSLHData* data) {
    free(data);
}

// Tutto questo dovrà essere rifatto perché non voglio che il model diventi un tramite per lo scambio dati
// Dovrebbe esserci una struttura dati che tiene tipo la last read

uint32_t last_decode = 0;
void decode_faac_slh(void* context, FaacSLHRxEmuModel* model, FuriString* buffer) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    // There is a possibility che ci tocchi fare il decoding solo dopo che si è verificato se è un prog mode o no
    FURI_LOG_T(TAG, "Decoding FAAC SLH...");
    uint32_t now = furi_get_tick();
    if(now - last_decode < furi_ms_to_ticks(500)) {
        FURI_LOG_D(TAG, "Ignoring decode. Too soon.");
        last_decode = now;
        return;
    }
    last_decode = now;

    furi_string_printf(model->full_output, "Last transmission:\n%s", furi_string_get_cstr(buffer));

    FaacSLHData* data = faac_slh_data_alloc();

    if(app->mode == FaacSLHRxEmuWaitingProgSignal) {
        if(furi_string_search_str(buffer, "Master") == FURI_STRING_FAILURE) {
            furi_string_printf(model->info, "Normal key, ignoring");
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
            __furi_string_extract_int(buffer, "mCnt:", '\0' /* placeholder */, FAILED_TO_PARSE);
        model->count = data->cnt;
        if(data->seed != FAILED_TO_PARSE) {
            model->seed = data->seed;
        }
    } else {
        if(furi_string_search_str(buffer, "Master") != FURI_STRING_FAILURE) {
            furi_string_printf(model->info, "Prog key, ignoring");
            __gui_redraw();
            return;
        }
        __furi_string_extract_string(buffer, 0, "Key:", '\r', model->key);
        // (゜-゜)
        // Se invio un segnale normale restituisce BADCODE
        // Se invio un prog restituisce BADCODE
        // Se invio un normale dopo un prog il counter è corretto
        // Deve avere qualcosa a che fare con il decoding del protocollo che ancora non mi è chiaro
        if(model->seed == 0x0) {
            data->fix = __furi_string_extract_int(buffer, "Fix:", '\r', FAILED_TO_PARSE);
            data->cnt = FAILED_TO_PARSE;
        } else {
            data->fix = __furi_string_extract_int(buffer, "Fix:", ' ', FAILED_TO_PARSE);
            data->cnt = __furi_string_extract_int(buffer, "Cnt:", '\r', FAILED_TO_PARSE);
        }
        data->hop = __furi_string_extract_int(buffer, "Hop:", ' ', FAILED_TO_PARSE);
        data->sn = __furi_string_extract_int(buffer, "Sn:", ' ', FAILED_TO_PARSE);
        // Questo if è vagamente inutile, ma mettiamolo lo stesso
        if(data->sn == FAILED_TO_PARSE) {
            FURI_LOG_I(TAG, "Sn: not found, using fix data");
            data->sn = data->fix & 0x0FFFFFFF;
        }
        data->btn = __furi_string_extract_int(buffer, "Btn:", '\r', FAILED_TO_PARSE);
        model->count = data->cnt;
        model->code_fix = data->fix;
        model->hop = data->hop;
        subghz_receiver_reset(app->subghz->receiver); // Forse questa c'entra qualcosa
        // Ok forse il problema del fatto che il decoder comunque si ricorda il counter può essere
        // risolto alla carlona facendo in modo che se non avviene la memorizzazione completa il
        // ricevitore si rifiuta di aprire perché non ha memorizzato il seed nel modello/stato
        // Ha anche senso che si comporti così nella realtà, security risk? maybe
    }

    __gui_redraw();

    faac_slh_data_free(data);
}
