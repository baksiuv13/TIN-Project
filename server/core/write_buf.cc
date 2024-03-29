// Copyright 2019 Piotrek

#include <algorithm>

#include "core/write_buf.h"

namespace tin {

int WriteBuf::Add(const char *s, size_t n) {
  if (Bad()) {
    return -1;
  } else if (n > static_cast<size_t>(Place())) {
    // Przepełniony
    start_ = -1;
    return -1;
  }
  int i = 0;
  int j = i + len_;
  while (static_cast<size_t>(i) < n) {
    buf_[GetAt_(j)] = s[i];
    ++i;
    ++j;
  }
  len_ += i;
  return i;
}

/*
int WriteBuf::Add(const char *s, size_t n) {
  if (Bad()) return -1;
  
  const char *x = s;
  int i = 0;
  int j = i + len_;
  while (*x) {
    if (GetAt_(j) == start_) {
      // Przepełniony
      start_ = -1;
      return -1;
    }
    buf_[GetAt_(j)] = *x;
    ++i;
    ++j;
    ++x;
    ++len_;
  }
  return i;
}
*/

int WriteBuf::Add(const std::string &str) {
  return Add(str.c_str(), str.size());
}
/*
int WriteBuf::Add(const std::string &str) {
  if (Bad()) {
    return -1;
  } else if (str.size() > static_cast<size_t>(Place())) {
    // Przepełniony
    start_ = -1;
    return -1;
  }
  int i = 0;
  int j = i + len_;
  while (static_cast<size_t>(i) < str.size()) {
    buf_[GetAt_(j)] = str[i];
    ++i;
    ++j;
  }
  len_ += i;
  return i;
}
*/
int WriteBuf::Get(char *s, int n) {
  if (Bad()) {
    return -1;
  }
  int i = 0;
  while (i < n && i < len_) {
    s[i] = buf_[GetAt_(i)];
    ++i;
  }
  return i;
}

std::string WriteBuf::GetString(int n) {
  if (Bad()) {
    return std::string();
  }
  int how_much = std::min(n, len_);
  std::string ret;
  ret.resize(how_much);
  int i = 0;
  while (i < how_much) {
    ret[i] = buf_[GetAt_(i)];
    ++i;
  }
  return ret;
}

int WriteBuf::Pop(int n) {
  if (Bad()) return -1;
  if (n > len_) {
    // źle
    start_ = -1;
    return -1;
  }
  len_ -= n;
  start_ = GetAt_(n);
  return 0;
}

}  // namespace tin
