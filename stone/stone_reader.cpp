#include "stone_reader.hpp"

#include "bee/binary_format.hpp"
#include "bee/data_buffer.hpp"
#include "bee/file_descriptor.hpp"
#include "bee/hash_functions.hpp"
#include "header.hpp"
#include "yasf/cof.hpp"

#include <memory>
#include <vector>

using bee::BinaryFormat;
using bee::FileDescriptor;
using bee::HashFunctions;
using std::make_shared;
using std::optional;
using std::string;

namespace stone {

namespace {

struct StoneReaderImpl : public StoneReader {
 public:
  virtual ~StoneReaderImpl() {}

  static constexpr int slot_size = 12;

  static bee::OrError<StoneReader::ptr> open(const bee::FilePath& filename)
  {
    bail(fd, FileDescriptor::open_file(filename));
    bee::DataBuffer buf;
    bail_unit(fd.read(buf, 16));
    if (buf.size() < 16) { return bee::Error("Incomplete header"); }

    auto code = BinaryFormat::read_uint64(buf);
    auto table_size = BinaryFormat::read_uint64(buf);

    if (code != Header::header_code_v1) {
      return bee::Error::format(
        "Invalid header code, got $, wanted $", code, Header::header_code_v1);
    }

    return make_shared<StoneReaderImpl>(std::move(fd), table_size);
  }

  bee::OrError<value_type> read_slot(uint64_t idx)
  {
    uint64_t slot_offset = Header::header_size + idx * slot_size;
    bail_unit(_fd.seek(slot_offset));

    bee::DataBuffer buf;
    bail_unit(_fd.read(buf, slot_size));
    if (buf.size() < slot_size) { return bee::Error("Truncated file"); }

    uint64_t data_offset = BinaryFormat::read_uint64(buf) +
                           Header::header_size + _table_size * slot_size;
    uint32_t data_size = BinaryFormat::read_uint32(buf);

    if (data_size == 0) { return value_type(); }

    bail_unit(_fd.seek(data_offset));
    bail(ret, _fd.read(buf, data_size));
    if (ret.bytes_read() < data_size) { return bee::Error("Truncated file"); }

    return yasf::Cof::deserialize<value_type>(buf.to_string());
  }

  virtual bee::OrError<optional<std::string>> lookup(
    const std::string& key) override
  {
    auto hash = HashFunctions::simple_string_hash(key);
    uint64_t table_idx = hash % _table_size;

    bail(values, read_slot(table_idx));

    for (const auto& [k, v] : values) {
      if (k == key) { return v; }
    }

    return std::nullopt;
  }

  virtual bee::OrError<value_type> read_all() override
  {
    value_type output;
    for (uint64_t i = 0; i < _table_size; i++) {
      bail(values, read_slot(i));
      for (auto& v : values) { output.emplace_back(std::move(v)); }
    }
    return output;
  }

  virtual size_t table_size() const override { return _table_size; }

  StoneReaderImpl(FileDescriptor&& fd, uint64_t table_size)
      : _fd(std::move(fd)), _table_size(table_size)
  {}

 private:
  FileDescriptor _fd;
  uint64_t _table_size;
};

} // namespace

StoneReader::~StoneReader() {}

bee::OrError<StoneReader::ptr> StoneReader::open(const bee::FilePath& filename)
{
  return StoneReaderImpl::open(filename);
}

} // namespace stone
