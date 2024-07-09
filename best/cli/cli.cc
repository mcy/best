#include "best/cli/cli.h"

#include "best/text/format.h"

namespace best {

namespace {
inline constexpr best::str Reset = "\N{ESCAPE}[0m";
inline constexpr best::str Red = "\N{ESCAPE}[31m";
}  // namespace

void cli::error::print_and_exit(int bad_exit) const {
  if (is_fatal()) {
    best::eprintln("{}{}{}", Red, message(), Reset);
    std::exit(128);
  } else {
    best::println("{}", message());
    std::exit(0);
  }
}
}  // namespace best