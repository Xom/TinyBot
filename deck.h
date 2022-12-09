#ifndef TINYBOT_DECK_H
#define TINYBOT_DECK_H

#include <queue>
#include <string>
#include <vector>

#include "pcg/pcg_random.h"

#include "board.h"
#include "constants.h"
#include "misc.h"

namespace tinybot {

class Board;

struct Offer {
  Offer(int t0, int t1, int z0, int z1);
  explicit Offer(const std::string& s);

  int offer_tile[2]{};
  int offer_zone[2]{};
};

class IDeck {
 public:
  virtual void offer(Board* board) = 0;  // responsible for modifying deck, zone, offer_tile, and offer_zone only
};


class PcgDeck : public IDeck {
 public:
  explicit PcgDeck(pcg32 r)
      : rng(r){};

  void offer(Board* board) override;

  pcg32 rng;
};

class FixedDeck : public IDeck {
 public:
  explicit FixedDeck(const std::string& s);
  explicit FixedDeck(const std::vector<std::string>& tokens);

  void offer(Board* board) override;

 private:
  void init(const std::vector<std::string>& tokens);

  std::queue<Offer> offers;
};

static constexpr int kNumFixedShuffles = 64;
extern FixedDeck* kFixedShuffles[kNumFixedShuffles];

}  // namespace tinybot

#endif  // TINYBOT_DECK_H
