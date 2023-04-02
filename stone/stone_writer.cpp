#include "stone_writer.hpp"

#include "header.hpp"

#include "bee/binary_format.hpp"
#include "bee/file_writer.hpp"
#include "bee/hash_functions.hpp"
#include "yasf/cof.hpp"

#include <cstdint>

using bee::BinaryFormat;
using bee::HashFunctions;
using std::max;
using std::string;
using std::vector;

namespace stone {

struct TableSlot {
  vector<StoneWriter::value_type> values;

  string serialized;
};

bee::OrError<bee::Unit> StoneWriter::write(
  const bee::FilePath& filename, const vector<value_type>& values)
{
  uint64_t table_size = max<uint64_t>(1, values.size() / 2);
  vector<TableSlot> table;
  table.resize(table_size);

  for (auto& value : values) {
    auto hash = HashFunctions::simple_string_hash(value.first);
    auto pos = hash % table_size;
    table.at(pos).values.push_back(value);
  }

  bail(writer, bee::FileWriter::create(filename));

  {
    bee::DataBuffer header;
    BinaryFormat::write_uint64(header, Header::header_code_v1);
    BinaryFormat::write_uint64(header, table_size);
    bail_unit(writer->write(header));
  }

  uint64_t offset = 0;
  for (auto& slot : table) {
    if (!slot.values.empty()) {
      slot.serialized = yasf::Cof::serialize(slot.values);
    }
    {
      bee::DataBuffer slot_header;
      BinaryFormat::write_uint64(slot_header, offset);
      BinaryFormat::write_uint32(slot_header, slot.serialized.size());
      bail_unit(writer->write(slot_header));
    }
    offset += slot.serialized.size();
  }
  for (auto& slot : table) {
    if (slot.serialized.empty()) continue;
    bail_unit(writer->write(slot.serialized));
  }
  return bee::ok();
}

}; // namespace stone
