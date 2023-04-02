#pragma once

#include "bee/error.hpp"
#include "bee/file_path.hpp"

#include <string>
#include <utility>
#include <vector>

namespace stone {

struct StoneWriter {
 public:
  using value_type = std::pair<std::string, std::string>;

  static bee::OrError<bee::Unit> write(
    const bee::FilePath& filename, const std::vector<value_type>& values);

 private:
};

} // namespace stone
