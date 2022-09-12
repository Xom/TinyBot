#ifndef TINYBOT_SEARCH_H
#define TINYBOT_SEARCH_H

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "dirichlet/dirichlet.h"
#include "pcg/pcg_random.h"

#include "board.h"
#include "constants.h"
#include "deck.h"
#include "infer.h"
#include "misc.h"

namespace tinybot {

struct Node {
  Node() = default;
  explicit Node(Board& other);

  void populateDraw();
  // methods on priors assume !empty()
  void uniformPriors(pcg32& rng);
  void logitsToPriors(pcg32& rng, bool is_root);
  void normalizePriors(bool do_sort);
  void noisifyPriors(pcg32& rng, bool do_temperature);
  void sortMoves();
  int selectChild(bool apply_coef_unvisited, double* coefs_explore);
  int getSearchThreshold(const int* search_thresholds) const;
  double getCoefExplore(const double* coefs_explore) const;
  double getCoefUnvisited(const double* coefs_explore) const;
  double getCoefUnvisited(double coef_explore) const;

  Board board;
  std::vector<int> moves;
  std::vector<double> priors;
  std::vector<std::shared_ptr<Node>> children;
  int move{-1};
  int visits{0};
  double sum{0};
};

class Game {
 public:
  Game() = default;
  Game(const Game&) = delete;
  Game& operator=(const Game&) = delete;

  void reset();
  void doOffer(pcg32& rng, IDeck& deck);
  void doPlace(int z, int t);
  void doPlace(int move);
  void doPlace(pcg32& rng, std::shared_ptr<Node> child, double* coefs_explore);  // must use this version if child exists
  void doDraw(int move);
  void doDraw(pcg32& rng, std::shared_ptr<Node> child, double* coefs_explore);  // must use this version if child exists
  void reportVisits(int selected_move, double* coefs_explore);
  void start(pcg32& rng, IDeck& deck);
  void restart(pcg32& rng, IDeck& deck, std::ofstream& out_file, bool stuck);

  std::shared_ptr<Node> root;
  std::shared_ptr<Node> sim;
  std::string record;
  std::vector<std::shared_ptr<Node>> stack;
  InferenceTicket ticket{-1};
  int random_moves{0};
  unsigned long seed{};
  unsigned long sim_stream{0};
  PcgDeck sim_deck{pcg32()};  // dummy initial value
  pcg32 sim_rng_place{};
  pcg32 sim_rng_draw{};
};

class SearchManager {
 public:
  SearchManager(int st, const char* trt_filename);

  static unsigned long stringToSeed(const std::string& filename);
  std::string threadInfo(const std::string& filename, int thread_id, const int* search_thresholds, const double* coefs_explore) const;
  [[noreturn]] void search(const std::string& filename, int thread_id) const;
  void run();

 private:
  int search_threads;
  std::unique_ptr<InferenceManager> inference_manager;
};

}  // namespace tinybot

#endif  // TINYBOT_SEARCH_H
