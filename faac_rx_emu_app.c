#include <furi.h>

#include "faac_rx_emu_structs.h"

FaacSLHRxEmuApp* faac_emu_rx_app_alloc() {
    FaacSLHRxEmuApp* app = (FaacSLHRxEmuApp*)malloc(sizeof(FaacSLHRxEmuApp));

    Gui* gui = furi_record_open(RECORD_GUI);

    app->emu = faac_slh_rx_emu_alloc();
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);

    app->notifications = furi_record_open(RECORD_NOTIFICATION);

    return app;
}

void faac_emu_rx_app_free(FaacSLHRxEmuApp* app) {
    furi_record_close(RECORD_GUI);

    faac_slh_rx_emu_free(app->emu);

    free(app);
}

int32_t faac_emu_main(void* p) {
    UNUSED(p);

    FaacSLHRxEmuApp* app = faac_emu_rx_app_alloc();
    view_dispatcher_run(app->view_dispatcher);

    faac_emu_rx_app_free(app);

    return 0;
}
