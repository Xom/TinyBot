// contains a significant amount of code from KataGo (see katago/LICENSE)

#ifndef TINYBOT_MISC_H
#define TINYBOT_MISC_H

#include <algorithm>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace tinybot {

struct StringError : public std::exception {
  std::string message;
  StringError(const char* m)
      : exception(), message(m) {}
  StringError(const std::string& m)
      : exception(), message(m) {}
  const char* what() const throw() final { return message.c_str(); }
};

std::string doubleToString(double x);
std::string intToString(int x);
std::string int64ToString(int64_t x);

std::string trim(const std::string& s, const char* delims = " \t\r\n\v\f");
std::vector<std::string> split(const std::string& s);              // Split string into tokens, trimming off whitespace
std::vector<std::string> split(const std::string& s, char delim);  // Split string based on the given delim, no trimming

// Assumes arr is sorted.
// Finds the first index i within [low,high) where arr[i] > x, or high if such an index does not exist.
size_t findFirstGt(const double* arr, double x, size_t low, size_t high);

}  // namespace tinybot

#endif  // TINYBOT_MISC_H
