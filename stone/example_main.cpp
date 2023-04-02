#include "stone_reader.hpp"
#include "stone_writer.hpp"

#include "bee/format.hpp"
#include "bee/format_optional.hpp"
#include "command/command_builder.hpp"
#include "command/group_builder.hpp"

using bee::format;
using bee::print_line;
using std::pair;
using std::string;
using std::vector;

namespace stone {
namespace {

bee::OrError<bee::Unit> write_stone(const bee::FilePath& stone_file)
{
  vector<pair<string, string>> values;

  for (int64_t i = 0; i < 1000000; i++) {
    values.emplace_back(format("$", i), format("$", i * i));
  }

  bail_unit(stone::StoneWriter::write(stone_file, values));

  return bee::ok();
}

bee::OrError<bee::Unit> read_stone(const bee::FilePath& stone_file)
{
  bail(stone, stone::StoneReader::open(stone_file));
  print_line("Read file with table size: $", stone->table_size());

  bail(all, stone->read_all());

  for (const auto& [key, value] : all) {
    print_line(string(80, '='));
    print_line(key);
    print_line(string(80, '-'));
    print_line(value);
  }

  string key = "99";
  bail(value, stone->lookup(key));
  print_line("$:$", key, value);

  return bee::ok();
}

auto file_path_arg = command::flags::flag_of_value_type<bee::FilePath>();

command::Cmd write_stone_command()
{
  using namespace command::flags;
  auto builder = command::CommandBuilder("Write stone file");
  auto stone_file = builder.required_anon(file_path_arg, "stone-file");
  return builder.run([=]() { return write_stone(*stone_file); });
}

command::Cmd read_stone_command()
{
  using namespace command::flags;
  auto builder = command::CommandBuilder("Read stone file");
  auto stone_file = builder.required_anon(file_path_arg, "stone-file");
  return builder.run([=]() { return read_stone(*stone_file); });
}

} // namespace
} // namespace stone

int main(int argc, char** argv)
{
  return command::GroupBuilder("Stone Example")
    .cmd("write", stone::write_stone_command())
    .cmd("read", stone::read_stone_command())
    .build()
    .main(argc, argv);
}
