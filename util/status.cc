// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include <stdio.h>
#include "port/port.h"
#include "leveldb/status.h"

namespace leveldb {
//返回一个state的拷贝
const char* Status::CopyState(const char* state) {
  uint32_t size;//32位 4个byte
  memcpy(&size, state, sizeof(size));//从state处拷贝4个byte到size，前4个byte表示message长度
  char* result = new char[size + 5]; //5表示前面的额外信息
  memcpy(result, state, size + 5); //
  return result;
}

Status::Status(Code code, const Slice& msg, const Slice& msg2) {
  assert(code != kOk);//code一定不是ok
  const uint32_t len1 = msg.size();
  const uint32_t len2 = msg2.size();
  //这里假设msg1必须不为空，如果msg2不为空要多加两个字符 : 和 ‘ ’，否则为0
  const uint32_t size = len1 + (len2 ? (2 + len2) : 0);
  char* result = new char[size + 5];
  memcpy(result, &size, sizeof(size));//前面4位时msg的长度
  result[4] = static_cast<char>(code);//然后是状态码
  memcpy(result + 5, msg.data(), len1);
  if (len2) {
    result[5 + len1] = ':';
    result[6 + len1] = ' ';
    memcpy(result + 7 + len1, msg2.data(), len2);
  }
  state_ = result;
}

std::string Status::ToString() const {
  if (state_ == NULL) {
    return "OK";
  } else {
    char tmp[30];
    const char* type;
    switch (code()) {
      case kOk:
        type = "OK";
        break;
      case kNotFound:
        type = "NotFound: ";
        break;
      case kCorruption:
        type = "Corruption: ";
        break;
      case kNotSupported:
        type = "Not implemented: ";
        break;
      case kInvalidArgument:
        type = "Invalid argument: ";
        break;
      case kIOError:
        type = "IO error: ";
        break;
      default:
        snprintf(tmp, sizeof(tmp), "Unknown code(%d): ",
                 static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    uint32_t length;
    memcpy(&length, state_, sizeof(length));
    //append第二个参数时添加的字符串长度
    result.append(state_ + 5, length);
    return result;
  }
}

}  // namespace leveldb
