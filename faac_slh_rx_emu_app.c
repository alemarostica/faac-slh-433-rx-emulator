#include <furi.h>

#include "faac_slh_rx_emu_structs.h"
#include "faac_slh_rx_emu_about.h"
#include "faac_slh_rx_emu_utils.h"
#include "faac_slh_rx_emu_parser.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "FaacSLHRxEmuSubGHzApp"

/**
 * @brief   View IDs.
*/
typedef enum {
    FaacSLHRxEmuViewNormal,
    FaacSLHRxEmuViewProg,
    FaacSLHRxEmuViewAbout,
    FaacSLHRxEmuViewSubmenu,
    FaacSLHRxEmuViewLastTransmission,
    FaacSLHRxEmuViewMemory,
} FaacSLHRxEmuView;

/**
 * @brief   Submenu Indexes.
*/
typedef enum {
    FaacSLHRxEmuSubmenuIndexAbout,
    FaacSLHRxEmuSubmenuIndexNormal,
    FaacSLHRxEmuSubmenuIndexProg,
    FaacSLHRxEmuSubmenuIndexLastTransmission,
    FaacSLHRxEmuSubmenuIndexMemory,
} FaacSLHRxEmuSubmenuIndex;

/**
 * @brief   Callback for parsing normal keys.
*/
static bool parse_packet_normal(FuriString* buffer, void* context) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    furi_hal_vibro_on(true);
    furi_delay_ms(50);
    furi_hal_vibro_on(false);

    if(furi_string_start_with_str(buffer, "Faac SLH 64bit")) {
        FURI_LOG_I(TAG, "FAAC SLH");
        if(furi_string_search_str(buffer, "Master") != FURI_STRING_FAILURE) {
            furi_string_printf(app->model_normal->info, "Prog key, ignoring");
            __gui_redraw();
        } else {
            parse_faac_slh_normal(app, buffer);
        }
    } else {
        FURI_LOG_I(TAG, "Not FAAC SLH");
    }

    return true;
}

/**
 * @brief   Callback for parsing prog keys.
*/
static bool parse_packet_prog(FuriString* buffer, void* context) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;

    furi_hal_vibro_on(true);
    furi_delay_ms(50);
    furi_hal_vibro_on(false);

    if(furi_string_start_with_str(buffer, "Faac SLH 64bit")) {
        FURI_LOG_I(TAG, "FAAC SLH");
        parse_faac_slh_prog(app, buffer);
    } else {
        FURI_LOG_I(TAG, "Not FAAC SLH");
    }

    return true;
}

/**
 * @brief   [UNUSED] Callback for custom events.
*/
bool faac_slh_rx_emu_view_dispatcher_custom_event_callback(void* context, uint32_t event) {
    UNUSED(context);
    FURI_LOG_I(TAG, "Custom event received: %ld", event);
    return true;
}

/**
 * @brief   Callback for exiting app.
*/
uint32_t faac_slh_rx_emu_navigation_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

/**
 * @brief   Callback to submenu.
*/
uint32_t faac_slh_rx_emu_navigation_submenu_callback(void* context) {
    UNUSED(context);
    return FaacSLHRxEmuViewSubmenu;
}

/**
 * @brief   Callback to submenu from Receive view.
*/
uint32_t faac_slh_rx_emu_navigation_submenu_normal_callback(void* context) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    /*
    if(app->model_normal->status == FaacSLHRxEmuNormalStatusSyncFirst ||
       app->model_normal->status == FaacSLHRxEmuNormalStatusSyncSecond) {
        app->model_normal->status = FaacSLHRxEmuNormalStatusNone;
        furi_string_printf(app->model_normal->info, "No remote memorized");

        app->memory->seed = 0x0;
        for(int i = 0; i < MEMORY_SIZE; i++) {
            app->memory->remotes[i]->fix = 0x0;
            app->memory->remotes[i]->count = 0xFFF00000;
        }
        app->memory->status = FaacSLHRxEmuMemStatusEmpty;
    }
    */
    stop_listening(app->subghz);

    return FaacSLHRxEmuViewSubmenu;
}

/**
 * @brief   Callback to submenu from Prog view.
*/
uint32_t faac_slh_rx_emu_navigation_submenu_prog_callback(void* context) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    stop_listening(app->subghz);

    return FaacSLHRxEmuViewSubmenu;
}

