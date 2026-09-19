#include "mvalue.h"

using namespace synthux;

MValue::MValue():
_init_value   { 0.f },
_value        { 0.f },
_is_active    { false },
_is_tracking  { false },
_has_value    { false }
{};

float MValue::Process(const float value, const bool active) {
  if (active && !_is_active) {
    _init_value = value;
    _is_tracking = !_has_value;
  }
  _is_active = active;
  if (!active) return _value;
  if (!_is_tracking) {
    auto from_below = _init_value < _value;
    auto reached = from_below ? value >= _value - kThreshold
                              : value <= _value + kThreshold;
    if (!reached) return _value;
    _is_tracking = true;
  }
  _value = value;
  _has_value = true;
  return _value;
};
