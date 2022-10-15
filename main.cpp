#include <cstdlib>
#include <future>
#include <iostream>
#include <string>

#include "misc.h"
#include "npy.h"
#include "search.h"

int main(const int argc, const char* argv[]) {
  if (argc < 2) {
    return tinybot::npy(false);  // converter mode, to convert game records into training data (npy format)
  }
  std::string kTrtFilename = "tb07_" + tinybot::intToString(tinybot::kBatchSize) + ".trt";
  int search_threads = atoi(argv[1]);
  if (search_threads == 0) {
    return 1;  // argv[0] not a number, TODO custom game
  }
  std::cout << search_threads << " search thread(s).\n";
  tinybot::SearchManager search_manager{search_threads, kTrtFilename.c_str()};
  search_manager.run();
  std::promise<void>().get_future().wait();  // wait forever
  return 0;
}
