#pragma once

#include "applications_user/faac_rx_emulator/faac_slh_rx_emu_subghz.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

// Sicuro un model per la parte del receiver mi servirà, lo metto giù in breve
/**
 * @brief   The model is used in the Receive view and stores information on the current readings, remotes and status
*/
typedef struct {
    uint32_t code_fix;
    uint32_t hop;
    uint32_t serial;
    uint32_t count;
    uint32_t future_count;
    bool opened;

    FuriString* key;
    FuriString* status;
    FuriString* full_output;
} FaacSLHRxEmuModel;

/**
 * @brief   Reference to a FaacSLHRxEmuModel object
*/
typedef struct {
    FaacSLHRxEmuModel* model;
} FaacSLHRxEmuRefModel;

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
} FaacSLHRxEmuApp;
