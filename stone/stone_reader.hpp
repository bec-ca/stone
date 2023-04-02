#pragma once

#include "bee/data_buffer.hpp"
#include "bee/error.hpp"
#include "bee/file_path.hpp"

#include <memory>
#include <vector>

namespace stone {

struct StoneReader {
 public:
  using value_type = std::vector<std::pair<std::string, std::string>>;

  using ptr = std::shared_ptr<StoneReader>;

  static bee::OrError<ptr> open(const bee::FilePath& filename);

  virtual ~StoneReader();

  virtual bee::OrError<std::optional<std::string>> lookup(
    const std::string& key) = 0;

  virtual bee::OrError<value_type> read_all() = 0;

  virtual size_t table_size() const = 0;

 private:
};

} // namespace stone
