#include "common.h"
#include "../util.h"
#include "../external/p-ranav/indicators.hpp"
#include <curl/curl.h>
#include <filesystem>
#include <string>

int download_progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                               curl_off_t ultotal, curl_off_t ulnow) {
  indicators::ProgressBar *progress_bar = static_cast<indicators::ProgressBar *>(clientp);

  if (progress_bar->is_completed()) {
    ;
  } else if (dltotal == 0) {
    progress_bar->set_progress(0);
  } else {
    int percentage = static_cast<float>(dlnow) / static_cast<float>(dltotal) * 100;
    progress_bar->set_progress(percentage);
  }

  // If your callback function returns CURL_PROGRESSFUNC_CONTINUE it will
  // cause libcurl to continue executing the default progress function. return
  // CURL_PROGRESSFUNC_CONTINUE;

  return 0;
}

int download_progress_default_callback(void *clientp, curl_off_t dltotal,
                                       curl_off_t dlnow, curl_off_t ultotal,
                                       curl_off_t ulnow) {
  return CURL_PROGRESSFUNC_CONTINUE;
}

namespace faf {

bool Common::download_font(font_props font, bool system_wide) {
  std::string homedir = Util::get_home_dir();

  std::filesystem::path install_dir;

  if (system_wide) {
    install_dir = std::string("/usr/share/fonts/") + font.name + "/";
  } else {
    install_dir = (homedir + "/.fonts/" + font.name + "/");
  }

  if (!std::filesystem::exists(install_dir)) {
    std::filesystem::create_directories(install_dir);
  }

  // Hide cursor
  indicators::show_console_cursor(false);

  indicators::ProgressBar progress_bar{
      indicators::option::BarWidth{30}, indicators::option::Start{" ["},
      indicators::option::Fill{"="}, indicators::option::Lead{"="},
      indicators::option::Remainder{"-"}, indicators::option::End{"]"},
      indicators::option::PrefixText{font.name + "-" + font.prop + font.file_format},
      // indicators::option::ForegroundColor{indicators::Color::yellow},
      indicators::option::ShowElapsedTime{true},
      indicators::option::ShowRemainingTime{true},
      // indicators::option::FontStyles{
      //     std::vector<indicators::FontStyle>{indicators::FontStyle::bold}}
  };

  CURL *curl;
  FILE *fp;
  CURLcode res;
  curl = curl_easy_init();
  if (curl) {
    fp = fopen(
        (install_dir.string() + font.name + "-" + font.prop + font.file_format).c_str(),
        "wb");
    curl_easy_setopt(curl, CURLOPT_URL, font.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, download_progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, static_cast<void *>(&progress_bar));
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    // Perform a file transfer synchronously.
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);
  }

  // Show cursor
  indicators::show_console_cursor(true);

  if (res == CURLE_OK) {
    return true;
  } else {
    return false;
  }
}

std::uintmax_t Common::remove_font_family(std::string font_name, bool system_wide) {
  std::string homedir = Util::get_home_dir();

  for (size_t i = 0; i < font_name.size(); i++) {
    if (font_name[i] == ' ')
       font_name[i] = '-';
  }

  std::string fp = "";

  if (system_wide) {
    fp = "/usr/share/fonts/" + font_name;
  } else {
    fp = std::string(homedir) + "/.fonts/" + font_name;
  }

  return std::filesystem::remove_all(fp); // returns 0 (which is false) if nothing was deleted
}

bool Common::remove_single_font(std::string font_name, std::string font_type, bool system_wide) {
  std::string homedir = Util::get_home_dir();

  for (size_t i = 0; i < font_name.size(); i++) {
    if (font_name[i] == ' ')
       font_name[i] = '-';
  }

  std::string fp = "";

  // FIXME: get file extensions "more smarter"
  if (system_wide) {
    fp = "/usr/share/fonts/" + font_name + "/" + font_name + "-" + font_type + ".ttf";
  } else {
    fp = std::string(homedir) + "/.fonts/" + font_name + "/" + font_name + "-" + font_type + ".ttf";
  }

  return std::filesystem::remove(fp); 
}

} // namespace faf
