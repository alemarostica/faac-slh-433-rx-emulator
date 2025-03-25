#include "faac_slh_rx_emu_decoder.h"
#include "faac_slh_rx_emu_utils.h"

typedef struct {
    uint32_t fix;
    uint32_t hop;
    uint32_t sn;
    uint8_t btn;
    uint32_t cnt;
} FaacSLHData;

FaacSLHData* faac_slh_data_alloc() {
    FaacSLHData* data = malloc(sizeof(FaacSLHData));
    return data;
}

void faac_slh_data_free(FaacSLHData* data) {
    free(data);
}

uint32_t last_decode = 0;
void decode_faac_slh(FaacSLHRxEmuModel* model, FuriString* buffer) {
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
    __furi_string_extract_string(buffer, 0, "Key:", '\r', model->key);

    data->fix = __furi_string_extract_int(buffer, "Fix:", ' ', FAILED_TO_PARSE);
    data->hop = __furi_string_extract_int(buffer, "Hop:", ' ', FAILED_TO_PARSE);
    data->sn = __furi_string_extract_int(buffer, "Sn:", ' ', FAILED_TO_PARSE);
    // Questo if è vagamente inutile, ma mettiamolo lo stesso
    if(data->sn == FAILED_TO_PARSE) {
        FURI_LOG_I(TAG, "Sn: not found, using fix data");
        data->sn = data->fix & 0x0FFFFFFF;
    }
    data->btn = __furi_string_extract_int(buffer, "Btn:", '\r', FAILED_TO_PARSE);
    // (゜-゜)
    // Se invio un segnale normale restituisce BADCODE
    // Se invio un prog restituisce BADCODE
    // Se invio un normale dopo un prog il counter è corretto
    // Deve avere qualcosa a che fare con il decoding del protocollo che ancora non mi è chiaro
    data->cnt = __furi_string_extract_int(buffer, "Cnt:", '\r', FAILED_TO_PARSE);
    model->count = data->cnt;
    __gui_redraw();

    faac_slh_data_free(data);
}
