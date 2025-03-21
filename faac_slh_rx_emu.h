#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <lib/subghz/receiver.h>

typedef enum {
    SUBGHZ_RECEIVER_INITIALIZING,
    SUBGHZ_RECEIVER_LISTENING,
    SUBGHZ_RECEIVER_SYNCHRONIZED,
    SUBGHZ_RECEIVER_NOTLISTENING,
    SUBGHZ_RECEIVER_UNINITIALIZING,
    SUBGHZ_RECEIVER_UNINITIALIZED,
} SubghzReceiverState;

typedef struct {
    SubGhzEnvironment* environment;
    FuriStreamBuffer* stream;
    FuriThread* thread;
    SubGhzReceiver* receiver;
    SubghzReceiverState status;
    void* callback_context;
} FaacSLHRxEmu;

FaacSLHRxEmu* faac_slh_rx_emu_alloc();
void faac_slh_rx_emu_free(FaacSLHRxEmu* context);

void start_listening(FaacSLHRxEmu* context, void* callback_context);
void stop_listening(FaacSLHRxEmu* context);
