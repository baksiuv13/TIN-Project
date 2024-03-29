// Copyright 2019 Piotrek

#ifndef SERVER_SEND_MSGS_USER_MSG_H_
#define SERVER_SEND_MSGS_USER_MSG_H_

#include <string>
#include <list>

#include "core/out_message.h"
#include "core/write_buf.h"

namespace tin {

// This is class of message that shall be broadcast.
class UserMsg : public OutMessage {
 public:
  UserMsg() {}
  UserMsg(const Username &un, const std::string &content)
      : username_(un), content_(content) {}

  virtual ~UserMsg() {}

  virtual std::string GetTypeName() const {return "UserMsg";}

  virtual int Audience() const {return BROADCAST_U;}

  Username GetUsername() const {return username_;}
  const std::string &GetContent() const {return content_;}

  virtual std::string GetStr() const;
 private:
  Username username_;
  std::string content_;
};  // class UserMsg
}  // namespace tin

#endif  // SERVER_SEND_MSGS_USER_MSG_H_