/**
 * @brief   Draw callback for Receive view.
*/
void faac_slh_rx_emu_normal_draw_callback(Canvas* canvas, void* my_model) {
    FaacSLHRxEmuModelNormal* model = ((FaacSLHRxEmuRefModelNormal*)my_model)->model;
    FuriString* str = furi_string_alloc();

    canvas_set_bitmap_mode(canvas, 1);
    canvas_set_font(canvas, FontPrimary);
    furi_string_printf(str, "Normal mode");
    canvas_draw_str(canvas, 0, 8, furi_string_get_cstr(str));
    canvas_set_font(canvas, FontSecondary);
    furi_string_printf(str, "Key: %s", furi_string_get_cstr(model->key));
    canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(str));
    furi_string_printf(str, "Serial: %07lX  Btn: %01lX", model->fix >> 4, model->fix & 0xF);
    canvas_draw_str(canvas, 0, 30, furi_string_get_cstr(str));
    if(model->count == 0xFFF00000) {
        furi_string_printf(str, "Count: Unknown");
    } else {
        furi_string_printf(str, "Count: %05lX", model->count);
    }
    canvas_draw_str(canvas, 0, 41, furi_string_get_cstr(str));
    furi_string_printf(str, "Info: %s", furi_string_get_cstr(model->info));
    canvas_draw_str(canvas, 0, 52, furi_string_get_cstr(str));

    furi_string_free(str);
}

/**
 * @brief   Draw callback for Prog view.
*/
void faac_slh_rx_emu_prog_draw_callback(Canvas* canvas, void* my_model) {
    FaacSLHRxEmuModelProg* model = ((FaacSLHRxEmuRefModelProg*)my_model)->model;
    FuriString* str = furi_string_alloc();

    canvas_set_bitmap_mode(canvas, 1);
    canvas_set_font(canvas, FontPrimary);
    furi_string_printf(str, "Programming mode");
    canvas_draw_str(canvas, 0, 8, furi_string_get_cstr(str));
    canvas_set_font(canvas, FontSecondary);
    furi_string_printf(str, "Key: %s", furi_string_get_cstr(model->key));
    canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(str));
    furi_string_printf(str, "Seed: %08lX  mCnt: %02X", model->seed, model->mCnt);
    canvas_draw_str(canvas, 0, 30, furi_string_get_cstr(str));
    furi_string_printf(str, "Info: %s", furi_string_get_cstr(model->info));
    canvas_draw_str(canvas, 0, 41, furi_string_get_cstr(str));

    furi_string_free(str);
}

/**
 * @brief   Draw callback for Memory view.
*/
void faac_slh_rx_emu_memory_draw_callback(Canvas* canvas, void* my_model) {
    FaacSLHRxEmuModelMemory* model = ((FaacSLHRxEmuRefModelMemory*)my_model)->model;
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)model->app;
    FuriString* str = furi_string_alloc();

    canvas_set_bitmap_mode(canvas, 1);
    canvas_set_font(canvas, FontPrimary);
    furi_string_printf(str, "Remote Memory");
    canvas_draw_str(canvas, 0, 8, furi_string_get_cstr(str));
    canvas_set_font(canvas, FontSecondary);
    if(app->memory->status == FaacSLHRxEmuMemStatusFull) {
        /*
        furi_string_printf(str, "Fix: %08lX", app->mem_remote->fix);
        canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(str));
        furi_string_printf(str, "Seed: %08lX", app->mem_seed);
        canvas_draw_str(canvas, 0, 30, furi_string_get_cstr(str));
        furi_string_printf(str, "Cnt: %05lX", app->mem_remote->count);
        canvas_draw_str(canvas, 0, 41, furi_string_get_cstr(str));
        */
        furi_string_printf(str, "Seed: %08lX", *model->seed);
        canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(str));
    } else {
        furi_string_printf(str, "No remote in memory");
        canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(str));
    }

    furi_string_free(str);
}

/**
 * @brief   [UNUSED] Callback for input.
*/
bool faac_slh_rx_emu_input_callback(InputEvent* event, void* context) {
    UNUSED(event);
    UNUSED(context);

    return false;
}

