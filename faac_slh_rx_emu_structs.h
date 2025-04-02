#pragma once

#include "applications_user/faac_rx_emulator/faac_slh_rx_emu_subghz.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

/**
 * @brief   The model for the Normal view
 * @details This model is only relevant to the Normal view, its only purpose is to keep data that will be drawn to screen
*/
typedef struct {
    uint32_t code_fix;
    uint32_t hop;
    uint32_t count;
    uint32_t seed;

    FuriString* key;
    FuriString* info;
} FaacSLHRxEmuModelNormal;

/**
 * @brief   Keeps serial and count of memorized remote
 * @details This struct is used to save a memorized remote but also comes into play when two sequential keys must be received in the programming phase
*/
typedef struct {
    uint32_t serial;
    uint32_t count;
} FaacSLHRxEmuInteral;

/**
 * @brief   Reference to a FaacSLHRxEmuModelNormal object
*/
typedef struct {
    FaacSLHRxEmuModelNormal* model;
} FaacSLHRxEmuRefModelNormal;

typedef enum {
    FaacSLHRxEmuProgStatusNone,
    FaacSLHRxEmuProgStatusWaitingForProg,
    FaacSLHRxEmuProgStatusWaitingForFirst,
    FaacSLHRxEmuProgStatusWaitingForSecond,
} FaacSLHRxEmuProgStatus;

/**
 * @brief   The model for the Prog view
 * @details This model is only relevant to the Prog view, its only purpose is to keep data that will be drawn to screen
*/
typedef struct {
    uint32_t seed;
    uint8_t mCnt;

    FuriString* key;
    FuriString* info;

    FaacSLHRxEmuProgStatus status;

    FaacSLHRxEmuInteral* first_key;
    FaacSLHRxEmuInteral* second_key;
} FaacSLHRxEmuModelProg;

/**
 * @brief   Reference to a FaacSLHRxEmuModelProg
*/
typedef struct {
    FaacSLHRxEmuModelProg* model;
} FaacSLHRxEmuRefModelProg;

/**
 * @brief   Struct of the main app
*/
typedef struct {
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

    View* view_normal;
    View* view_prog;

    Widget* widget_about;
    Widget* widget_last_transmission;

    FaacSLHRxEmuModelNormal* model_normal;
    FaacSLHRxEmuModelProg* model_prog;
    FaacSLHRxEmu* subghz;
    FaacSLHRxEmuInteral* mem_remote;
    FuriString* last_transmission;
} FaacSLHRxEmuApp;
