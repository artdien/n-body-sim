#pragma once

#include <string>
#include <string_view>

#include "platform/types.hpp"

namespace nbodysim::utils {

/// Replaces all occurrences of a specified substring within a given text with another string.
///
/// This function calculates the required memory upfront to ensure that only a single
/// allocation is performed for the resulting string.
///
/// @param source The source string to be processed.
/// @param to_replace The substring to search for.
/// @param replace_with The substring to replace with.
/// @return A new string containing the source with all replacements applied.
inline auto replace_all(std::string_view source, std::string_view to_replace, std::string_view replace_with) -> std::string {
  if (to_replace.empty()) {
    return std::string(source);
  }

  usize occurrences {0uz};
  for (auto position {source.find(to_replace)}; position != std::string_view::npos; position = source.find(to_replace, position + to_replace.size())) {
    ++occurrences;
  }

  // Allocate the result string once regardless of how many occurrences are found
  auto result {std::string {}};
  auto result_size {source.size() + occurrences * (replace_with.size() - to_replace.size())};
  result.reserve(result_size);

  auto last_position {0uz};
  auto current_position {source.find(to_replace)};
  while (current_position != std::string_view::npos) {
    result.append(source.substr(last_position, current_position - last_position));
    result.append(replace_with);

    last_position = current_position + to_replace.size();
    current_position = source.find(to_replace, last_position);
  }
  result.append(source.substr(last_position));

  return result;
}

} // namespace nbodysim::utils