/**
 * @brief   Submenu callback.
 * @details This function executes code based on the selected submenu entry.
*/
void faac_slh_rx_emu_submenu_callback(void* context, uint32_t index) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;

    switch(index) {
    case FaacSLHRxEmuSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewAbout);
        break;
    case FaacSLHRxEmuSubmenuIndexNormal:
        start_listening(app->subghz, parse_packet_normal, app);
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewNormal);
        break;
    case FaacSLHRxEmuSubmenuIndexProg:
        // Every time this view in entered a new remote is programmed and the last forgotten
        furi_string_printf(app->model_prog->info, "Waiting for prog key");
        furi_string_printf(app->model_prog->key, "None received");
        app->model_prog->mCnt = 0x0;
        app->model_prog->seed = 0x0;
        app->model_prog->status = FaacSLHRxEmuProgStatusWaitingForProg;
        app->memory->status = FaacSLHRxEmuMemStatusEmpty;
        app->memory->seed = 0x0;
        app->memory->saved_num = 0;
        for(int i = 0; i < MEMORY_SIZE; i++) {
            app->memory->remotes[i]->fix = 0x0;
            app->memory->remotes[i]->count = 0xFFF00000;
        }
        for(uint32_t i = 0; i < HISTORY_SIZE; i++) {
            app->history[i]->fix = 0x0;
            app->history[i]->count = 0xFFF00000;
        }
        if(app->subghz != NULL) {
            // This is ugly. Doing it differently would require more substantial firmware editing
            faac_slh_rx_emu_subghz_free(app->subghz);
            app->subghz = faac_slh_rx_emu_subghz_alloc();
        }
        start_listening(app->subghz, parse_packet_prog, app);
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewProg);
        break;
    case FaacSLHRxEmuSubmenuIndexMemory:
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewMemory);
        break;
    case FaacSLHRxEmuSubmenuIndexLastTransmission:
        // Also ugly, but I couldn't find a better solution
        // A new widget is created every time the submenu is accessed
        if(app->widget_last_transmission) {
            view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewLastTransmission);
            widget_free(app->widget_last_transmission);
        }
        app->widget_last_transmission = widget_alloc();
        widget_add_string_multiline_element(
            app->widget_last_transmission,
            0,
            0,
            AlignLeft,
            AlignTop,
            FontSecondary,
            furi_string_get_cstr(app->last_transmission));
        view_set_previous_callback(
            widget_get_view(app->widget_last_transmission),
            faac_slh_rx_emu_navigation_submenu_callback);
        view_dispatcher_add_view(
            app->view_dispatcher,
            FaacSLHRxEmuViewLastTransmission,
            widget_get_view(app->widget_last_transmission));
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewLastTransmission);
        break;
    default:
        break;
    }
}

