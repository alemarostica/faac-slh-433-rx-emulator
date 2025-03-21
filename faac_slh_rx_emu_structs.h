#include "applications_user/faac_rx_emulator/faac_slh_rx_emu.h"
#include "gui/modules/submenu.h"
#include "gui/modules/widget.h"
#include "gui/view_dispatcher.h"
#include "notification/notification.h"

// Sicuro un model per la parte del receiver mi servirà, lo metto giù in breve
typedef struct {
    uint32_t count;
    uint32_t future_count;
    bool opened;

    FuriString* key;
    FuriString* status;
} FaacSLHRxEmuModel;

typedef struct {
    FaacSLHRxEmuModel* model;
} FaacSLHRxEmuRefModel;

typedef struct {
    NotificationApp* notifications;

    ViewDispatcher* view_dispatcher;
    Submenu* submenu;

    View* view_receive;
    View* view_about;

    Widget* widget_about;

    FaacSLHRxEmuModel* model;
    FaacSLHRxEmu* emu;
} FaacSLHRxEmuApp;
