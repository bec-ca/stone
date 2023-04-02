#pragma once

#include <cstddef>
#include <cstdint>

namespace stone {

struct Header {
 public:
  static constexpr uint64_t header_code_v1 = 0xd8418cc854956262;
  static constexpr size_t header_size = 16;

  uint64_t header;
  uint64_t table_size;
};

} // namespace stone
