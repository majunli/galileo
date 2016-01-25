#ifndef __GALILEO_BASE_EXTENDABLE_PACKET_H__
#define __GALILEO_BASE_EXTENDABLE_PACKET_H__

#include "misc.h"

#include <algorithm>
#include <memory>
#include <string>

template<bool little_endian, typename size_type, uint32_t alloc_unit, bool caluate_header_size>
// 可用于数字、字符串、数据块的可扩展包类型
// little_endian 指明字节顺序是小端还是大端
// size_type 长度的数据类型
// alloc_unit 内存分配的基本单元，请使用2的幂
// caluate_header_size 是否计算自身头部的大小
class extendable_packet {
public:
  extendable_packet()
    : header_(NULL)
    , capacity_(0)
    , read_offset_(sizeof(size_type)) {
    resize(alloc_unit);
    initlize_header();
  }

  extendable_packet(const extendable_packet &other)
    : header_(NULL)
    , capacity_(other.capacity_)
    , read_offset_(other.read_offset_) {
    resize(capacity_);
    memcpy(header_, other.header_, capacity_);

  }

  extendable_packet(extendable_packet &&other)
    : header_(other.header_)
    , capacity_(other.capacity_)
    , read_offset_(other.read_offset_) {
    other.header_ = NULL;
    other.capacity_ = 0;
    other.read_offset_ = 0;
  }

  ~extendable_packet() {
    if (capacity_) {
      free(header_);
    }
  }

  extendable_packet &operator=(const extendable_packet &other) {
    capacity_ = other.capacity_;
    read_offset_ = other.read_offset_;
    resize(other.capacity_);
    memcpy(header_, other.header_, capacity_);
    return *this;
  }

  extendable_packet &operator=(extendable_packet &&other) {
    std::swap(capacity_, other.capacity_);
    std::swap(read_offset_, other.read_offset_);
    std::swap(header_, other.header_);
    return *this;
  }

  void clear() {
    initlize_header();
    capacity_ = 0;
    read_offset_ = sizeof(size_type);
  }

  // 返回数据包指针
  char *data() const { return reinterpret_cast<char*>(header_); }

  // 返回数据包的长度
  size_type size() const { return little_endian ? header_->size : size_type(swap_bytes(header_->size)); }

  // 返回数据包长度（不含头的大小）
  size_type raw_size() const { return size() + (caluate_header_size ? 0 : sizeof(size_type)); }

  // 在包的结尾处追加布尔值
  bool append_bool(bool value) {
    size_type tmp = value ? 1 : 0;
    return append_number(tmp);
  }

  // 从包中读取布尔值
  bool read_bool(bool *result) {
    size_type tmp;
    if (!read_number(&tmp))
      return false;
    *result = tmp ? true : false;
    return true;
  }

  template<typename T>
  // 在包的结尾处追加数字
  bool append_number(T value) {
    value = little_endian ? value : swap_bytes(value);
    return append_bytes(&value, sizeof(T));
  }

  template<typename T>
  // 从包中读取数字
  bool read_number(T *result) {
    uint32_t next_read_offset = read_offset_ + sizeof(T);
    if (next_read_offset > raw_size()) return false;

    *result = *reinterpret_cast<T *>((data() + read_offset_));
    if (!little_endian) *result = swap_bytes(*result);

    read_offset_ = next_read_offset;

    return true;
  }

  // 在包的结尾处追加字符串（ANSI版）
  bool append_string(const std::string &text) {
    size_type size = size_type(text.size());
    return append_number(size) && append_bytes(text.c_str(), size);
  }

  // 从包中读取字符串（ANSI版）
  bool readString(std::string *text) {
    size_type size;
    if (read_number(&size)) {
      text->resize(size);
      return read_bytes(const_cast<char *>(text->c_str()), size);
    }

    return false;
  }

  // 在包的结尾处追加字符串（宽字符版）
  bool append_wstring(const std::wstring &text) {
    size_type size = size_type(text.size() * 2);
    return append_number(size) && append_bytes(text.c_str(), size);
  }

  // 从包中读取字符串（宽字符版）
  bool read_wstring(std::wstring *str) {
    size_type size;
    if (read_number(&size)) {
      str->resize(size / 2);
      return read_bytes(const_cast<wchar_t *>(str->c_str()), size);
    }

    return false;
  }

  // 在包的结尾处追加指定字节的数据
  bool append_bytes(const void* data, int length) {
    char* dest = get_pointer_for_write(length);
    if (!dest)
      return false;

    memcpy(dest, data, length);

    header_->size = little_endian ? size() + length : swap_bytes(size_type(size() + length));

    return true;
  }

  // 从包中读取指定字节的数据
  bool read_bytes(void *dst, int length) {
    uint32_t next_read_offset = read_offset_ + length;

    if (next_read_offset > raw_size()) return false;

    memcpy(dst, data() + read_offset_, length);

    read_offset_ = next_read_offset;

    return true;
  }

private:
  typedef struct _header_type {
    typename size_type size;
  } header_type;

  template<uint32_t alignment> struct ConstantAligner {
    static uint32_t align(uint32_t bytes) {
      static_assert((alignment & (alignment - 1)) == 0,
        "alignment must be a power of two");
      return (bytes + (alignment - 1)) & ~static_cast<uint32_t>(alignment - 1);
    }
  };

  void initlize_header() {
    if (caluate_header_size)
      header_->size = little_endian ? size_type(sizeof(size_type)) : swap_bytes(size_type(sizeof(size_type)));
    else
      header_->size = 0;
  }

  bool resize(uint32_t new_capacity) {
    new_capacity = ConstantAligner<alloc_unit>::align(uint32_t(new_capacity));

    void* p = realloc(header_, std::size_t(new_capacity));
    if (!p)
      return false;

    header_ = reinterpret_cast<header_type*>(p);
    capacity_ = new_capacity;
    return true;
  }

  char* get_pointer_for_write(uint32_t length) {
    uint32_t offset = size();
    if (!caluate_header_size) offset += sizeof(size_type);
    uint32_t new_size = offset + length;
    if (new_size > capacity_ && !resize(std::max(capacity_ * 2, new_size)))
      return NULL;

    char* buffer = data() + offset;

    return buffer;
  }

  header_type *header_;
  uint32_t capacity_;
  uint32_t read_offset_;

};

#endif // !__GALILEO_BASE_EXTENDABLE_PACKET_H__
