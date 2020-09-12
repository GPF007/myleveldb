// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "util/arena.h"
#include <assert.h>

namespace leveldb {

static const int kBlockSize = 4096;

Arena::Arena() : memory_usage_(0) {
  alloc_ptr_ = NULL;  // First allocation will allocate a block
  alloc_bytes_remaining_ = 0;
}

Arena::~Arena() {
  for (size_t i = 0; i < blocks_.size(); i++) {
    delete[] blocks_[i];
  }
}

char* Arena::AllocateFallback(size_t bytes) {
  if (bytes > kBlockSize / 4) { //如果申请的bytes大于1kb
    // Object is more than a quarter of our block size.  Allocate it separately
    // to avoid wasting too much space in leftover bytes.
    char* result = AllocateNewBlock(bytes);//创建一个新的block，大小为需要的bytes
    return result;
  }

  //在当前的地址创建一个block，分配给需要的bytes后还会剩余一些供下次分配，
  // We waste the remaining space in the current block.
  alloc_ptr_ = AllocateNewBlock(kBlockSize); //创建一个新的block，大小为4KB，
  alloc_bytes_remaining_ = kBlockSize;

  char* result = alloc_ptr_;
  alloc_ptr_ += bytes;
  alloc_bytes_remaining_ -= bytes;
  return result;
}

//???
char* Arena::AllocateAligned(size_t bytes) {
  //在64位的机器上 align为8
  const int align = (sizeof(void*) > 8) ? sizeof(void*) : 8;
  assert((align & (align-1)) == 0);   // Pointer size should be a power of 2
  size_t current_mod = reinterpret_cast<uintptr_t>(alloc_ptr_) & (align-1);// & 0111 ,后三位是否是0
  size_t slop = (current_mod == 0 ? 0 : align - current_mod); //slop时后三位与8的差值
  size_t needed = bytes + slop;//需要多分配slop位才能够保证alloc_ptr_ 后三位为0
  char* result;
  if (needed <= alloc_bytes_remaining_) {
    result = alloc_ptr_ + slop;
    alloc_ptr_ += needed;
    alloc_bytes_remaining_ -= needed;
  } else {//这会创建一个新的块，不用理会slop
    // AllocateFallback always returned aligned memory
    result = AllocateFallback(bytes);
  }
  //新分配的后三位一定为0
  assert((reinterpret_cast<uintptr_t>(result) & (align-1)) == 0);
  return result;
}

char* Arena::AllocateNewBlock(size_t block_bytes) {
  char* result = new char[block_bytes];
  blocks_.push_back(result);
  memory_usage_.NoBarrier_Store(
      reinterpret_cast<void*>(MemoryUsage() + block_bytes + sizeof(char*)));
  return result;
}

}  // namespace leveldb
