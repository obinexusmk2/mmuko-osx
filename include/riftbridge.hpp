/*
 * riftbridge.hpp - MMUKO-OS Rift Bridge C++ Interface
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * C++ wrapper around the C riftbridge API, providing:
 *   - RAII lifecycle management
 *   - Object-oriented panel/widget composition
 *   - Event-driven boot visualization
 *   - Bridge to C# managed layer via extern "C" exports
 *
 * Architecture: src/ ↔ include/ separation
 *   include/riftbridge.hpp  → C++ class declarations
 *   src/riftbridge.cpp      → C++ implementations
 *   include/riftbridge.h    → C API (shared with C# P/Invoke)
 *   src/riftbridge.c        → C rendering core
 */

#ifndef MMUKO_RIFTBRIDGE_HPP
#define MMUKO_RIFTBRIDGE_HPP

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

/* Include C API */
extern "C" {
#include "riftbridge.h"
#include "ringboot.h"
#include "bootsec.h"
}

namespace mmuko {
namespace riftbridge {

/* ================================================================
 * EVENT SYSTEM
 * ================================================================ */

/** Event callback type */
using EventCallback = std::function<void(UIEventType, void*)>;

/** Event listener registration */
struct EventListener {
    UIEventType     event_type;
    EventCallback   callback;
    std::string     name;
};

/* ================================================================
 * WIDGET CLASSES
 * ================================================================ */

/** Base widget wrapper */
class Widget {
public:
    Widget(WidgetType type, UIRect bounds, const std::string& label);
    virtual ~Widget();

    virtual void render() const;
    virtual void update();

    void setVisible(bool visible);
    bool isVisible() const;
    bool isDirty() const;
    void markDirty();

    WidgetType getType() const { return type_; }
    const std::string& getLabel() const { return label_; }
    UIRect getBounds() const { return bounds_; }

protected:
    WidgetType  type_;
    UIRect      bounds_;
    std::string label_;
    bool        visible_;
    bool        dirty_;
    UIWidget*   c_widget_;  /* Underlying C widget */
};

/** Compass ring widget */
class CompassWidget : public Widget {
public:
    CompassWidget(UIRect bounds);
    ~CompassWidget() override = default;

    void render() const override;
    void setDirection(CompassDirection dir);
    void setSpin(double spin);
    void setQubitStates(const int states[8]);
    void setEntanglement(const bool entangled[8]);

private:
    CompassDisplayData data_;
};

/** Drift graph widget */
class DriftWidget : public Widget {
public:
    DriftWidget(UIRect bounds);
    ~DriftWidget() override = default;

    void render() const override;
    void addSample(double discriminant);
    void setColor(NSIGIIColor color);

private:
    DriftGraphData data_;
};

/** NSIGII consent display widget */
class ConsentWidget : public Widget {
public:
    ConsentWidget(UIRect bounds);
    ~ConsentWidget() override = default;

    void render() const override;
    void setConsent(uint8_t user, uint8_t inst, uint8_t arb);
    void setLivingLoop(bool active);
    NSIGIIColor getColor() const;

private:
    ConsentBoxData data_;
};

/** Phase progress bar widget */
class PhaseWidget : public Widget {
public:
    PhaseWidget(UIRect bounds);
    ~PhaseWidget() override = default;

    void render() const override;
    void setPhase(RingBootPhase phase, bool passed);
    int getCurrentPhase() const;

private:
    PhaseBarData data_;
};

/* ================================================================
 * PANEL CLASS
 * ================================================================ */

class Panel {
public:
    Panel(PanelPosition position, const std::string& title);
    ~Panel();

    void addWidget(std::shared_ptr<Widget> widget);
    void render() const;
    void setVisible(bool visible);
    bool isVisible() const;

    const std::string& getTitle() const { return title_; }
    PanelPosition getPosition() const { return position_; }

private:
    PanelPosition                       position_;
    std::string                         title_;
    UIRect                              bounds_;
    std::vector<std::shared_ptr<Widget>> widgets_;
    bool                                visible_;
    UIPanel*                            c_panel_;
};

/* ================================================================
 * RIFT BRIDGE APPLICATION CLASS
 * ================================================================ */

class RiftBridgeApp {
public:
    RiftBridgeApp();
    ~RiftBridgeApp();

    /* Lifecycle */
    bool initialize();
    void shutdown();
    int  run();

    /* Connection to ring boot system */
    void attachRingBootSystem(RingBootSystem* sys);
    void detachRingBootSystem();

    /* Event handling */
    void addEventListener(UIEventType type, EventCallback callback,
                          const std::string& name = "");
    void removeEventListener(const std::string& name);
    void dispatchEvent(UIEventType type, void* data = nullptr);

    /* Panel management */
    void addPanel(std::shared_ptr<Panel> panel);
    std::shared_ptr<Panel> getPanel(PanelPosition pos) const;

    /* State updates (from ring boot callbacks) */
    void onPhaseChange(RingBootPhase phase, bool success);
    void onConsentUpdate(uint8_t user, uint8_t inst, uint8_t arb);
    void onDriftUpdate(double discriminant, NSIGIIColor color);
    void onCompassUpdate(CompassDirection dir, double spin);

    /* Rendering */
    void render();
    bool isRunning() const { return running_; }

private:
    RiftBridgeUI*                        c_ui_;
    RingBootSystem*                      ring_sys_;
    std::vector<std::shared_ptr<Panel>>  panels_;
    std::vector<EventListener>           listeners_;
    bool                                 running_;
    bool                                 initialized_;
    uint32_t                             frame_count_;

    /* Internal helpers */
    void setupDefaultPanels();
    void processInput();
    void updateFromSystem();
};

/* ================================================================
 * EXTERN "C" BRIDGE EXPORTS (for C# P/Invoke)
 * ================================================================ */

extern "C" {

/** Create riftbridge application instance */
void* riftbridge_app_create(void);

/** Destroy riftbridge application instance */
void riftbridge_app_destroy(void* app);

/** Initialize the application */
int riftbridge_app_init(void* app);

/** Run one frame of the application */
int riftbridge_app_frame(void* app);

/** Update consent state */
void riftbridge_app_set_consent(void* app, uint8_t user,
                                 uint8_t inst, uint8_t arb);

/** Update phase */
void riftbridge_app_set_phase(void* app, int phase, int passed);

/** Update drift */
void riftbridge_app_set_drift(void* app, double discriminant, int color);

/** Check if running */
int riftbridge_app_is_running(void* app);

/** Shutdown */
void riftbridge_app_shutdown(void* app);

}  /* extern "C" */

}  /* namespace riftbridge */
}  /* namespace mmuko */

#endif /* MMUKO_RIFTBRIDGE_HPP */
