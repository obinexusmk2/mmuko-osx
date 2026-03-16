/*
 * riftbridge.c - MMUKO-OS Rift Bridge UI Implementation (C)
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Terminal-based UI for MMUKO-OS ring boot visualization.
 * Renders:
 *   - Compass direction display (8-qubit ring)
 *   - NSIGII consent state (Green/Yellow/Red)
 *   - Drift discriminant graph
 *   - Boot phase progress bar
 *   - Living consent indicator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/riftbridge.h"

/* ================================================================
 * TERMINAL UTILITIES
 * ================================================================ */

void riftbridge_clear_screen(void) {
    printf("\033[2J\033[H");
}

void riftbridge_move_cursor(int x, int y) {
    printf("\033[%d;%dH", y + 1, x + 1);
}

void riftbridge_set_color(const char *color) {
    printf("%s", color);
}

void riftbridge_print_at(int x, int y, const char *text) {
    riftbridge_move_cursor(x, y);
    printf("%s", text);
}

/* ================================================================
 * BOX DRAWING
 * ================================================================ */

void riftbridge_draw_box(int x, int y, int w, int h, const char *title) {
    /* Top border */
    riftbridge_move_cursor(x, y);
    printf("+");
    for (int i = 0; i < w - 2; i++) printf("-");
    printf("+");

    /* Title */
    if (title) {
        riftbridge_move_cursor(x + 2, y);
        printf("[ %s ]", title);
    }

    /* Sides */
    for (int i = 1; i < h - 1; i++) {
        riftbridge_move_cursor(x, y + i);
        printf("|");
        riftbridge_move_cursor(x + w - 1, y + i);
        printf("|");
    }

    /* Bottom border */
    riftbridge_move_cursor(x, y + h - 1);
    printf("+");
    for (int i = 0; i < w - 2; i++) printf("-");
    printf("+");
}

/* ================================================================
 * COMPASS DISPLAY (8-qubit ring visualization)
 * ================================================================ */

void riftbridge_draw_compass(int x, int y, int direction) {
    /* ASCII compass rose */
    const char* compass_art[] = {
        "       N       ",
        "    NW   NE    ",
        "   \\   |   /   ",
        " W ---[*]--- E ",
        "   /   |   \\   ",
        "    SW   SE    ",
        "       S       "
    };

    /* Direction indicators */
    const char dir_chars[] = "^>v<";

    for (int i = 0; i < 7; i++) {
        riftbridge_print_at(x, y + i, compass_art[i]);
    }

    /* Highlight active direction */
    int dx = 0, dy = 0;
    switch (direction) {
        case 0: dx = 7; dy = 0; break;  /* N */
        case 1: dx = 11; dy = 1; break; /* NE */
        case 2: dx = 14; dy = 3; break; /* E */
        case 3: dx = 11; dy = 5; break; /* SE */
        case 4: dx = 7; dy = 6; break;  /* S */
        case 5: dx = 3; dy = 5; break;  /* SW */
        case 6: dx = 0; dy = 3; break;  /* W */
        case 7: dx = 3; dy = 1; break;  /* NW */
    }

    riftbridge_set_color(UI_COLOR_BOLD);
    riftbridge_set_color(UI_COLOR_CYAN);
    riftbridge_move_cursor(x + dx, y + dy);
    printf("*");
    riftbridge_set_color(UI_COLOR_RESET);
}

/* ================================================================
 * DRIFT GRAPH (discriminant visualization)
 * ================================================================ */

void riftbridge_draw_drift_graph(int x, int y, int w, int h,
                                  const DriftGraphData *data) {
    if (!data) return;

    riftbridge_draw_box(x, y, w, h, "DRIFT b2-4ac");

    /* Draw baseline (zero line) at middle */
    int baseline = y + h / 2;
    riftbridge_move_cursor(x + 1, baseline);
    for (int i = 0; i < w - 2; i++) printf("-");

    /* Plot history points */
    int plot_width = w - 4;
    int plot_height = h - 4;
    int start = (data->history_count > plot_width) ?
                data->history_count - plot_width : 0;

    for (int i = start; i < data->history_count && (i - start) < plot_width; i++) {
        double val = data->history[i % 60];
        /* Normalize to plot height */
        int py = baseline - (int)(val * (plot_height / 4.0));
        if (py < y + 1) py = y + 1;
        if (py >= y + h - 1) py = y + h - 2;

        /* Color based on value */
        if (val > 0) {
            riftbridge_set_color(UI_COLOR_GREEN);
        } else if (val == 0) {
            riftbridge_set_color(UI_COLOR_YELLOW);
        } else {
            riftbridge_set_color(UI_COLOR_RED);
        }

        riftbridge_move_cursor(x + 2 + (i - start), py);
        printf("*");
    }

    riftbridge_set_color(UI_COLOR_RESET);

    /* Current value label */
    riftbridge_move_cursor(x + 2, y + h - 2);
    printf("disc=%.4f", data->discriminant);
}

