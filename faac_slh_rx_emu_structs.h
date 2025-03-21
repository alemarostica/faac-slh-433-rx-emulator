#include "applications_user/faac_rx_emulator/faac_slh_rx_emu.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

typedef struct {
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

    View* view_receive;
    View* view_about;

    Widget* widget_about;

    FaacSLHRxEmu* emu;
} FaacSLHRxEmuApp;
