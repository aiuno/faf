#include "util.h"
#include <unistd.h>
#include <pwd.h>

namespace faf {

std::string Util::get_home_dir() {
  struct passwd *pw = getpwuid(getuid());
  return std::string(pw->pw_dir);
}

}

