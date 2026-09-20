#pragma once

#include <array>
#include <atomic>
#include <cstdint>

// A small queue from the UI to the audio side: one writer, one reader,
// whole commands, nothing overwritten. The audio side drains it at the
// start of every block.

namespace synthux {

template <typename T, uint8_t N>
class Queue {
public:
  bool Push(const T& item) {
    auto tail = _tail.load(std::memory_order_relaxed);
    auto next = (uint8_t)((tail + 1) % N);
    if (next == _head.load(std::memory_order_acquire)) return false;
    _items[tail] = item;
    _tail.store(next, std::memory_order_release);
    return true;
  }

  bool Pop(T& item) {
    auto head = _head.load(std::memory_order_relaxed);
    if (head == _tail.load(std::memory_order_acquire)) return false;
    item = _items[head];
    _head.store((uint8_t)((head + 1) % N), std::memory_order_release);
    return true;
  }

private:
  std::array<T, N> _items;
  std::atomic<uint8_t> _head { 0 };
  std::atomic<uint8_t> _tail { 0 };
};

};
