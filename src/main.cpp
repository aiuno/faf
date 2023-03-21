#include <cstddef>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "couriers/common.h"
#include "couriers/google.h"
#include "util.h"

enum class MODE { DOWNLOAD, REMOVE, SEARCH, NONE };

void print_usage() {
  std::cout << "Usage:\n"
            << "    faf <operation> <options> [...]\n\n"
            << "operations:\n"
            << "    faf -S [fonts]                   Download font(s)\n"
            << "    faf -R [fonts]                   Remove installed font(s)\n"
            << "    faf -Q [fonts]                   Search for font(s)\n"
            << "\noptions:\n"
            << "    --system                         Install fonts for all user\n"
            << "    --ignore <variant>(,variant)     Ignore a font variant\n"
            << "    --attend <weight>(,<weight>)     Download \"extra\" font weights\n"
            << "\n"
            << "    extra weights:\n"
            << "        thin                         (100)\n"
            << "        extralight                   (200)\n"
            << "        light                        (300)\n"
            << "        medium                       (500)\n"
            << "        semibold                     (600)\n"
            << "        extrabold                    (800)\n"
            << "        heavy                        (900)\n"
            << "    variants:\n"
            << "        regular\n"
            << "        italic\n"
            << "        bold" << std::endl;
}

int main(int argc, char *argv[]) {
  std::string homedir = faf::Util::get_home_dir();

  const std::filesystem::path faf_cfg_dir(homedir + "/.config/faf");

  if (!std::filesystem::exists(faf_cfg_dir)) {
    std::filesystem::create_directory(faf_cfg_dir);
  }

  // TODO: config file

  int mode_supplied = 0;
  MODE cur_mode = MODE::NONE;

  std::vector<std::string> items;

  // TODO: https://www.fontsquirrel.com/blog/2010/12/the-font-squirrel-api
  // TODO: if (config.google.enabled)
  faf::Google gfonts;

  bool system_wide = false;
  bool ignore_italic = false;
  bool ignore_regular = false;
  bool ignore_bold = false;

  std::vector<std::string> extra_weights;

  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]).compare("-S") == 0) {
      cur_mode = MODE::DOWNLOAD;
      mode_supplied++;
    } else if (std::string(argv[i]).compare("-R") == 0) {
      cur_mode = MODE::REMOVE;
      mode_supplied++;
    } else if (std::string(argv[i]).compare("-Q") == 0) {
      cur_mode = MODE::SEARCH;
      mode_supplied++;
    } else if (std::string(argv[i]).compare("-h") == 0) {
      print_usage();
      return 0;
    } else if (std::string(argv[i]).compare("--system") == 0) {
      if (homedir != "/root/") {
        std::cout
            << "Error: faf must run as root (e.g sudo) to install fonts with --system"
            << std::endl;
        exit(99);

        if (!system_wide) {
          system_wide = true;
        }
      } else {
        std::cout << "--system supplied more than once. This is not needed" << std::endl;
      }
    } else if (std::string(argv[i]).compare("--ignore") == 0) {
      if (std::vector<std::string>(argv + 1, argv + argc).size() > i) {
        size_t pos = 0;
        std::vector<std::string> ignores;

        std::string cur = std::string(argv[i + 1]);
        while ((pos = cur.find(',')) != std::string::npos) {
          ignores.push_back(cur.substr(0, pos));
          cur.erase(0, pos + 1);
        }

        if (cur.size() > 0) {
          ignores.push_back(cur);
        }

        for (const auto &ignore : ignores) {
          if (ignore == "regular") {
            ignore_regular = true;
          } else if (ignore == "italic") {
            ignore_italic = true;
          } else if (ignore == "bold") {
            ignore_bold = true;
          } else {
            std::cout << "Error: --ignore supplied without a valid argument\n"
                      << "Valid arguments are: regular, italic, bold" << std::endl;
            exit(15);
          }
          i++;
        }
      } else {
        std::cout << "Error: --ignore supplied without an argument" << std::endl;
        exit(16);
      }
    } else if (std::string(argv[i]).compare("--attend") == 0) {
      if (std::vector<std::string>(argv + 1, argv + argc).size() > i) {
        size_t pos = 0;

        std::string cur = std::string(argv[i + 1]);
        while ((pos = cur.find(',')) != std::string::npos) {
          std::string cuh = cur.substr(0, pos);
          if ((cuh == "thin" || cuh == "extralight" || cuh == "light" ||
               cuh == "medium" || cuh == "semibold" || cuh == "extrabold" ||
               cuh == "black") &&
              !std::count(extra_weights.begin(), extra_weights.end(), cuh)) {
            extra_weights.push_back(cuh);
          }
          cur.erase(0, pos + 1);
        }

        if (cur.size() > 0) {
          extra_weights.push_back(cur);
        }

      } else {
        std::cout << "Error: --attend supplied without an argument" << std::endl;
        exit(16);
      }
      i++;
    } else {
      auto arg = std::string(argv[i]);
      std::transform(arg.begin(), arg.end(), arg.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      for (int i = 0; i < arg.size(); i++) {
        if (arg[i] == '-') {
          arg[i] = ' ';
        }
      }
      items.push_back(arg);
    }
  }

  if (mode_supplied > 1) {
    std::cout << "Error: Only one operation can be used at a time" << std::endl;
    exit(11);
  } else if (items.empty()) {
    std::cout << "Error: No fonts specified (use -h for help)" << std::endl;
    exit(13);
  } else if (mode_supplied == 0) {
    std::cout << "Error: No operation specified (use -h for help)" << std::endl;
    exit(12);
  }

  switch (cur_mode) {
  case MODE::SEARCH: {
    auto res = gfonts.search(items);
    std::string cur_font_name;
    int i;
    bool has_italic = false;
    bool has_regular = false;
    for (const auto &font : res) {
      if (font.name != cur_font_name) {
        std::cout << (cur_font_name.empty()
                          ? ""
                          : std::string("\nVariants:") +
                                (has_regular ? has_italic ? " regular," : " regular"
                                             : "") +
                                (has_italic ? " italic" : "") + "\n\n")
                  << "\033[92mFound:    " << font.name << "\033[0m"
                  << "\nWeights:  "
                  << (font.weight.find("italic") != std::string::npos ? "" : font.weight)
                  << " ";

        has_regular = false;
        has_italic = false;
        i = 1;
      } else {
        if (font.prop.find("italic") != std::string::npos) {
          has_italic = true;
          continue;
        }
        if (font.prop.find("regular") != std::string::npos) {
          has_regular = true;
          continue;
        }
        if (font.weight.find("italic") == std::string::npos) {
          std::cout << (i % 3 == 0 ? "\n          " : "") << font.weight << " ";
          i++;
        }
      }
      cur_font_name = font.name;
    }
    std::cout << std::string("\nVariants:") +
                     (has_regular ? has_italic ? " regular," : " regular" : "") +
                     (has_italic ? " italic" : "")
              << std::endl;

    break;
  }

  case MODE::DOWNLOAD: {
    auto res = gfonts.search(items);
    int dl = 0;
    for (const auto &font : res) {
      if (ignore_regular && (font.weight != "bold" || !font.prop.ends_with("italic")) &&
          font.prop == "regular") {
        continue;
      }
      if (ignore_bold && font.weight == "bold") {
        continue;
      }

      if (ignore_italic && font.prop == "italic") {
        continue;
      }

      if (font.weight != "regular" && font.weight != "bold" && font.prop != "italic" &&
          !std::count(extra_weights.begin(), extra_weights.end(), font.weight)) {
        continue;
      }

      if (!faf::Common::download_font(font, system_wide)) {
        std::cout << "\033[91mError: could not download font: '" << font.name
                  << "'\n\033[0m";
      } else {
        dl++;
      }
    }
    if (dl == 0) {
      std::cout << "\033[93mNo fonts downloaded\033[0m" << std::endl;
    }

    break;
  }
  
  case MODE::REMOVE: {
      for (const auto &font : items) {
        std::uintmax_t cnt;

        if (!ignore_regular && !ignore_italic && !ignore_bold) {
          cnt = faf::Common::remove_font_family(font, system_wide); // remove everything
        } else {
          if (!ignore_regular) {
            faf::Common::remove_single_font(font, "regular", system_wide);
            cnt++;
          }
          if (!ignore_italic) {
            faf::Common::remove_single_font(font, "italic", system_wide);
            cnt++;
          }
          if (!ignore_bold) {
            faf::Common::remove_single_font(font, "bold", system_wide);
            cnt++;
          }
        }

        if (cnt == 0) {
          std::cout << "\033[91mError: could not remove font: '" << font << "'. (Probably because it doesn't exist)\n\033[0m";
        } else {
          std::cout << "Removed " << cnt << " fonts in family: '" << font << "'\n";;
        }
      }
      break;
    }

  case MODE::NONE:
    break;
  }

  return 0;
}