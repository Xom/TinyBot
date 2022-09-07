#ifndef TINYBOT_SEARCH_H
#define TINYBOT_SEARCH_H

#include <cmath>
#include <ctime>
#include <fstream>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

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
  void normalizePriorsAndSortMoves(pcg32& rng, bool boost);  // assumes !moves.empty()
  void unboost();
  int selectChild(double coef_unvisited);

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
  void doPlace(std::shared_ptr<Node> child);  // must use this version if child exists
  void doDraw(int move);
  void doDraw(std::shared_ptr<Node> child);  // must use this version if child exists
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
  [[noreturn]] void search(const std::string& filename) const;
  //  [[noreturn]] void searchPurePolicy(const std::string& filename) const;
  void run();

  int search_threads;
  std::unique_ptr<InferenceManager> inference_manager;  // TODO make private after testing
};

}  // namespace tinybot

#endif  // TINYBOT_SEARCH_H
