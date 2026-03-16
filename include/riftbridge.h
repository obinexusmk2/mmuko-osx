/*
 * riftbridge.h - MMUKO-OS Rift Bridge UI Interface (C API)
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Rift Bridge provides the UI abstraction layer between:
 *   - The MMUKO ring boot kernel (C)
 *   - The riftbridge presentation layer (C++/C#)
 *   - External rift compiler chain (riftlang.exe → .so.a → rift.exe)
 *
 * UI Model (from handwritten notes):
 *   - AOB grid: Application Object Browser
 *   - .mds/.mids: MMUKO data stream / MMUKO indexed data stream
 *   - Ring zone cursor navigation
 *   - Three-way color state display (Green/Yellow/Red)
 *   - Drift visualization panel
 *
 * The riftbridge serves as the "surface" and "face" layer (cube face model)
 * connecting the firmware to human-readable consent interfaces.
 */

#ifndef MMUKO_RIFTBRIDGE_H
#define MMUKO_RIFTBRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================================================================
 * RIFT BRIDGE CONSTANTS
 * ================================================================ */

#define RIFTBRIDGE_VERSION      "2.0.0"
#define RIFTBRIDGE_MAX_PANELS   8
#define RIFTBRIDGE_MAX_WIDGETS  32
#define RIFTBRIDGE_TERM_WIDTH   80
#define RIFTBRIDGE_TERM_HEIGHT  24

/* UI Color codes (ANSI terminal) */
#define UI_COLOR_RESET    "\033[0m"
#define UI_COLOR_GREEN    "\033[32m"
#define UI_COLOR_YELLOW   "\033[33m"
#define UI_COLOR_RED      "\033[31m"
#define UI_COLOR_CYAN     "\033[36m"
#define UI_COLOR_BOLD     "\033[1m"
#define UI_COLOR_DIM      "\033[2m"

/* Widget types */
typedef enum {
    WIDGET_LABEL       = 0,
    WIDGET_STATUS_BAR  = 1,
    WIDGET_COMPASS     = 2,
    WIDGET_QUBIT_RING  = 3,
    WIDGET_DRIFT_GRAPH = 4,
    WIDGET_CONSENT_BOX = 5,
    WIDGET_LOG_VIEW    = 6,
    WIDGET_PHASE_BAR   = 7
} WidgetType;

/* Panel positions */
typedef enum {
    PANEL_TOP_LEFT     = 0,
    PANEL_TOP_RIGHT    = 1,
    PANEL_MID_LEFT     = 2,
    PANEL_MID_CENTER   = 3,
    PANEL_MID_RIGHT    = 4,
    PANEL_BOTTOM_LEFT  = 5,
    PANEL_BOTTOM_CENTER= 6,
    PANEL_BOTTOM_RIGHT = 7
} PanelPosition;

/* UI Event types */
typedef enum {
    UI_EVENT_NONE        = 0,
    UI_EVENT_KEY_PRESS   = 1,
    UI_EVENT_CONSENT_YES = 2,
    UI_EVENT_CONSENT_NO  = 3,
    UI_EVENT_BREATH      = 4,   /* Breath confirmation event */
    UI_EVENT_PHASE_CHANGE= 5,
    UI_EVENT_DRIFT_UPDATE= 6,
    UI_EVENT_QUIT        = 7
} UIEventType;

/* ================================================================
 * STRUCTURES
 * ================================================================ */

/* UI Coordinate */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} UIRect;

/* UI Color (RGB) */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} UIColor;

/* Widget base */
typedef struct {
    WidgetType  type;
    UIRect      bounds;
    bool        visible;
    bool        dirty;       /* Needs redraw? */
    const char* label;
    void*       data;        /* Widget-specific data pointer */
} UIWidget;

/* Panel container */
typedef struct {
    PanelPosition  position;
    UIRect         bounds;
    UIWidget*      widgets[RIFTBRIDGE_MAX_WIDGETS];
    int            widget_count;
    bool           visible;
    const char*    title;
} UIPanel;

