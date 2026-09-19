#pragma once
#include "nocopy.h"

// A knob value for a function that is not always the knob's job. While
// the function is inactive the last value holds. When it becomes active
// again the knob has to come back to that value before it takes over, so
// switching functions never jumps a setting.

namespace synthux {

class MValue {
public:
  MValue();
  ~MValue() {}

  float Process(const float value, const bool active);

  float Value() const { return _value; }

private:
  NOCOPY(MValue)

  static constexpr float kThreshold = 0.02f;

  float _init_value;
  float _value;
  bool _is_active;
  bool _is_tracking;
  bool _has_value;
};

};
