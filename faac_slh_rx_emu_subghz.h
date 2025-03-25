#pragma once

#include <furi.h>
#include <furi_hal.h>

#include <lib/subghz/receiver.h>
#include <lib/subghz/protocols/protocol_items.h>
#include "devices/cc1101_int/cc1101_int_interconnect.h"
#include "devices/devices.h"
#include "devices/types.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "FAACRxEmuSubGHz"

typedef bool (*SubghzPacketCallback)(FuriString* buffer, void* context);

/**
 * @brief   The status of the receiver
*/
typedef enum {
    SUBGHZ_RECEIVER_INITIALIZING,
    SUBGHZ_RECEIVER_LISTENING,
    SUBGHZ_RECEIVER_SYNCHRONIZED,
    SUBGHZ_RECEIVER_NOTLISTENING,
    SUBGHZ_RECEIVER_UNINITIALIZING,
    SUBGHZ_RECEIVER_UNINITIALIZED,
} SubghzReceiverState;

/**
 * @brief   The receiver emulator object
*/
typedef struct {
    SubGhzEnvironment* environment;
    FuriStreamBuffer* stream;
    FuriThread* thread;
    SubGhzReceiver* receiver;
    SubghzReceiverState status;
    SubghzPacketCallback callback;
    void* callback_context;
    bool overrun;
} FaacSLHRxEmu;

FaacSLHRxEmu* faac_slh_rx_emu_subghz_alloc();
void faac_slh_rx_emu_subghz_free(FaacSLHRxEmu* context);

void start_listening(FaacSLHRxEmu* context, SubghzPacketCallback callback, void* callback_context);
void stop_listening(FaacSLHRxEmu* context);
