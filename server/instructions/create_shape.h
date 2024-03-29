// Copyright 2019 TIN

#ifndef SERVER_INSTRUCTIONS_CREATE_SHAPE_H_
#define SERVER_INSTRUCTIONS_CREATE_SHAPE_H_

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>


#include "core/instr_struct.h"
#include "core/nquad.h"
#include "core/ndouble.h"
#include "image/image_types.h"
#include "core/username.h"

namespace tin {
class CreateShape : public InstrStruct {
 public:
  enum {
    START = INSTR + NQS,
    SHAPE_THINGS = START + NQS,

    COLORS_OFFSET = SHAPE_THINGS,
    STROKE_WIDTH_OFFSET = SHAPE_THINGS + sizeof(Color[2]),
    X_OFFSET = STROKE_WIDTH_OFFSET + sizeof(NDouble),
    Y_OFFSET = X_OFFSET + sizeof(NDouble),
    WIDTH_OFFSET = Y_OFFSET + sizeof(NDouble),
    HEIGHT_OFFSET = WIDTH_OFFSET + sizeof(NDouble),

    LINE_STROKE_COLOR_OFFSET = SHAPE_THINGS,
    LINE_STROKE_WIDTH_OFFSET = SHAPE_THINGS + sizeof(Color),
    LINE_POINT_COUNT_OFFSET = LINE_STROKE_WIDTH_OFFSET + sizeof(NDouble),
    LINE_FIRST_OFFSET = LINE_POINT_COUNT_OFFSET + sizeof(NQuad),
    LINE_SHIFT = 2 * sizeof(NDouble),
  };

  CreateShape() : shape_type_(), points_readd_(0) {}
  virtual ~CreateShape() {}
  virtual int Fn(Server *, SocketStuff *, World *);

 private:
  enum class Stage {
    BAD,
    READ_TYPE,
    DEAL_WITH_SHAPE
  };
  static constexpr Stage GetStage_(int offset) {
    return offset < START ? Stage::BAD :
      offset < SHAPE_THINGS ? Stage::READ_TYPE :
      Stage::DEAL_WITH_SHAPE;
  }
  Username un_;
  NQuad shape_type_;
  NQuad colors_[2];
  NQuad point_count_;
  NDouble stroke_width_;
  size_t points_readd_;


  NDouble x_, y_, width_, height_;
  std::vector<Vec2> points_;
};
}  // namespace tin

#endif  // SERVER_INSTRUCTIONS_CREATE_SHAPE_H_
