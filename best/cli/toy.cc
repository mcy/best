#include "best/cli/app.h"
#include "best/cli/toy_flags.h"

//! A very simply binary for demonstrating best's CLI library in action.

namespace best::cli_toy { 
best::app CliToy = [](MyFlags& flags) { best::println("{:#?}", flags); };
}  // namespace best::cli_toy