/* UI Event */
typedef struct {
    UIEventType type;
    int         key_code;
    uint32_t    timestamp;
    void*       data;
} UIEvent;

/* Drift visualization data */
typedef struct {
    double discriminant;     /* b²-4ac current value */
    double history[60];      /* Last 60 samples */
    int    history_count;
    int    color;            /* 0=green, 1=yellow, 2=red */
} DriftGraphData;

/* Compass visualization data */
typedef struct {
    int    direction;        /* 0-7 compass direction */
    double spin;             /* Current spin value */
    int    qubit_states[8];  /* State per qubit */
    bool   entangled[8];     /* Entanglement flags */
} CompassDisplayData;

/* Consent box data */
typedef struct {
    uint8_t user_state;      /* NSIGII_YES/NO/MAYBE */
    uint8_t inst_state;
    uint8_t arb_state;
    bool    living_loop;
    int     color;           /* Three-way graph color */
} ConsentBoxData;

/* Phase progress bar data */
typedef struct {
    int    current_phase;    /* 0-4 */
    int    total_phases;     /* 5 */
    bool   phases_ok[5];     /* Per-phase pass/fail */
} PhaseBarData;

/* Rift Bridge UI context */
typedef struct {
    UIPanel*    panels[RIFTBRIDGE_MAX_PANELS];
    int         panel_count;
    bool        running;
    int         term_width;
    int         term_height;
    uint32_t    frame_count;
    UIEvent     last_event;

    /* Linked data from ring boot system */
    DriftGraphData    drift_data;
    CompassDisplayData compass_data;
    ConsentBoxData    consent_data;
    PhaseBarData      phase_data;
} RiftBridgeUI;

/* ================================================================
 * FUNCTION PROTOTYPES - C API
 * ================================================================ */

/* Lifecycle */
RiftBridgeUI*   riftbridge_create(void);
void            riftbridge_destroy(RiftBridgeUI *ui);
int             riftbridge_init(RiftBridgeUI *ui);
void            riftbridge_shutdown(RiftBridgeUI *ui);

/* Main loop */
int             riftbridge_run(RiftBridgeUI *ui);
void            riftbridge_render(RiftBridgeUI *ui);
UIEvent         riftbridge_poll_event(RiftBridgeUI *ui);
void            riftbridge_handle_event(RiftBridgeUI *ui, UIEvent *event);

/* Panel management */
UIPanel*        riftbridge_create_panel(PanelPosition pos, const char *title);
void            riftbridge_add_panel(RiftBridgeUI *ui, UIPanel *panel);
void            riftbridge_render_panel(const UIPanel *panel);

/* Widget creation */
UIWidget*       riftbridge_create_widget(WidgetType type, UIRect bounds,
                                          const char *label);
void            riftbridge_add_widget(UIPanel *panel, UIWidget *widget);
void            riftbridge_render_widget(const UIWidget *widget);

/* Data update functions */
void            riftbridge_update_drift(RiftBridgeUI *ui, double discriminant,
                                         int color);
void            riftbridge_update_compass(RiftBridgeUI *ui, int direction,
                                           double spin);
void            riftbridge_update_consent(RiftBridgeUI *ui, uint8_t user,
                                           uint8_t inst, uint8_t arb);
void            riftbridge_update_phase(RiftBridgeUI *ui, int phase,
                                         bool passed);

/* Rendering helpers */
void            riftbridge_draw_box(int x, int y, int w, int h,
                                     const char *title);
void            riftbridge_draw_compass(int x, int y, int direction);
void            riftbridge_draw_drift_graph(int x, int y, int w, int h,
                                             const DriftGraphData *data);
void            riftbridge_draw_consent_indicator(int x, int y,
                                                   const ConsentBoxData *data);
void            riftbridge_draw_phase_bar(int x, int y, int w,
                                           const PhaseBarData *data);

/* Terminal utilities */
void            riftbridge_clear_screen(void);
void            riftbridge_move_cursor(int x, int y);
void            riftbridge_set_color(const char *color);
void            riftbridge_print_at(int x, int y, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* MMUKO_RIFTBRIDGE_H */
