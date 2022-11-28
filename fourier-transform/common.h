// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
#ifndef __COMMON_H_
#define __COMMON_H_
#include <cstdint>
#include <cstring>
#include <cassert>
#include <iterator>
#include <memory>
#include <initializer_list>

// Runtime-sized buffer owning a resource.
// If you want to create a unique array from a C-style pointer
// (malloc/free) remember to use as Deleter the provided CDeleter.
template<class T = uint8_t, class Deleter = std::default_delete<T[]>>
class UniqueBuffer {
public:
  UniqueBuffer() = delete;

  using value_type = T;
  typedef T *iterator;
  typedef const T *const_iterator;

  const_iterator begin() const { return data_; }
  const_iterator end() const { return data_ + size_; }

  iterator begin() { return data_; }
  iterator end() { return data_ + size_; }

  UniqueBuffer(const UniqueBuffer&) = delete;
  UniqueBuffer& operator= (const UniqueBuffer&) = delete;

  UniqueBuffer(UniqueBuffer&& other)
    : data_(other.data_), size_(other.size_), deleter_(other.deleter_) {
    other.data_ = nullptr;
    other.size_ = 0;
  }
  UniqueBuffer& operator= (UniqueBuffer&& other) = default;

  // Construct a UniqueBuffer from a data pointer and a size of the data pointer.
  // The constructed UniqueBuffer takes ownership of the pointer.
  // Be careful of using the right deleter depending on the pointer passed.
  UniqueBuffer(T *data, size_t size) : data_(data), size_(size), deleter_(Deleter()) {}

  // Construct an empty UniqueBuffer.
  UniqueBuffer(size_t size) : data_(new T[size]), size_(size), deleter_(Deleter()) {}

  // Construct a UniqueBuffer from an std::unique_ptr and takes ownership
  // of the holded pointer.
  UniqueBuffer(std::unique_ptr<T[], Deleter> &&data, const size_t &size)
    : data_(data.release()), size_(size), deleter_(data.get_deleter()) {}

  ~UniqueBuffer() {
    deleter_(data_);
  }

  // Create a unique uniform buffer from an initializer list.
  // The value type of the list
  explicit UniqueBuffer(std::initializer_list<T> value)
    : data_(nullptr), size_(value.size()), deleter_(Deleter()) {
    data_ = new T[size_];
    std::copy(value.begin(), value.end(), data_);
  }

  explicit UniqueBuffer(T value) : UniqueBuffer({value}) {}

  // Since we allocate the memory, we use the default type and deleter.
  UniqueBuffer<T, Deleter> Copy() const {
    UniqueBuffer<T, Deleter> b{ std::make_unique<T[]>(size_), size_ };
    std::copy(begin(), end(), b.data_);
    return b;
  }

  size_t Size() const { return size_; }
  T *Get() { return data_; }
  T *Get() const { return data_; }

  T& operator[](std::size_t i) const {
    assert(i < Size());
    return data_[i];
  }
private:
  T *data_;  // Pointer to the holded data.
  size_t size_;  // Number of elements of the array.
  Deleter deleter_;  // Object taking care of the deletion.
};

// Copy pasted char reversing from:
// https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
inline unsigned char reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

template <typename T>
T reverse_bits(T u) {
  const auto size = sizeof(T);
  unsigned char src[size];
  unsigned char dest[size];
  // Reverse chars bits.
  for (size_t i = 0; i < size; ++i) {
    dest[i] = reverse(src[i]);
  }
  // Change endianess.
  std::copy(std::rbegin(src), std::rend(src), dest);
  T out;
  std::memcpy(&out, &dest, size);
  return out;
}
#endif  // __COMMON_H_
