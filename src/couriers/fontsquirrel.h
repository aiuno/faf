#pragma once

#include <string>
#include <vector>

#include "common.h"

namespace faf {
struct font_props;

class FontSquirrel {
public:
  FontSquirrel() = default;
  ~FontSquirrel() = default;
  
  std::vector<font_props> search(std::vector<std::string> query);
};

} // namespace faf