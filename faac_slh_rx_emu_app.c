#include "faac_slh_rx_emu_utils.h"
#include <furi.h>

#include "faac_slh_rx_emu_structs.h"
#include "faac_slh_rx_emu_about.h"
#include "faac_slh_rx_emu_decoder.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "FAACRxEmuSubGHzApp"

typedef enum {
    FaacSLHRxEmuViewReceive,
    FaacSLHRxEmuViewAbout,
    FaacSLHRxEmuViewSubmenu,
    FaacSLHRxEmuViewLastTransmission,
} FaacSLHRxEmuView;

typedef enum {
    FaacSLHRxEmuSubmenuIndexAbout,
    FaacSLHRxEmuSubMenuIndexReceive,
    FaacSLHRxEmuSubMenuIndexLastTransmission,
} FaacSLHRxEmuSubmenuIndex;

static bool decode_packet(FuriString* buffer, void* ctx) {
    FaacSLHRxEmuApp* context = (FaacSLHRxEmuApp*)ctx;
    UNUSED(context);
    // PERHCÉ VIBRA DUE VOLTE?
    // Ha smesso 	(ㆆ _ ㆆ)
    // HA RICOMINCIATO ?!?!?!?!?!
    // NON CAPISCO
    // Penso sia legato al fatto che il firmware fa vibrare alla ricezione di un segnale FAAC SLH quindi questa che imposto è una seconda vibrazione
    furi_hal_vibro_on(true);
    furi_delay_ms(50);
    furi_hal_vibro_on(false);

    if(furi_string_start_with_str(buffer, "Faac SLH 64bit")) {
        // Mi piacerebbe avere una devboard
        FURI_LOG_I(TAG, "FAAC SLH");
        decode_faac_slh(context, context->model, buffer);
        // funzione che checka se si apre o no?
    } else {
        // Mi piacerebbe avere una devboard
        FURI_LOG_I(TAG, "Not FAAC SLH");
    }

    return true;
}

// I had trouble drawing and icon to the screen so I hardcoded it with a bitmap
// To be honest I have no idea how this works
// I just found a site which gives the code given a drawing you make
static const uint8_t image_ButtonCenter_0_bits[] = {0x1c, 0x22, 0x5d, 0x5d, 0x5d, 0x22, 0x1c};

// DON'T EVER TRY TO PUT ANYTHING IN THIS CALLBACK THAT IS NOT STRICTLY NECESSARY TO THE CALLBACK
// THEY HAVE THE POWER TO MESS STUFF UP SERIOUSLY

/**
 * @brief       Callback which handles custom events
 * @details     This function is invoked every time an event occurs
 * @param       context The context
 * @param       event The id of the event
 * @return      true if everything executed correctly, false otherwise
*/
bool faac_slh_rx_emu_view_dispatcher_custom_event_callback(void* context, uint32_t event) {
    UNUSED(context);

    // Mi piacerebbe avere una devboard
    FURI_LOG_I(TAG, "Custom event received: %ld", event);

    return true;
}

uint32_t faac_slh_rx_emu_navigation_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

uint32_t faac_slh_rx_emu_navigation_submenu_callback(void* context) {
    UNUSED(context);

    return FaacSLHRxEmuViewSubmenu;
}

uint32_t faac_slh_rx_emu_navigation_submenu_stop_receiving_callback(void* context) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;
    stop_listening(app->subghz);

    return FaacSLHRxEmuViewSubmenu;
}

void faac_slh_rx_emu_receive_draw_callback(Canvas* canvas, void* model) {
    FaacSLHRxEmuModel* my_model = ((FaacSLHRxEmuRefModel*)model)->model;
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)my_model->app;

    FuriString* str = furi_string_alloc();
    canvas_set_bitmap_mode(canvas, 1);
    canvas_set_font(canvas, FontSecondary);
    furi_string_printf(str, "Key: %s", furi_string_get_cstr(my_model->key));
    canvas_draw_str(canvas, 0, 19, furi_string_get_cstr(str));

    // No idea di come si metta l'iconcina del button
    if(app->mode != FaacSLHRxEMuNormal) {
        canvas_set_font(canvas, FontPrimary);
        furi_string_printf(str, "Listening Prog Mode");
        canvas_draw_str(canvas, 0, 8, furi_string_get_cstr(str));
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(str, "Seed: %08lX  mCnt: %02lX", my_model->seed, my_model->count);
        canvas_draw_str(canvas, 0, 30, furi_string_get_cstr(str));
        canvas_draw_xbm(canvas, 119, 55, 7, 7, image_ButtonCenter_0_bits);
        canvas_draw_str(canvas, 88, 62, "Normal");
    } else {
        canvas_set_font(canvas, FontPrimary);
        furi_string_printf(str, "Listening Normal Mode");
        canvas_draw_str(canvas, 0, 8, furi_string_get_cstr(str));
        canvas_set_font(canvas, FontSecondary);
        furi_string_printf(
            str, "Serial: %07lX  Btn: %01lX", my_model->code_fix >> 4, my_model->code_fix & 0xF);
        canvas_draw_str(canvas, 0, 30, furi_string_get_cstr(str));
        if(my_model->count == FAILED_TO_PARSE) {
            furi_string_printf(str, "Count: Unknown");
        } else {
            furi_string_printf(str, "Count: %05lX", (uint32_t)my_model->count);
        }
        canvas_draw_str(canvas, 0, 41, furi_string_get_cstr(str));
        canvas_draw_xbm(canvas, 119, 55, 7, 7, image_ButtonCenter_0_bits);
        canvas_draw_str(canvas, 98, 62, "Prog");
    }

    furi_string_free(str);
}

