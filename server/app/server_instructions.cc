// Copyright 2019 Piotrek

#include "core/nquad.h"
#include "core/mquads.h"

#include "instructions/capture_session.h"
#include "instructions/say.h"
#include "instructions/move_mouse.h"

namespace tin {

const std::map<InstrId, InstrSupp> Server::instructions {
  {InstrId(MQ::CAPTURE_SESSION),
    InstrSupp(
      &CaptureSession::Fn,
      sizeof(CaptureSession),
      &CaptureSession::Construct,
      &CaptureSession::Destroy)},
/*
  {InstrId(MQ::REQUEST_LOGIN),
    InstrSupp(
      nullptr,
      0,
      nullptr,
      nullptr)},
*/
  {InstrId(MQ::SAY),
    InstrSupp(
      &Say::Fn,
      sizeof(Say),
      &Say::Construct,
      &Say::Destroy)},

  {InstrId(MQ::MOVE_MOUSE),
    InstrSupp(
      &MoveMouse::Fn,
      sizeof(MoveMouse),
      &MoveMouse::Construct,
      &MoveMouse::Destroy)},
/*
  {InstrId(NQuad(10)),
    InstrSupp(InstrSupp::EXPAND)},
    */
};

}