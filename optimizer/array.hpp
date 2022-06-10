#pragma once
#include <cstdint>
#include <cassert>
#include <initializer_list>
#include <algorithm>

// Lightweight array<double> with size immutable after creation but dynamic (can
// be determined at runtime)
template<class T>
class Array {
  T *data;
  std::uint32_t size;

public:
  // not zeroed!
  explicit Array(std::uint32_t size) : data(new T[size]), size(size) {}

  Array(std::initializer_list<T> list)
      : data(new T[list.size()]), size(list.size()) {
    std::uint32_t index = 0;
    for (T const& d : list) {
      data[index++] = d;
    }
  }

  Array(Array<T> const &other)
      : data(new T[other.size]), size(other.size) {
    for (std::uint32_t i = 0; i < other.size; ++i) {
      data[i] = other[i];
    }
  }

  Array &operator=(Array<T> const &other) {
    if (this == &other)
      return *this;
    Array tmp(other);
    std::swap(tmp.data, data);
    std::swap(tmp.size, size);
    return *this;
  }

  Array(Array &&other) : data(other.data), size(other.size) {
    other.data = nullptr;
    other.size = 0;
  }

  std::uint32_t Size() const { return size; }

  T operator[](std::uint32_t i) const {
#ifndef NDEBUG
    assert(i < size);
#endif
    return data[i];
  }

  T &operator[](std::uint32_t i) {
#ifndef NDEBUG
    assert(i < size);
#endif
    return data[i];
  }

  ~Array() { delete[] data; }
};
