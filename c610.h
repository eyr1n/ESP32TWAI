#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

#include "esp32twai.h"

class C610 {
public:
  enum class ID {
    ID1,
    ID2,
    ID3,
    ID4,
    ID5,
    ID6,
    ID7,
    ID8,
  };

  void begin(ESP32TWAI *twai) {
    twai_ = twai;
  }

  void update() {
    twai_message_t msg;
    while (twai_->receive(msg)) {
      for (size_t i = 0; i < 8; ++i) {
        if (msg.identifier == 0x201 + i) {
          int16_t angle = static_cast<int16_t>(msg.data[0] << 8 | msg.data[1]);
          int16_t delta = angle - prevAngle_[i];
          if (delta > 4096) {
            delta -= 8192;
          } else if (delta < -4096) {
            delta += 8192;
          }
          position_[i] += delta;
          prevAngle_[i] = angle;

          rpm_[i] = static_cast<int16_t>(msg.data[2] << 8 | msg.data[3]);
          current_[i] = static_cast<int16_t>(msg.data[4] << 8 | msg.data[5]);
          break;
        }
      }
    }

    msg.identifier = 0x200;
    msg.data_length_code = 8;
    for (size_t i = 0; i < 4; ++i) {
      msg.data[i * 2] = currentTarget_[i] >> 8;
      msg.data[i * 2 + 1] = currentTarget_[i];
    }
    twai_->transmit(msg);
    msg.identifier = 0x1FF;
    for (size_t i = 0; i < 4; ++i) {
      msg.data[i * 2] = currentTarget_[i + 4] >> 8;
      msg.data[i * 2 + 1] = currentTarget_[i + 4];
    }
    twai_->transmit(msg);
  }

  float getRpm(ID id) {
    return rpm_[toUnderlying(id)];
  }

  float getRps(ID id) {
    return getRpm(id) / 60.0f;
  }

  float getPosition(ID id) {
    return position_[toUnderlying(id)] / 8192.0f;
  }

  void setPosition(ID id, float position) {
    position_[toUnderlying(id)] = position * 8192;
  }

  // -10000 ~ 10000 mA
  int16_t getCurrent(ID id) {
    return current_[toUnderlying(id)];
  }

  void setCurrent(ID id, int16_t current) {
    currentTarget_[toUnderlying(id)] = current;
  }

private:
  ESP32TWAI *twai_;

  std::array<int16_t, 8> prevAngle_ = {};
  std::array<int64_t, 8> position_ = {};
  std::array<int16_t, 8> rpm_ = {};
  std::array<int16_t, 8> current_ = {};
  std::array<int16_t, 8> currentTarget_ = {};

  template<class T> constexpr std::underlying_type_t<T> toUnderlying(T value) noexcept {
    return static_cast<std::underlying_type_t<T>>(value);
  }
};
