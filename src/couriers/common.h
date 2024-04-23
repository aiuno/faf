#pragma once

#include <cstdint>
#include <string>

namespace faf {

struct font_props {
  std::string name;
  std::string prop;
  std::string file_format;
  std::string url;
  std::string weight;
};

class Common {
public:
  static bool download_font(font_props font, bool system_wide);

  static std::uintmax_t remove_font_family(std::string font_name, bool system_wide);
  static bool remove_single_font(std::string font_name, std::string font_type,
                                 bool system_wide);

  static size_t CurlWrite_CallbackFunc_StdString(void *contents, size_t size, size_t nmemb,
                                          std::string *s);
};

} // namespace faf