/* ================================================================
 * CONSENT INDICATOR (three-way colorable graph)
 * ================================================================ */

void riftbridge_draw_consent_indicator(int x, int y,
                                        const ConsentBoxData *data) {
    if (!data) return;

    riftbridge_draw_box(x, y, 30, 8, "NSIGII CONSENT");

    const char* state_str[] = {"MAYBE", "YES  ", "NO   "};
    auto const char* get_state = NULL;

    /* User consent */
    riftbridge_move_cursor(x + 2, y + 2);
    const char *u_color = (data->user_state == 0x55) ? UI_COLOR_GREEN :
                          (data->user_state == 0x00) ? UI_COLOR_YELLOW :
                          UI_COLOR_RED;
    printf("%sU(User):  %s%s",
           u_color,
           (data->user_state == 0x55) ? "YES" :
           (data->user_state == 0x00) ? "MAYBE" : "NO",
           UI_COLOR_RESET);

    /* Institution consent */
    riftbridge_move_cursor(x + 2, y + 3);
    const char *v_color = (data->inst_state == 0x55) ? UI_COLOR_GREEN :
                          (data->inst_state == 0x00) ? UI_COLOR_YELLOW :
                          UI_COLOR_RED;
    printf("%sV(Inst):  %s%s",
           v_color,
           (data->inst_state == 0x55) ? "YES" :
           (data->inst_state == 0x00) ? "MAYBE" : "NO",
           UI_COLOR_RESET);

    /* Arbiter consent */
    riftbridge_move_cursor(x + 2, y + 4);
    const char *w_color = (data->arb_state == 0x55) ? UI_COLOR_GREEN :
                          (data->arb_state == 0x00) ? UI_COLOR_YELLOW :
                          UI_COLOR_RED;
    printf("%sW(Arb):   %s%s",
           w_color,
           (data->arb_state == 0x55) ? "YES" :
           (data->arb_state == 0x00) ? "MAYBE" : "NO",
           UI_COLOR_RESET);

    /* Living loop status */
    riftbridge_move_cursor(x + 2, y + 6);
    if (data->living_loop) {
        printf("%sLiving Loop: ACTIVE%s", UI_COLOR_GREEN, UI_COLOR_RESET);
    } else {
        printf("%sLiving Loop: INACTIVE%s", UI_COLOR_RED, UI_COLOR_RESET);
    }
}

/* ================================================================
 * PHASE PROGRESS BAR
 * ================================================================ */

void riftbridge_draw_phase_bar(int x, int y, int w,
                                const PhaseBarData *data) {
    if (!data) return;

    const char* phase_names[] = {
        "SPARSE", "REMEMBER", "ACTIVE", "VERIFY", "COMPLETE"
    };

    riftbridge_move_cursor(x, y);
    printf("Boot: ");

    int seg_width = (w - 8) / data->total_phases;
    for (int i = 0; i < data->total_phases; i++) {
        if (i < data->current_phase) {
            riftbridge_set_color(data->phases_ok[i] ? UI_COLOR_GREEN : UI_COLOR_RED);
            printf("[%s]", phase_names[i]);
        } else if (i == data->current_phase) {
            riftbridge_set_color(UI_COLOR_YELLOW);
            printf("[%s]", phase_names[i]);
        } else {
            riftbridge_set_color(UI_COLOR_DIM);
            printf("[%s]", phase_names[i]);
        }
        riftbridge_set_color(UI_COLOR_RESET);

        if (i < data->total_phases - 1) printf("->");
    }
    printf("\n");
}

