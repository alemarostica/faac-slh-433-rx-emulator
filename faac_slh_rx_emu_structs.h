#pragma once

#include "applications_user/faac_rx_emulator/faac_slh_rx_emu_subghz.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

#define QUEUE_SIZE 8

typedef enum {
    FaacSLHRxEmuNormalStatusNone,
    FaacSLHRxEmuNormalStatusSyncNormal,
    FaacSLHRxEmuNormalStatusSyncFirst,
    FaacSLHRxEmuNormalStatusSyncSecond,
} FaacSLHRxEmuNormalStatus;

typedef enum {
    FaacSLHRxEmuProgStatusLearned,
    FaacSLHRxEmuProgStatusWaitingForProg,
} FaacSLHRxEmuProgStatus;

typedef enum {
    FaacSLHRxEmuMemStatusEmpty,
    FaacSLHRxEmuMemStatusFull
} FaacSLHRxEmuMemStatus;

/**
 * @brief   Keeps serial and count of memorized remote
 * @details This struct is used to save a memorized remote but also comes into play when two sequential keys must be received in the programming phase
*/
typedef struct {
    uint32_t fix;
    uint32_t count;
} FaacSLHRxEmuInteral;

/**
 * @brief   The model for the Normal view
 * @details This model is only relevant to the Normal view, its only purpose is to keep data that will be drawn to screen
*/
typedef struct {
    uint32_t fix;
    uint32_t hop;
    uint32_t count;
    uint32_t seed;

    FuriString* key;
    FuriString* info;

    FaacSLHRxEmuNormalStatus status;
} FaacSLHRxEmuModelNormal;

/**
 * @brief   Reference to a FaacSLHRxEmuModelNormal object
*/
typedef struct {
    FaacSLHRxEmuModelNormal* model;
} FaacSLHRxEmuRefModelNormal;

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
} FaacSLHRxEmuModelProg;

/**
 * @brief   Reference to a FaacSLHRxEmuModelProg
*/
typedef struct {
    FaacSLHRxEmuModelProg* model;
} FaacSLHRxEmuRefModelProg;

typedef struct {
    FaacSLHRxEmuInteral* remote;
    void* app;
} FaacSLHRxEmuModelMemory;

typedef struct {
    FaacSLHRxEmuModelMemory* model;
} FaacSLHRxEmuRefModelMemory;

/**
 * @brief   Struct of the main app
*/
typedef struct {
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

    View* view_normal;
    View* view_prog;
    View* view_memory;

    Widget* widget_about;
    Widget* widget_last_transmission;

    FaacSLHRxEmuModelNormal* model_normal;
    FaacSLHRxEmuModelProg* model_prog;
    FaacSLHRxEmuModelMemory* model_memory;
    FaacSLHRxEmu* subghz;
    FaacSLHRxEmuInteral* mem_remote;
    FaacSLHRxEmuMemStatus mem_status;
    uint32_t mem_seed;
    FaacSLHRxEmuInteral* keys[QUEUE_SIZE];
    FuriString* last_transmission;
} FaacSLHRxEmuApp;
