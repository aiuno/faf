#include "fontsquirrel.h"

#include <curl/curl.h>

#include "../external/nlohmann/json.hpp"
#include "../external/p-ranav/indicators.hpp"

namespace faf {
using json = nlohmann::json;

std::vector<font_props> FontSquirrel::search(std::vector<std::string> query) {
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

  std::vector<font_props> fonts;

  CURL *curl_handle = curl_easy_init();
  CURLcode ret;

  std::string res;

  if (curl_handle) {
    std::string url = "https://www.fontsquirrel.com/api/fontlist/all";
    // for (const auto &q : query) {
    //   url += "/" + q;
    // }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Common::CurlWrite_CallbackFunc_StdString);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &res);

    ret = curl_easy_perform(curl_handle);

    if (ret != CURLE_OK) {
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(ret) << std::endl;
    }

    curl_easy_cleanup(curl_handle);
  }

  json j = json::parse(res);

  std::vector<std::string> error_fonts;

  for (const auto &q : query) {
    std::cout << "Searching for font: " << q << "\n";
    bool found = false;
    for (const auto &font : j) {
      auto at = std::string(font["family_name"]);
      std::transform(at.begin(), at.end(), at.begin(),
                     [](unsigned char c) { return std::tolower(c); });
      
      if (at.find(q) == 0) {
        for (int i = 0; i < at.size(); i++) {
          if (at[i] == ' ') {
            at[i] = '-';
          }
        }

        font_props f;
        f.name = at;
        f.prop = font["is_monospace"] == "N" ? "proportional" : "monospace";
        f.file_format = std::string(font["font_filename"]).substr(
            std::string(font["font_filename"]).find_last_of("."));
        f.url = "https://www.fontsquirrel.com/fonts/download/" + std::string(font["family_urlname"]);

        fonts.push_back(f);
        found = true;
      }
    }

    if (!found) {
      error_fonts.push_back(q);
    }
  }

  spinner.mark_as_completed();
  thread.join();
  indicators::show_console_cursor(true);

  for (const auto &font : error_fonts) {
    std::cout << "\033[91mError (FontSquirrel): could not find font with the name '" << font
              << "'\n\033[0m";
  }

  return fonts;
}

} // namespace faf