/* ================================================================
 * WIDGET AND PANEL MANAGEMENT
 * ================================================================ */

UIWidget* riftbridge_create_widget(WidgetType type, UIRect bounds,
                                    const char *label) {
    UIWidget *w = (UIWidget*)calloc(1, sizeof(UIWidget));
    if (!w) return NULL;

    w->type = type;
    w->bounds = bounds;
    w->visible = true;
    w->dirty = true;
    w->label = label;
    w->data = NULL;

    return w;
}

UIPanel* riftbridge_create_panel(PanelPosition pos, const char *title) {
    UIPanel *p = (UIPanel*)calloc(1, sizeof(UIPanel));
    if (!p) return NULL;

    p->position = pos;
    p->title = title;
    p->widget_count = 0;
    p->visible = true;

    /* Default panel sizes based on position */
    switch (pos) {
        case PANEL_TOP_LEFT:
            p->bounds = (UIRect){0, 0, 40, 10};
            break;
        case PANEL_TOP_RIGHT:
            p->bounds = (UIRect){41, 0, 38, 10};
            break;
        case PANEL_MID_LEFT:
            p->bounds = (UIRect){0, 11, 40, 8};
            break;
        case PANEL_MID_CENTER:
            p->bounds = (UIRect){20, 11, 40, 8};
            break;
        case PANEL_MID_RIGHT:
            p->bounds = (UIRect){41, 11, 38, 8};
            break;
        case PANEL_BOTTOM_LEFT:
            p->bounds = (UIRect){0, 20, 40, 4};
            break;
        case PANEL_BOTTOM_CENTER:
            p->bounds = (UIRect){20, 20, 40, 4};
            break;
        case PANEL_BOTTOM_RIGHT:
            p->bounds = (UIRect){41, 20, 38, 4};
            break;
    }

    return p;
}

void riftbridge_add_widget(UIPanel *panel, UIWidget *widget) {
    if (!panel || !widget) return;
    if (panel->widget_count >= RIFTBRIDGE_MAX_WIDGETS) return;
    panel->widgets[panel->widget_count++] = widget;
}

void riftbridge_add_panel(RiftBridgeUI *ui, UIPanel *panel) {
    if (!ui || !panel) return;
    if (ui->panel_count >= RIFTBRIDGE_MAX_PANELS) return;
    ui->panels[ui->panel_count++] = panel;
}

/* ================================================================
 * UI LIFECYCLE
 * ================================================================ */

RiftBridgeUI* riftbridge_create(void) {
    RiftBridgeUI *ui = (RiftBridgeUI*)calloc(1, sizeof(RiftBridgeUI));
    if (!ui) return NULL;

    ui->panel_count = 0;
    ui->running = false;
    ui->term_width = RIFTBRIDGE_TERM_WIDTH;
    ui->term_height = RIFTBRIDGE_TERM_HEIGHT;
    ui->frame_count = 0;

    /* Initialize data structures */
    ui->drift_data.discriminant = 0.0;
    ui->drift_data.history_count = 0;

    ui->compass_data.direction = 0;
    ui->compass_data.spin = 0.0;

    ui->consent_data.user_state = 0x00;  /* MAYBE */
    ui->consent_data.inst_state = 0x00;
    ui->consent_data.arb_state = 0x00;
    ui->consent_data.living_loop = false;
    ui->consent_data.color = 1;          /* Yellow */

    ui->phase_data.current_phase = 0;
    ui->phase_data.total_phases = 5;
    memset(ui->phase_data.phases_ok, 0, sizeof(ui->phase_data.phases_ok));

    return ui;
}

void riftbridge_destroy(RiftBridgeUI *ui) {
    if (!ui) return;

    for (int i = 0; i < ui->panel_count; i++) {
        if (ui->panels[i]) {
            for (int j = 0; j < ui->panels[i]->widget_count; j++) {
                free(ui->panels[i]->widgets[j]);
            }
            free(ui->panels[i]);
        }
    }
    free(ui);
}

