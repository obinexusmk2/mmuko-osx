/*
 * riftbridge.cpp - MMUKO-OS Rift Bridge C++ Implementation
 * Project: OBINexus / MMUKO-OS
 * Protocol: NSIGII Human Rights Verification Firmware
 *
 * Implements the C++ object-oriented layer for the rift bridge UI.
 * Wraps the C terminal rendering with RAII and event-driven patterns.
 */

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include "../include/riftbridge.hpp"

namespace mmuko {
namespace riftbridge {

/* ================================================================
 * WIDGET BASE CLASS
 * ================================================================ */

Widget::Widget(WidgetType type, UIRect bounds, const std::string& label)
    : type_(type), bounds_(bounds), label_(label),
      visible_(true), dirty_(true), c_widget_(nullptr) {
    c_widget_ = riftbridge_create_widget(type, bounds, label.c_str());
}

Widget::~Widget() {
    if (c_widget_) {
        free(c_widget_);
    }
}

void Widget::render() const {
    if (!visible_ || !c_widget_) return;
    riftbridge_draw_box(bounds_.x, bounds_.y,
                        bounds_.width, bounds_.height,
                        label_.c_str());
}

void Widget::update() {
    dirty_ = false;
}

void Widget::setVisible(bool visible) { visible_ = visible; }
bool Widget::isVisible() const { return visible_; }
bool Widget::isDirty() const { return dirty_; }
void Widget::markDirty() { dirty_ = true; }

/* ================================================================
 * COMPASS WIDGET
 * ================================================================ */

CompassWidget::CompassWidget(UIRect bounds)
    : Widget(WIDGET_COMPASS, bounds, "COMPASS RING") {
    std::memset(&data_, 0, sizeof(data_));
}

void CompassWidget::render() const {
    if (!visible_) return;
    riftbridge_draw_compass(bounds_.x, bounds_.y, data_.direction);
}

void CompassWidget::setDirection(CompassDirection dir) {
    data_.direction = static_cast<int>(dir);
    dirty_ = true;
}

void CompassWidget::setSpin(double spin) {
    data_.spin = spin;
    dirty_ = true;
}

void CompassWidget::setQubitStates(const int states[8]) {
    std::memcpy(data_.qubit_states, states, sizeof(data_.qubit_states));
    dirty_ = true;
}

void CompassWidget::setEntanglement(const bool entangled[8]) {
    std::memcpy(data_.entangled, entangled, sizeof(data_.entangled));
    dirty_ = true;
}

/* ================================================================
 * DRIFT WIDGET
 * ================================================================ */

DriftWidget::DriftWidget(UIRect bounds)
    : Widget(WIDGET_DRIFT_GRAPH, bounds, "DRIFT b2-4ac") {
    std::memset(&data_, 0, sizeof(data_));
}

void DriftWidget::render() const {
    if (!visible_) return;
    riftbridge_draw_drift_graph(bounds_.x, bounds_.y,
                                bounds_.width, bounds_.height,
                                &data_);
}

void DriftWidget::addSample(double discriminant) {
    data_.discriminant = discriminant;
    int idx = data_.history_count % 60;
    data_.history[idx] = discriminant;
    data_.history_count++;
    dirty_ = true;
}

void DriftWidget::setColor(NSIGIIColor color) {
    data_.color = static_cast<int>(color);
    dirty_ = true;
}

/* ================================================================
 * CONSENT WIDGET
 * ================================================================ */

ConsentWidget::ConsentWidget(UIRect bounds)
    : Widget(WIDGET_CONSENT_BOX, bounds, "NSIGII CONSENT") {
    std::memset(&data_, 0, sizeof(data_));
}

void ConsentWidget::render() const {
    if (!visible_) return;
    riftbridge_draw_consent_indicator(bounds_.x, bounds_.y, &data_);
}

void ConsentWidget::setConsent(uint8_t user, uint8_t inst, uint8_t arb) {
    data_.user_state = user;
    data_.inst_state = inst;
    data_.arb_state = arb;

    /* Determine three-way color */
    if (user == NSIGII_YES && inst == NSIGII_YES && arb == NSIGII_YES) {
        data_.color = 0;  /* Green */
        data_.living_loop = true;
    } else if (user == NSIGII_NO || inst == NSIGII_NO || arb == NSIGII_NO) {
        data_.color = 2;  /* Red */
        data_.living_loop = false;
    } else {
        data_.color = 1;  /* Yellow */
        data_.living_loop = true;
    }
    dirty_ = true;
}

void ConsentWidget::setLivingLoop(bool active) {
    data_.living_loop = active;
    dirty_ = true;
}

NSIGIIColor ConsentWidget::getColor() const {
    return static_cast<NSIGIIColor>(data_.color);
}

/* ================================================================
 * PHASE WIDGET
 * ================================================================ */

PhaseWidget::PhaseWidget(UIRect bounds)
    : Widget(WIDGET_PHASE_BAR, bounds, "BOOT PHASES") {
    std::memset(&data_, 0, sizeof(data_));
    data_.total_phases = 5;
}

void PhaseWidget::render() const {
    if (!visible_) return;
    riftbridge_draw_phase_bar(bounds_.x, bounds_.y,
                               bounds_.width, &data_);
}

void PhaseWidget::setPhase(RingBootPhase phase, bool passed) {
    int p = static_cast<int>(phase);
    if (p >= 0 && p < 5) {
        data_.current_phase = p;
        data_.phases_ok[p] = passed;
    }
    dirty_ = true;
}

int PhaseWidget::getCurrentPhase() const {
    return data_.current_phase;
}

/* ================================================================
 * PANEL CLASS
 * ================================================================ */

Panel::Panel(PanelPosition position, const std::string& title)
    : position_(position), title_(title), visible_(true), c_panel_(nullptr) {
    c_panel_ = riftbridge_create_panel(position, title.c_str());
    if (c_panel_) {
        bounds_ = c_panel_->bounds;
    }
}

Panel::~Panel() {
    if (c_panel_) {
        free(c_panel_);
    }
}

void Panel::addWidget(std::shared_ptr<Widget> widget) {
    widgets_.push_back(widget);
}

void Panel::render() const {
    if (!visible_) return;

    riftbridge_draw_box(bounds_.x, bounds_.y,
                        bounds_.width, bounds_.height,
                        title_.c_str());

    for (const auto& widget : widgets_) {
        if (widget->isVisible()) {
            widget->render();
        }
    }
}

void Panel::setVisible(bool visible) { visible_ = visible; }
bool Panel::isVisible() const { return visible_; }

/* ================================================================
 * RIFT BRIDGE APPLICATION
 * ================================================================ */

RiftBridgeApp::RiftBridgeApp()
    : c_ui_(nullptr), ring_sys_(nullptr),
      running_(false), initialized_(false), frame_count_(0) {
}

RiftBridgeApp::~RiftBridgeApp() {
    shutdown();
}

bool RiftBridgeApp::initialize() {
    c_ui_ = riftbridge_create();
    if (!c_ui_) return false;

    if (riftbridge_init(c_ui_) != 0) {
        riftbridge_destroy(c_ui_);
        c_ui_ = nullptr;
        return false;
    }

    setupDefaultPanels();
    running_ = true;
    initialized_ = true;

    return true;
}

void RiftBridgeApp::shutdown() {
    if (c_ui_) {
        riftbridge_shutdown(c_ui_);
        riftbridge_destroy(c_ui_);
        c_ui_ = nullptr;
    }
    running_ = false;
    initialized_ = false;
    panels_.clear();
    listeners_.clear();
}

void RiftBridgeApp::setupDefaultPanels() {
    /* Compass panel (top-left) */
    auto compass_panel = std::make_shared<Panel>(PANEL_TOP_LEFT, "COMPASS RING");
    auto compass_widget = std::make_shared<CompassWidget>(
        UIRect{2, 2, 16, 7});
    compass_panel->addWidget(compass_widget);
    panels_.push_back(compass_panel);

    /* Consent panel (top-right) */
    auto consent_panel = std::make_shared<Panel>(PANEL_TOP_RIGHT, "NSIGII CONSENT");
    auto consent_widget = std::make_shared<ConsentWidget>(
        UIRect{42, 2, 30, 8});
    consent_panel->addWidget(consent_widget);
    panels_.push_back(consent_panel);

    /* Drift panel (mid-left) */
    auto drift_panel = std::make_shared<Panel>(PANEL_MID_LEFT, "DRIFT GRAPH");
    auto drift_widget = std::make_shared<DriftWidget>(
        UIRect{0, 10, 40, 8});
    drift_panel->addWidget(drift_widget);
    panels_.push_back(drift_panel);

    /* Phase panel (bottom) */
    auto phase_panel = std::make_shared<Panel>(PANEL_BOTTOM_CENTER, "BOOT PHASES");
    auto phase_widget = std::make_shared<PhaseWidget>(
        UIRect{0, 19, 78, 2});
    phase_panel->addWidget(phase_widget);
    panels_.push_back(phase_panel);
}

void RiftBridgeApp::attachRingBootSystem(RingBootSystem* sys) {
    ring_sys_ = sys;
}

void RiftBridgeApp::detachRingBootSystem() {
    ring_sys_ = nullptr;
}

void RiftBridgeApp::addEventListener(UIEventType type, EventCallback callback,
                                      const std::string& name) {
    listeners_.push_back({type, callback, name});
}

void RiftBridgeApp::removeEventListener(const std::string& name) {
    listeners_.erase(
        std::remove_if(listeners_.begin(), listeners_.end(),
                       [&name](const EventListener& l) {
                           return l.name == name;
                       }),
        listeners_.end());
}

void RiftBridgeApp::dispatchEvent(UIEventType type, void* data) {
    for (auto& listener : listeners_) {
        if (listener.event_type == type) {
            listener.callback(type, data);
        }
    }
}

void RiftBridgeApp::onPhaseChange(RingBootPhase phase, bool success) {
    if (c_ui_) {
        riftbridge_update_phase(c_ui_, static_cast<int>(phase), success);
    }
    dispatchEvent(UI_EVENT_PHASE_CHANGE, nullptr);
}

void RiftBridgeApp::onConsentUpdate(uint8_t user, uint8_t inst, uint8_t arb) {
    if (c_ui_) {
        riftbridge_update_consent(c_ui_, user, inst, arb);
    }
    dispatchEvent(UI_EVENT_CONSENT_YES, nullptr);
}

void RiftBridgeApp::onDriftUpdate(double discriminant, NSIGIIColor color) {
    if (c_ui_) {
        riftbridge_update_drift(c_ui_, discriminant, static_cast<int>(color));
    }
    dispatchEvent(UI_EVENT_DRIFT_UPDATE, nullptr);
}

void RiftBridgeApp::onCompassUpdate(CompassDirection dir, double spin) {
    if (c_ui_) {
        riftbridge_update_compass(c_ui_, static_cast<int>(dir), spin);
    }
}

void RiftBridgeApp::render() {
    if (!c_ui_) return;
    riftbridge_render(c_ui_);
    frame_count_++;
}

int RiftBridgeApp::run() {
    if (!initialized_) {
        if (!initialize()) return -1;
    }

    /* Single frame execution (for integration with boot sequence) */
    if (ring_sys_) {
        updateFromSystem();
    }

    render();
    return running_ ? 0 : 1;
}

void RiftBridgeApp::updateFromSystem() {
    if (!ring_sys_ || !c_ui_) return;

    /* Pull state from ring boot system */
    riftbridge_update_compass(c_ui_,
                               static_cast<int>(ring_sys_->frame_of_reference),
                               compass_spin_value(ring_sys_->frame_of_reference));

    /* Update phase */
    riftbridge_update_phase(c_ui_,
                             static_cast<int>(ring_sys_->state_machine.phase),
                             ring_sys_->state_machine.phase < RINGBOOT_PHASE_FAILED);

    /* Update consent from boot sector */
    if (ring_sys_->boot_sector) {
        riftbridge_update_consent(c_ui_,
                                   ring_sys_->boot_sector->consent.user_consent,
                                   ring_sys_->boot_sector->consent.inst_consent,
                                   ring_sys_->boot_sector->consent.arb_consent);

        riftbridge_update_drift(c_ui_,
                                 ring_sys_->boot_sector->consent.discriminant,
                                 static_cast<int>(ring_sys_->boot_sector->consent.color));
    }
}

/* ================================================================
 * EXTERN "C" BRIDGE (for C# P/Invoke)
 * ================================================================ */

extern "C" {

void* riftbridge_app_create(void) {
    return new RiftBridgeApp();
}

void riftbridge_app_destroy(void* app) {
    delete static_cast<RiftBridgeApp*>(app);
}

int riftbridge_app_init(void* app) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    return a->initialize() ? 0 : -1;
}

int riftbridge_app_frame(void* app) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    return a->run();
}

void riftbridge_app_set_consent(void* app, uint8_t user,
                                 uint8_t inst, uint8_t arb) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    a->onConsentUpdate(user, inst, arb);
}

void riftbridge_app_set_phase(void* app, int phase, int passed) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    a->onPhaseChange(static_cast<RingBootPhase>(phase), passed != 0);
}

void riftbridge_app_set_drift(void* app, double discriminant, int color) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    a->onDriftUpdate(discriminant, static_cast<NSIGIIColor>(color));
}

int riftbridge_app_is_running(void* app) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    return a->isRunning() ? 1 : 0;
}

void riftbridge_app_shutdown(void* app) {
    auto* a = static_cast<RiftBridgeApp*>(app);
    a->shutdown();
}

}  /* extern "C" */

}  /* namespace riftbridge */
}  /* namespace mmuko */
