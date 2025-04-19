#pragma once

#include "faac_slh_rx_emu_subghz.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

// The queue of past keys received, used in the resync phase
#define HISTORY_SIZE 8
#define MEMORY_SIZE  248

/**
 * @brief   States of the receiver
*/
typedef enum {
    FaacSLHRxEmuNormalStatusNone, // Receiving keys normally
    FaacSLHRxEmuNormalStatusSyncNormal, // Resyncing if last received key was future
    FaacSLHRxEmuNormalStatusSyncProg,
} FaacSLHRxEmuNormalStatus;

/**
 * @brief   Prog mode states
*/
typedef enum {
    FaacSLHRxEmuProgStatusLearned, // The seed was learned
    FaacSLHRxEmuProgStatusWaitingForProg, // Seed clear, waiting to learn another seed
} FaacSLHRxEmuProgStatus;

/**
 * @brief   Memory states
*/
typedef enum {
    FaacSLHRxEmuMemStatusEmpty, // No remote memorized
    FaacSLHRxEmuMemStatusFull, // Remote is memorized and synced
} FaacSLHRxEmuMemStatus;

/**
 * @brief   Keeps serial and count of memorized remote
 * @details This struct is used to save a memorized remote but also comes into play when two sequential keys must be received in the programming and syncing phases
*/
typedef struct {
    uint32_t fix;
    uint32_t count;
} FaacSLHRxEmuInteral;

/**
 * @brief   The model for the Normal view
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

/**
 * @brief   The model for the Memory View
*/
typedef struct {
    uint32_t* seed;
    void* app;
} FaacSLHRxEmuModelMemory;

/**
 * @brief   Reference to a FaacSLHRxEmuModelMemory
*/
typedef struct {
    FaacSLHRxEmuModelMemory* model;
} FaacSLHRxEmuRefModelMemory;

typedef struct {
    FaacSLHRxEmuMemStatus status;
    FaacSLHRxEmuInteral* remotes[MEMORY_SIZE];
    uint32_t seed;
    uint8_t saved_num;
} FaacSLHRxEmuMemory;

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
    FaacSLHRxEmuMemory* memory;
    FaacSLHRxEmuInteral* history[HISTORY_SIZE];
    FuriString* last_transmission;
} FaacSLHRxEmuApp;
