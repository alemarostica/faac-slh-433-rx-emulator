#include <furi.h>

#include "faac_slh_rx_emu_structs.h"
#include "faac_slh_rx_emu_about.h"

#ifdef TAG
#undef TAG
#endif
#define TAG "FAACRxEmuSubGHzApp"

typedef enum {
    FaacSLHRxEmuViewReceive,
    FaacSLHRxEmuViewAbout,
    FaacSLHRxEmuViewSubmenu,
} FaacSLHRxEmuView;

typedef enum {
    FaacSLHRxEmuEventIdReceivedNormalSignal,
    FaacSLHRxEmuEventIdReceivedProgSignal,
} FaacSLHRxEmuEventId;

typedef enum {
    FaacSLHRxEmuSubmenuIndexAbout,
    FaacSLHRxEmuSubMenuIndexReceive,
} FaacSLHRxEmuSubmenuIndex;

bool faac_slh_rx_emu_view_dispatcher_custom_event_callback(void* context, uint32_t event) {
    UNUSED(context);

    FURI_LOG_I(TAG, "Custom event received: %ld", event);

    return true;
}

void faac_slh_rx_submenu_callback(void* context, uint32_t index) {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)context;

    switch(index) {
    case FaacSLHRxEmuSubmenuIndexAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewAbout);
        break;
    case FaacSLHRxEmuSubMenuIndexReceive:
        view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewReceive);
        break;
    default:
        break;
    }
}

uint32_t faac_slh_rx_emu_navigation_exit_callback(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

uint32_t faac_slh_rx_emu_navigation_submenu_callback(void* context) {
    UNUSED(context);
    // stop_listening

    return FaacSLHRxEmuViewSubmenu;
}

FaacSLHRxEmuApp* faac_slh_rx_emu_app_alloc() {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)malloc(sizeof(FaacSLHRxEmuApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->emu = faac_slh_rx_emu_alloc();
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(
        app->view_dispatcher, faac_slh_rx_emu_view_dispatcher_custom_event_callback);

    app->submenu = submenu_alloc();
    submenu_add_item(
        app->submenu, "About", FaacSLHRxEmuSubmenuIndexAbout, faac_slh_rx_submenu_callback, app);
    submenu_add_item(
        app->submenu,
        "Receive",
        FaacSLHRxEmuSubMenuIndexReceive,
        faac_slh_rx_submenu_callback,
        app);

    view_set_previous_callback(
        submenu_get_view(app->submenu), faac_slh_rx_emu_navigation_exit_callback);

    view_dispatcher_add_view(
        app->view_dispatcher, FaacSLHRxEmuViewSubmenu, submenu_get_view(app->submenu));
    view_dispatcher_switch_to_view(app->view_dispatcher, FaacSLHRxEmuViewSubmenu);

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
    furi_record_close(RECORD_GUI);

    faac_slh_rx_emu_free(app->emu);

    free(app);
}

int32_t faac_emu_main(void* p) {
    UNUSED(p);

    FaacSLHRxEmuApp* app = faac_slh_rx_emu_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    faac_slh_rx_emu_app_free(app);

    return 0;
}
