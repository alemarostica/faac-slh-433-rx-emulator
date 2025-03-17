#include "applications_user/faac_rx_emulator/faac_rx_emu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

typedef struct {
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;

    View* view_receive_normal;
    View* view_receiver_prog;

    FaacSLHRxEmu* emu;
} FaacSLHRxEmuApp;