bool faac_slh_rx_emu_input_callback(InputEvent* event, void* context) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;

    FURI_LOG_I(TAG, "Input event received: %d", event->type);
    if(event->type == InputTypeShort) {
        FURI_LOG_I(TAG, "Input key: %d", event->key);
        if(event->key == InputKeyOk) {
            if(app->mode == FaacSLHRxEMuNormal) {
                app->mode = FaacSLHRxEmuWaitingProgSignal;
            } else {
                app->mode = FaacSLHRxEMuNormal;
            }
            __gui_redraw();
        }
    }

    return false;
}

void faac_slh_rx_emu_submenu_callback(void* context, uint32_t index) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;

    switch(index) {
    case FaacSLHRxEmuSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewAbout);
        break;
    case FaacSLHRxEmuSubMenuIndexReceive:
        start_listening(app->subghz, decode_packet, app);
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewReceive);
        break;
    case FaacSLHRxEmuSubMenuIndexLastTransmission:
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
            furi_string_get_cstr(app->model->full_output));
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

FaacSLHRxEmuApp* faac_slh_rx_emu_app_alloc() {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)malloc(sizeof(FaacSLHRxEmuApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->subghz = faac_slh_rx_emu_subghz_alloc();

    app->model = malloc(sizeof(FaacSLHRxEmuModel));
    app->model->count = 0x0;
    app->model->key = furi_string_alloc();
    furi_string_printf(app->model->key, "None received");
    app->model->opened = false;
    app->model->seed = 0x0;
    app->mode = FaacSLHRxEMuNormal;
    app->model->full_output = furi_string_alloc();
    app->model->app = app;
    furi_string_printf(app->model->full_output, "No transmission received yet");

    app->last_transmission = faac_slh_data_alloc();

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
        FaacSLHRxEmuSubMenuIndexReceive,
        faac_slh_rx_emu_submenu_callback,
        app);
    submenu_add_item(
        app->submenu,
        "Last Transmission",
        FaacSLHRxEmuSubMenuIndexLastTransmission,
        faac_slh_rx_emu_submenu_callback,
        app);

    view_set_previous_callback(
        submenu_get_view(app->submenu), faac_slh_rx_emu_navigation_exit_callback);

    view_dispatcher_add_view(
        app->view_dispatcher, FaacSLHRxEmuViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewSubmenu);

    app->view_receive = view_alloc();
    view_set_context(app->view_receive, app);
    view_set_draw_callback(app->view_receive, faac_slh_rx_emu_receive_draw_callback);
    view_set_input_callback(app->view_receive, faac_slh_rx_emu_input_callback);
    view_set_previous_callback(
        app->view_receive, faac_slh_rx_emu_navigation_submenu_stop_receiving_callback);
    view_allocate_model(
        app->view_receive,
        ViewModelTypeLockFree /* what is, credo qualcosa legato alla concurrency */,
        sizeof(FaacSLHRxEmuRefModel));
    FaacSLHRxEmuRefModel* refmodel = view_get_model(app->view_receive);
    refmodel->model = app->model; // Che strano modo di assegnare un model
    view_dispatcher_add_view(app->view_dispatcher, FaacSLHRxEmuViewReceive, app->view_receive);

    app->widget_about = widget_alloc();
    widget_add_text_scroll_element(app->widget_about, 0, 0, 128, 64, FAAC_SLH_RX_EMU_ABOUT_TEXT);
    view_set_previous_callback(
        widget_get_view(app->widget_about), faac_slh_rx_emu_navigation_submenu_callback);
    view_dispatcher_add_view(
        app->view_dispatcher, FaacSLHRxEmuViewAbout, widget_get_view(app->widget_about));

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    return app;
}

void faac_slh_rx_emu_app_free(FaacSLHRxEmuApp* app) {
    furi_record_close(RECORD_NOTIFICATION);

    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewAbout);
    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewReceive);
    view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewSubmenu);
    if(app->widget_last_transmission) {
        view_dispatcher_remove_view(app->view_dispatcher, FaacSLHRxEmuViewLastTransmission);
        widget_free(app->widget_last_transmission);
    }
    submenu_free(app->submenu);
    view_free(app->view_receive);
    widget_free(app->widget_about);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);

    faac_slh_rx_emu_subghz_free(app->subghz);

    furi_string_free(app->model->key);
    free(app->model);

    faac_slh_data_free(app->last_transmission);

    free(app);
}

int32_t faac_slh_rx_emu_main(void* p) {
    UNUSED(p);

    FaacSLHRxEmuApp* app = faac_slh_rx_emu_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    faac_slh_rx_emu_app_free(app);

    return 0;
}
