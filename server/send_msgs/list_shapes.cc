// Copyright 2019 Piotrek

#include <iostream>
#include <utility>
#include <string>

#include "send_msgs/list_shapes.h"
#include "core/mquads.h"

namespace tin {
std::string ListShapes::GetStr() const {
  std::string str;
  MQ::OWO.AppendToCppString(&str);
  MQ::SERV_LS_SHAPES.AppendToCppString(&str);
  NQuad(shapes_count_).AppendToCppString(&str);
  str.append(shapes_string_);
  return str;
}
}  // namespace tin
