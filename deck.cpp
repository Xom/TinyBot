#include "deck.h"

namespace tinybot {

Offer::Offer(const int t0, const int t1, const int z0, const int z1) {
  offer_tile[0] = t0;
  offer_tile[1] = t1;
  offer_zone[0] = z0;
  offer_zone[1] = z1;
}

Offer::Offer(const std::string& s) {
  offer_tile[0] = parseTile(s[0]);
  offer_tile[1] = parseTile(s[2]);
  offer_zone[0] = parseZone(s[1]);
  offer_zone[1] = parseZone(s[3]);
}

void PcgDeck::offer(Board* board) {
  const int n = board->deck[0];
  board->deck[0] -= 2;
  int c = rng(n);
  int d = rng(n);
  int e = rng(n - 1);
  int f = rng(n - 1);
  for (int i = kForest; i <= kBoat; ++i) {
    c -= board->deck[i];
    if (c < 0) {
      --board->deck[i];
      board->offer_tile[0] = i;
      break;
    }
  }
  for (int i = 0; i < 27; ++i) {
    d -= board->zone[i];
    if (d < 0) {
      --board->zone[i];
      board->offer_zone[0] = i;
      break;
    }
  }
  for (int i = kForest; i <= kBoat; ++i) {
    e -= board->deck[i];
    if (e < 0) {
      --board->deck[i];
      board->offer_tile[1] = i;
      break;
    }
  }
  for (int i = 0; i < 27; ++i) {
    f -= board->zone[i];
    if (f < 0) {
      --board->zone[i];
      board->offer_zone[1] = i;
      break;
    }
  }
}

FixedDeck::FixedDeck(const std::string& s) {
  init(split(s));
}

FixedDeck::FixedDeck(const std::vector<std::string>& tokens) {
  init(tokens);
}

void FixedDeck::offer(Board* board) {
  Offer my_offer = offers.front();
  board->deck[0] -= 2;
  --board->deck[my_offer.offer_tile[0]];
  --board->deck[my_offer.offer_tile[1]];
  --board->zone[my_offer.offer_zone[0]];
  --board->zone[my_offer.offer_zone[1]];
  board->offer_tile[0] = my_offer.offer_tile[0];
  board->offer_tile[1] = my_offer.offer_tile[1];
  board->offer_zone[0] = my_offer.offer_zone[0];
  board->offer_zone[1] = my_offer.offer_zone[1];
  offers.pop();
}

void FixedDeck::init(const std::vector<std::string>& tokens) {
  for (int i = 0; i <= 16; i += 2) {
    if (i >= tokens.size()) {
      return;
    }
    offers.emplace(tokens[i]);
  }
  for (int i = 19; i <= 35; i += 2) {
    if (i >= tokens.size()) {
      return;
    }
    offers.emplace(tokens[i]);
  }
  for (int i = 38; i <= 52; i += 2) {
    if (i >= tokens.size()) {
      return;
    }
    offers.emplace(tokens[i]);
  }
}

}  // namespace tinybot