/**
 * @brief   App allocation.
*/
FaacSLHRxEmuApp* faac_slh_rx_emu_app_alloc() {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)malloc(sizeof(FaacSLHRxEmuApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->subghz = faac_slh_rx_emu_subghz_alloc();

    app->last_transmission = furi_string_alloc();
    furi_string_printf(app->last_transmission, "Nothing received yet");

    app->model_normal = malloc(sizeof(FaacSLHRxEmuModelNormal));
    app->model_normal->fix = 0x0;
    app->model_normal->hop = 0x0;
    app->model_normal->seed = 0x0;
    app->model_normal->count = 0xFFF00000;
    app->model_normal->key = furi_string_alloc();
    app->model_normal->info = furi_string_alloc();
    app->model_normal->status = FaacSLHRxEmuNormalStatusNone;
    for(int i = 0; i < HISTORY_SIZE; i++) {
        app->history[i] = malloc(sizeof(FaacSLHRxEmuInteral));
    }
    furi_string_printf(app->model_normal->key, "None received");
    furi_string_printf(app->model_normal->info, "No remote memorized");

    app->model_prog = malloc(sizeof(FaacSLHRxEmuModelProg));
    app->model_prog->seed = 0x0;
    app->model_prog->mCnt = 0x0;
    app->model_prog->key = furi_string_alloc();
    app->model_prog->info = furi_string_alloc();
    app->model_prog->status = FaacSLHRxEmuProgStatusWaitingForProg;

    app->memory = malloc(sizeof(FaacSLHRxEmuMemory));
    app->memory->status = FaacSLHRxEmuMemStatusEmpty;
    app->memory->seed = 0x0;
    app->memory->saved_num = 0;
    for(int i = 0; i < MEMORY_SIZE; i++) {
        app->memory->remotes[i] = malloc(sizeof(FaacSLHRxEmuInteral));
        app->memory->remotes[i]->fix = 0x0;
        app->memory->remotes[i]->count = 0xFFF00000;
    }

    app->model_memory = malloc(sizeof(FaacSLHRxEmuModelMemory));
    app->model_memory->seed = &app->memory->seed;
    app->model_memory->app = app;

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, faac_slh_rx_emu_view_dispatcher_custom_event_callback);

    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu,
        "About",
        FaacSLHRxEmuSubmenuIndexAbout,
        faac_slh_rx_emu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Receive",
        FaacSLHRxEmuSubmenuIndexNormal,
        faac_slh_rx_emu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Program new remote",
        FaacSLHRxEmuSubmenuIndexProg,
        faac_slh_rx_emu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Memory",
        FaacSLHRxEmuSubmenuIndexMemory,
        faac_slh_rx_emu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Last Transmission",
        FaacSLHRxEmuSubmenuIndexLastTransmission,
        faac_slh_rx_emu_submenu_callback,
        app);

    view_set_previous_callback(
        submenu_get_view(app->submenu), faac_slh_rx_emu_navigation_exit_callback);

    view_dispatcher_add_view(
        app->view_dispatcher, FaacSLHRxEmuViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewSubmenu);

    app->view_normal = view_alloc();
    view_set_context(app->view_normal, app);
    view_set_draw_callback(app->view_normal, faac_slh_rx_emu_normal_draw_callback);
    view_set_input_callback(app->view_normal, faac_slh_rx_emu_input_callback);
    view_set_previous_callback(
        app->view_normal, faac_slh_rx_emu_navigation_submenu_normal_callback);
    view_allocate_model(
        app->view_normal,
        ViewModelTypeLockFree /* what is this, concurrency related stuff? */,
        sizeof(FaacSLHRxEmuRefModelNormal));
    FaacSLHRxEmuRefModelNormal* normal_model = view_get_model(app->view_normal);
    normal_model->model = app->model_normal; // Weird way to assign a model
    view_dispatcher_add_view(app->view_dispatcher, FaacSLHRxEmuViewNormal, app->view_normal);

    app->view_prog = view_alloc();
    view_set_context(app->view_prog, app);
    view_set_draw_callback(app->view_prog, faac_slh_rx_emu_prog_draw_callback);
    view_set_input_callback(app->view_prog, faac_slh_rx_emu_input_callback);
    view_set_previous_callback(app->view_prog, faac_slh_rx_emu_navigation_submenu_prog_callback);
    view_allocate_model(app->view_prog, ViewModelTypeLockFree, sizeof(FaacSLHRxEmuRefModelProg));
    FaacSLHRxEmuRefModelProg* prog_model = view_get_model(app->view_prog);
    prog_model->model = app->model_prog;
    view_dispatcher_add_view(app->view_dispatcher, FaacSLHRxEmuViewProg, app->view_prog);

    app->view_memory = view_alloc();
    view_set_context(app->view_memory, app);
    view_set_draw_callback(app->view_memory, faac_slh_rx_emu_memory_draw_callback);
    view_set_input_callback(app->view_memory, faac_slh_rx_emu_input_callback);
    view_set_previous_callback(app->view_memory, faac_slh_rx_emu_navigation_submenu_callback);
    view_allocate_model(app->view_memory, ViewModelTypeLockFree, sizeof(FaacSLHRxEmuModelMemory));
    FaacSLHRxEmuRefModelMemory* mem_model = view_get_model(app->view_memory);
    mem_model->model = app->model_memory;
    view_dispatcher_add_view(app->view_dispatcher, FaacSLHRxEmuViewMemory, app->view_memory);

    app->widget_about = widget_alloc();
    widget_add_text_scroll_element(app->widget_about, 0, 0, 128, 64, FAAC_SLH_RX_EMU_ABOUT_TEXT);
    view_set_previous_callback(
        widget_get_view(app->widget_about), faac_slh_rx_emu_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, FaacSLHRxEmuViewAbout, widget_get_view(app->widget_about));

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    return app;
}

/**
 * @brief   App deallocation.
*/
void faac_slh_rx_emu_app_free(FaacSLHRxEmuApp* app) {
    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewAbout);
    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewNormal);
    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewProg);
    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewSubmenu);
    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewMemory);
    if(app->widget_last_transmission) {
        view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewLastTransmission);
        widget_free(app->widget_last_transmission);
    }
    submenu_free(app->submenu);
    view_free(app->view_normal);
    view_free(app->view_prog);
    view_free(app->view_memory);
    widget_free(app->widget_about);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);

    faac_slh_rx_emu_subghz_free(app->subghz);

    furi_string_free(app->last_transmission);
    furi_string_free(app->model_normal->key);
    furi_string_free(app->model_normal->info);
    furi_string_free(app->model_prog->key);
    furi_string_free(app->model_prog->info);
    for(int i = 0; i < HISTORY_SIZE; i++) {
        free(app->history[i]);
    }
    for(int i = 0; i < MEMORY_SIZE; i++) {
        free(app->memory->remotes[i]);
    }
    free(app->memory);

    free(app->model_prog);
    free(app->model_normal);
    free(app->model_memory);

    free(app);
}

int32_t faac_slh_rx_emu_main(void* p) {
    UNUSED(p);

    FaacSLHRxEmuApp* app = faac_slh_rx_emu_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    faac_slh_rx_emu_app_free(app);

    return 0;
}
