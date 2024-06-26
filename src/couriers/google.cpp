#include "google.h"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <fstream> // IWYU pragma: keep
#include <initializer_list>
#include <iostream>
#include <new>
#include <string>
#include <thread>
#include <vector>

#include "../external/nlohmann/json.hpp"
#include "../external/p-ranav/indicators.hpp"
#include "../util.h"

namespace faf {
using json = nlohmann::json;

Google::Google() {
  std::string homedir = Util::get_home_dir();

  const std::filesystem::path config_path(homedir + "/.config/faf/config.json");

  json cfg;

  if (std::filesystem::exists(config_path)) {
    std::ifstream stream(config_path);
    stream >> cfg;
  } else {
    std::cerr << "Config file not found" << std::endl;
    exit(1);
  }

  if (cfg.contains("google")) {
    if (cfg["google"].contains("api_key") && !std::string(cfg["google"]["api_key"]).empty()) {
      api_key = cfg["google"]["api_key"];
    } else {
      add_api_key(config_path);
    }
  }
}

bool Google::add_api_key(std::filesystem::path config_path) {
  std::string api_key_url =
      "https://developers.google.com/fonts/docs/developer_api#APIKey";

  std::cout << "Google fonts API key not set...\n"
            << "You can get one from here: " << api_key_url << "\n";
  std::cout << "Enter API key: ";

  std::string key;
  std::cin >> key;

  json cfg;

  if (std::filesystem::exists(config_path)) {
    std::ifstream stream(config_path);
    stream >> cfg;
  } else {
    std::cerr << "Config file not found" << std::endl;
    exit(1);
  }

  cfg["google"]["api_key"] = key;

  std::ofstream stream(config_path);
  stream << cfg.dump(2);

  api_key = key;

  return true;
}

std::vector<font_props> Google::search(std::vector<std::string> query) {
  indicators::show_console_cursor(false);
  indicators::ProgressSpinner spinner{
      indicators::option::PostfixText{"Searching..."},
      indicators::option::ForegroundColor{indicators::Color::yellow},
      indicators::option::ShowPercentage{false},
      indicators::option::SpinnerStates{
          std::vector<std::string>{"◜", "◠", "◝", "◞", "◡", "◟"}},
      indicators::option::FontStyles{
          std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}};

  auto job = [&spinner]() {
    while (true) {
      if (spinner.is_completed()) {
        spinner.set_option(indicators::option::ForegroundColor{indicators::Color::green});
        spinner.set_option(indicators::option::PrefixText{"✔"});
        spinner.set_option(indicators::option::ShowSpinner{false});
        spinner.set_option(indicators::option::ShowPercentage{false});
        spinner.set_option(indicators::option::PostfixText{"Search completed"});
        spinner.mark_as_completed();
        break;
      } else
        spinner.tick();
      std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
    std::cout << "\n";
  };

  std::thread thread(job);

  CURL *curl_handle = curl_easy_init();
  CURLcode ret;

  std::string res;

  if (curl_handle) {
    curl_easy_setopt(curl_handle, CURLOPT_URL,
                     std::string("https://www.googleapis.com/webfonts/v1/webfonts?key=" +
                                 this->get_api_key())
                         .c_str());
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION,
                     Common::CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &res);

    ret = curl_easy_perform(curl_handle);

    curl_easy_cleanup(curl_handle);
  }

  json j = json::parse(res);
  std::vector<font_props> rr;

  std::vector<std::string> error_fonts;

  for (const auto &font : query) {
    bool found = false;
    for (const auto &obj : j["items"]) {
      std::string at = obj["family"];

      std::transform(at.begin(), at.end(), at.begin(),
                     [](unsigned char c) { return std::tolower(c); });

      // FIXME: in the case of roboto-flex where there are no special weights
      //        and only regular variant. The parser fails and shows "regular"
      //        as a weight
      if (at.find(font) == 0) {
        found = true;
        for (const auto &file : obj["files"].items()) {
          std::string weight;

          if (file.key().starts_with("100")) {
            weight = "thin";
          } else if (file.key().starts_with("200")) {
            weight = "extralight";
          } else if (file.key().starts_with("300")) {
            weight = "light";
          } else if (file.key().starts_with("400")) {
            weight = "regular";
          } else if (file.key().starts_with("500")) {
            weight = "medium";
          } else if (file.key().starts_with("600")) {
            weight = "semibold";
          } else if (file.key().starts_with("700")) {
            weight = "bold";
          } else if (file.key().starts_with("800")) {
            weight = "extrabold";
          } else if (file.key().starts_with("900")) {
            weight = "black";
          }

          if (file.key().size() > 3) {
            weight += weight.empty() ? file.key()
                                     : "-" + file.key().substr(3, file.key().size());
          }

          for (int i = 0; i < at.size(); i++) {
            if (at[i] == ' ') {
              at[i] = '-';
            }
          }

          size_t pos = 0;
          size_t len = 0;
          std::string prop = weight;
          while ((pos = prop.find('-')) != std::string::npos) {
            prop.erase(0, pos + 1);
            len++;
          }
          prop.erase(0, len);

          if (!(prop == "regular" || prop == "bold" || prop == "italic"))
            prop = "";

          rr.push_back((font_props){
              .name = at,
              .prop = prop,
              .file_format = std::string(file.value())
                                 .substr(std::string(file.value()).find_last_of('.'),
                                         static_cast<std::string>(file.value()).size()),
              .url = file.value(),
              .weight = weight});
        }
      }
    }
    if (!found) {
      error_fonts.push_back(font);
    }
  }
  spinner.mark_as_completed();
  thread.join();
  indicators::show_console_cursor(true);

  for (const auto &font : error_fonts) {
    std::cout << "\033[91mError (Google): could not find font with the name '" << font
              << "'\n\033[0m";
  }

  return rr;
}

std::string Google::get_api_key() { return api_key; }

} // namespace faf
