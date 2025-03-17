#include <furi_hal.h>

#include "faac_rx_emu.h"

static SubGhzEnvironment* load_environment() {
    SubGhzEnvironment* environment = subghz_environment_alloc();
    subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_NAME);
    subghz_environment_load_keystore(environment, SUBGHZ_KEYSTORE_DIR_USER_NAME);
    return environment;
}

FaacSLHRxEmu* faac_slh_rx_emu_alloc() {
    FaacSLHRxEmu* faac_emu = malloc(sizeof(FaacSLHRxEmu));
    faac_emu->status = SUBGHZ_RECEIVER_UNINITIALIZED;
    faac_emu->environment = load_environment();
    faac_emu->stream =
        furi_stream_buffer_alloc(sizeof(LevelDuration) * 1024, sizeof(LevelDuration));
    furi_check(faac_emu->stream);
    return faac_emu;
}

void faac_slh_rx_emu_free(FaacSLHRxEmu* context) {
    subghz_environment_free(context->environment);
    furi_stream_buffer_free(context->stream);
    free(context);
}
