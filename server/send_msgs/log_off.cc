// Copyright 2019 Piotrek

#include <iostream>
#include <utility>
#include <string>

#include "send_msgs/log_off.h"
#include "core/mquads.h"

namespace tin {
int LogOff::AddToBuf(WriteBuf *buf) {
  std::string str;
  MQ::OWO.AppendToCpp11String(&str);
  MQ::SERV_LOG_OUT.AppendToCpp11String(&str);
  return buf->Add(str);
}
}  // namespace tin