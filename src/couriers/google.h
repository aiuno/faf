#include <filesystem>
#include <string>
#include <vector>

namespace faf {
struct font_props;

class Google {
public:
  Google();

  std::string get_api_key();

  std::vector<font_props> search(std::vector<std::string> query);

private:
  std::string api_key;

  bool add_api_key(std::filesystem::path api_key_file);
  bool check_api_key(std::filesystem::path api_key_file);

  std::string *get_fonts();
};

} // namespace faf