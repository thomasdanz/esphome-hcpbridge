#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/core/hal.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"
#include "hoermann.h"

namespace esphome {
namespace hcpbridge {

// Shared by entities that only show a real value once the HCP bus connection is
// up: sets/clears the entity's warning flag to match `valid` and returns it, so
// callers can `if (!check_valid_or_warn(this, ...)) return;` instead of each
// repeating the same set/clear-warning boilerplate.
inline bool check_valid_or_warn(Component *component, bool valid) {
  if (!valid) {
    if (!component->status_has_warning())
      component->status_set_warning();
    return false;
  }
  if (component->status_has_warning())
    component->status_clear_warning();
  return true;
}

class HCPBridge : public PollingComponent {
 public:
  void setup() override;
  void update() override;
  void set_tx_pin(InternalGPIOPin *tx_pin) { this->tx_pin_ = tx_pin; }
  void set_rx_pin(InternalGPIOPin *rx_pin) { this->rx_pin_ = rx_pin; }
  void set_rts_pin(InternalGPIOPin *rts_pin) { this->rts_pin_ = rts_pin; }
  HoermannGarageEngine *engine;
  void add_on_state_callback(std::function<void()> &&callback, const char *tag);

 protected:
  InternalGPIOPin *tx_pin_;
  InternalGPIOPin *rx_pin_;
  InternalGPIOPin *rts_pin_;
  CallbackManager<void()> state_callback_;
};
}  // namespace hcpbridge
}  // namespace esphome