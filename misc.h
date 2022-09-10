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

// https://stackoverflow.com/questions/17074324/how-can-i-sort-two-vectors-in-the-same-way-with-criteria-that-uses-only-one-of/17074810#17074810
template <typename T, typename Compare>
std::vector<std::size_t> sort_permutation(const std::vector<T>& vec, const Compare& compare) {
  std::vector<std::size_t> p(vec.size());
  std::iota(p.begin(), p.end(), 0);
  std::stable_sort(p.begin(), p.end(), [&](std::size_t i, std::size_t j) { return compare(vec[i], vec[j]); });
  return p;
}

template <typename T>
std::vector<T> apply_permutation(const std::vector<T>& vec, const std::vector<std::size_t>& p) {
  std::vector<T> sorted_vec(vec.size());
  std::transform(p.begin(), p.end(), sorted_vec.begin(), [&](std::size_t i) { return vec[i]; });
  return sorted_vec;
}

}  // namespace tinybot

#endif  // TINYBOT_MISC_H
