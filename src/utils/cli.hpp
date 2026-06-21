#pragma once

#include <algorithm>
#include <charconv>
#include <optional>
#include <span>
#include <string_view>
#include <system_error>

#include "platform/types.hpp"

namespace nbodysim::utils {

inline std::optional<u32> parse_cli_argument(i32 argc, c8* argv[], std::string_view argument) {
  const auto arguments {std::span {argv, argv + argc}};
  const auto arguments_end {std::ranges::end(arguments)};

  if (auto it {std::ranges::find(arguments, argument)}; it != arguments_end && ++it != arguments_end) {
    const auto as_string {std::string_view {*it}};
    u32 parsed;

    if (const auto result {std::from_chars(as_string.data(), as_string.data() + as_string.size(), parsed)};
        result.ec != std::errc::invalid_argument) {
      return std::make_optional(parsed);
    }
  }

  return {};
}

} // namespace nbodysim::utils
