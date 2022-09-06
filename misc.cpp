#include "misc.h"

namespace tinybot {

std::string doubleToString(double x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

std::string intToString(int x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

std::string int64ToString(int64_t x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

std::string trim(const std::string& s, const char* delims) {
  size_t p2 = s.find_last_not_of(delims);
  if (p2 == std::string::npos)
    return {};
  size_t p1 = s.find_first_not_of(delims);
  if (p1 == std::string::npos)
    p1 = 0;

  return s.substr(p1, (p2 - p1) + 1);
}

std::vector<std::string> split(const std::string& s) {
  std::istringstream in(s);
  std::string token;
  std::vector<std::string> tokens;
  while (in >> token) {
    token = trim(token);
    tokens.push_back(token);
  }
  return tokens;
}

std::vector<std::string> split(const std::string& s, char delim) {
  std::istringstream in(s);
  std::string token;
  std::vector<std::string> tokens;
  while (getline(in, token, delim))
    tokens.push_back(token);
  return tokens;
}

size_t findFirstGt(const double* arr, double x, size_t low, size_t high) {
  if (low >= high) {
    return high;
  }
  size_t mid = (low + high) / 2;
  return arr[mid] > x ? findFirstGt(arr, x, low, mid) : findFirstGt(arr, x, mid + 1, high);
}

}  // namespace tinybot
