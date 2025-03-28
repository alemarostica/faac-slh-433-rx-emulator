#pragma once

#include "applications_user/faac_rx_emulator/faac_slh_rx_emu_subghz.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

/**
 * @brief   The model for the Receive view
 * @details This model is only relevant to the Receive view, its only purpose is to keep data that will be drawn to screen
*/
typedef struct {
    uint32_t code_fix;
    uint32_t hop;
    uint32_t count;
    uint32_t seed;
    bool opened;

    FuriString* key;
    FuriString* full_output;

    // This is needed to have a reference to the app context inside the draw callback for the receive view
    // I don't know if there is a better way to do it
    void* app;
} FaacSLHRxEmuModel;

typedef struct {
    uint32_t fix;
    uint32_t hop;
    uint32_t sn;
    uint8_t btn;
    uint32_t cnt;
    uint32_t seed;
} FaacSLHData;

/**
 * @brief   Reference to a FaacSLHRxEmuModel object
*/
typedef struct {
    FaacSLHRxEmuModel* model;
} FaacSLHRxEmuRefModel;

typedef enum {
    FaacSLHRxEmuWaitingProgSignal,
    FaacSLHRxEmuWaitingFirstAfterProg,
    FaacSLHRxEmuWaitingSecondAfterProg,
    FaacSLHRxEMuNormal,
} FaacSLHRxEmuReceiverMode;

/**
 * @brief   Struct of the main app
*/
typedef struct {
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

    View* view_receive;

    Widget* widget_about;
    Widget* widget_last_transmission;

    FaacSLHRxEmuModel* model;
    FaacSLHRxEmu* subghz;
    FaacSLHRxEmuReceiverMode mode;
    FaacSLHData* last_transmission;
} FaacSLHRxEmuApp;
