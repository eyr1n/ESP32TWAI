#pragma once

#include <cstddef>
#include <cstdint>

#include "driver/gpio.h"
#include "driver/twai.h"

class ESP32TWAI {
public:
  ESP32TWAI(gpio_num_t rxPin, gpio_num_t txPin) {
    config_ = TWAI_GENERAL_CONFIG_DEFAULT(txPin, rxPin, TWAI_MODE_NORMAL);
    filter_ = TWAI_FILTER_CONFIG_ACCEPT_ALL();
  }

  ~ESP32TWAI() {
    twai_stop();
    twai_driver_uninstall();
  }

  void setMode(twai_mode_t mode) {
    config_.mode = mode;
  }

  void setTxQueueSize(size_t txQueueSize) {
    config_.tx_queue_len = txQueueSize;
  }

  void setRxQueueSize(size_t rxQueueSize) {
    config_.rx_queue_len = rxQueueSize;
  }

  void setFilter(const twai_filter_config_t &filter) {
    filter_ = filter;
  }

  bool begin(const twai_timing_config_t &timing) {
    if (twai_driver_install(&config_, &timing, &filter_) != ESP_OK) {
      return false;
    }
    return twai_start() == ESP_OK;
  }

  bool transmit(const twai_message_t &msg, uint32_t timeoutMs = 0) {
    return twai_transmit(&msg, pdMS_TO_TICKS(timeoutMs)) == ESP_OK;
  }

  bool receive(twai_message_t &msg, uint32_t timeoutMs = 0) {
    return twai_receive(&msg, pdMS_TO_TICKS(timeoutMs)) == ESP_OK;
  }

private:
  twai_general_config_t config_;
  twai_filter_config_t filter_;
};