int riftbridge_init(RiftBridgeUI *ui) {
    if (!ui) return -1;

    /* Create default panels */
    UIPanel *compass_panel = riftbridge_create_panel(PANEL_TOP_LEFT, "COMPASS RING");
    UIPanel *consent_panel = riftbridge_create_panel(PANEL_TOP_RIGHT, "NSIGII CONSENT");
    UIPanel *drift_panel = riftbridge_create_panel(PANEL_MID_LEFT, "DRIFT GRAPH");
    UIPanel *log_panel = riftbridge_create_panel(PANEL_MID_RIGHT, "BOOT LOG");

    riftbridge_add_panel(ui, compass_panel);
    riftbridge_add_panel(ui, consent_panel);
    riftbridge_add_panel(ui, drift_panel);
    riftbridge_add_panel(ui, log_panel);

    ui->running = true;
    return 0;
}

void riftbridge_shutdown(RiftBridgeUI *ui) {
    if (!ui) return;
    ui->running = false;
    riftbridge_set_color(UI_COLOR_RESET);
    printf("\n");
}

/* ================================================================
 * DATA UPDATE FUNCTIONS
 * ================================================================ */

void riftbridge_update_drift(RiftBridgeUI *ui, double discriminant,
                              int color) {
    if (!ui) return;
    ui->drift_data.discriminant = discriminant;
    ui->drift_data.color = color;

    /* Add to history ring buffer */
    int idx = ui->drift_data.history_count % 60;
    ui->drift_data.history[idx] = discriminant;
    ui->drift_data.history_count++;
}

void riftbridge_update_compass(RiftBridgeUI *ui, int direction,
                                double spin) {
    if (!ui) return;
    ui->compass_data.direction = direction;
    ui->compass_data.spin = spin;
}

void riftbridge_update_consent(RiftBridgeUI *ui, uint8_t user,
                                uint8_t inst, uint8_t arb) {
    if (!ui) return;
    ui->consent_data.user_state = user;
    ui->consent_data.inst_state = inst;
    ui->consent_data.arb_state = arb;

    /* Determine color from consent states */
    if (user == 0x55 && inst == 0x55 && arb == 0x55) {
        ui->consent_data.color = 0;  /* Green */
        ui->consent_data.living_loop = true;
    } else if (user == 0xAA || inst == 0xAA || arb == 0xAA) {
        ui->consent_data.color = 2;  /* Red */
        ui->consent_data.living_loop = false;
    } else {
        ui->consent_data.color = 1;  /* Yellow */
        ui->consent_data.living_loop = true;
    }
}

void riftbridge_update_phase(RiftBridgeUI *ui, int phase, bool passed) {
    if (!ui || phase < 0 || phase >= 5) return;
    ui->phase_data.current_phase = phase;
    ui->phase_data.phases_ok[phase] = passed;
}

/* ================================================================
 * RENDER
 * ================================================================ */

void riftbridge_render(RiftBridgeUI *ui) {
    if (!ui) return;

    riftbridge_clear_screen();

    /* Header */
    riftbridge_set_color(UI_COLOR_BOLD);
    riftbridge_print_at(0, 0,
        "MMUKO-OS Rift Bridge v" RIFTBRIDGE_VERSION
        " | OBINexus NSIGII Protocol");
    riftbridge_set_color(UI_COLOR_RESET);

    /* Draw compass */
    riftbridge_draw_compass(2, 2, ui->compass_data.direction);

    /* Draw consent indicator */
    riftbridge_draw_consent_indicator(42, 2, &ui->consent_data);

    /* Draw drift graph */
    riftbridge_draw_drift_graph(0, 10, 40, 8, &ui->drift_data);

    /* Draw phase bar */
    riftbridge_draw_phase_bar(0, 19, 78, &ui->phase_data);

    /* Status line */
    riftbridge_move_cursor(0, 21);
    printf("Frame: %d | Phase: %d/%d | ",
           ui->frame_count,
           ui->phase_data.current_phase,
           ui->phase_data.total_phases);

    /* Color indicator */
    switch (ui->consent_data.color) {
        case 0:
            printf("%sGREEN%s", UI_COLOR_GREEN, UI_COLOR_RESET);
            break;
        case 1:
            printf("%sYELLOW%s", UI_COLOR_YELLOW, UI_COLOR_RESET);
            break;
        case 2:
            printf("%sRED%s", UI_COLOR_RED, UI_COLOR_RESET);
            break;
    }

    printf(" | [Q]uit [B]reath [Y]es-consent [N]o-consent");

    fflush(stdout);
    ui->frame_count++;
}
