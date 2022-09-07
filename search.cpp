#include "search.h"

namespace tinybot {

Node::Node(Board& other) {
  board = other;
}

void Node::populateDraw() {
  if (board.ink != 2) {
    moves.push_back(kMovePass);
  }
  for (int z = 0; z < 81; ++z) {
    if (board.input_local[z + 648] > 0.5f) {
      moves.push_back(z);
    }
  }
}

void Node::normalizePriorsAndSortMoves(pcg32& rng, const bool boost) {
  double total = 0;
  for (double& d : priors) {
    if (boost) {
      d *= d;  // boost signal from weak net
    }
    total += d;
  }
  for (double& d : priors) {
    d /= total;
  }

  auto perm = sort_permutation(priors, std::greater<>{});
  priors = apply_permutation(priors, perm);
  moves = apply_permutation(moves, perm);

  int streak_begin = 0;
  double prev = priors[0];
  for (int i = 1; i < priors.size(); ++i) {
    if (priors[i] != prev) {
      if (i != streak_begin + 1) {
        pcg_extras::shuffle(moves.begin() + streak_begin, moves.begin() + i, rng);
      }
      streak_begin = i;
      prev = priors[i];
    }
  }
}

void Node::unboost() {
  double total = 0;
  for (double& d : priors) {
    d = sqrt(d);
    total += d;
  }
  for (double& d : priors) {
    d /= total;
  }
}

int Node::selectChild(const double coef_unvisited) {
  //  if (moves.size() == 1) {
  //    return 0;
  //  }
  const double coef_explore = kCoefExplore * sqrt(static_cast<double>(visits));
  double p_explored = 0;
  int best_i = 0;
  double best_ucb = kNegativeInfinity;
  for (int i = 0; i < priors.size(); ++i) {
    if (i == children.size() || children[i]->visits == 0) {
      return (visits == 0 || sum / visits + coef_explore * priors[i] + coef_unvisited * sqrt(p_explored) > best_ucb) ? i : best_i;
    }
    p_explored += priors[i];
    const auto& child = children[i];
    const double ucb = child->sum / child->visits + coef_explore * priors[i] / (1.0 + child->visits);
    if (ucb > best_ucb) {
      best_ucb = ucb;
      best_i = i;
    }
  }
  return best_i;
}

void Game::reset() {
  sim.reset();
  record.clear();
  stack.clear();
  random_moves = 0;
  root = std::make_shared<Node>();
}

void Game::doOffer(pcg32& rng, IDeck& deck) {
  root->board.doOffer(deck, &root->moves);
  record.push_back(kTileChars[root->board.offer_tile[0]]);
  record.push_back(kZoneChars[root->board.offer_zone[0]]);
  record.push_back(kTileChars[root->board.offer_tile[1]]);
  record.push_back(kZoneChars[root->board.offer_zone[1]]);
  record.push_back(' ');
  root->visits = 0;
  root->sum = 0;
}

void Game::doPlace(const int z, const int t) {
  record.push_back(kTileChars[t]);
  record.push_back(kZoneChars[z % 9 + 9]);
  record.push_back(kZoneChars[z / 9]);
  record.push_back(' ');
  root->board.doPlace(z, t);
  root->moves.clear();
  root->priors.clear();
  sim_stream += root->visits;
}

void Game::doPlace(const int move) {
  doPlace(move % 81, move / 81);
}

void Game::doPlace(std::shared_ptr<Node> child) {
  root = std::move(child);
  const int z = root->move % 81;
  const int t = root->move / 81;
  record.push_back(kTileChars[t]);
  record.push_back(kZoneChars[z % 9 + 9]);
  record.push_back(kZoneChars[z / 9]);
  record.push_back(' ');
  if (root->board.is_player_turn) {
    root->unboost();
  }
  sim_stream += root->visits;
}

void Game::doDraw(const int move) {
  if (move == kMovePass) {
    record.push_back(' ');
  } else {
    record.push_back(kZoneChars[move % 9 + 9]);
    record.push_back(kZoneChars[move / 9]);
  }
  root->board.doDraw(move);
  root->moves.clear();
  root->priors.clear();
  root->populateDraw();
  sim_stream += root->visits;
  // not sure whether I should modify visits; hopefully it's no big deal...
}

void Game::doDraw(std::shared_ptr<Node> child) {
  root = std::move(child);
  if (root->move == kMovePass) {
    record.push_back(' ');
  } else {
    record.push_back(kZoneChars[root->move % 9 + 9]);
    record.push_back(kZoneChars[root->move / 9]);
    root->unboost();
  }
  sim_stream += root->visits;
}

void Game::start(pcg32& rng, IDeck& deck) {
  reset();
  int r = rng(1 << 24);
  while (random_moves < r % 8) {
    doOffer(rng, deck);
    doPlace(root->moves[rng(root->moves.size())]);
    ++random_moves;
    if (random_moves == 7) {
      break;
    }
    r /= 8;
  }
  doOffer(rng, deck);
}

void Game::restart(pcg32& rng, IDeck& deck, std::ofstream& out_file, const bool stuck) {
  sim_stream += root->visits;
  root->board.calculateScore();
  record += intToString(stuck ? kStuckPenaltyInt : root->board.score[0]);
  record.push_back(' ');
  for (int i = kForest; i <= kBoat; ++i) {
    record += intToString(root->board.score[i]);
    record.push_back(' ');
  }
  record += intToString(stuck ? root->board.score[0] + root->board.score[8] - kStuckPenaltyInt : root->board.score[8]);
  record.push_back(' ');
  record += intToString(random_moves);
  record += "\n";
  out_file << record << std::flush;
  start(rng, deck);
}

SearchManager::SearchManager(const int st, const char* trt_filename)
    : search_threads(st) {
  inference_manager = std::make_unique<InferenceManager>(trt_filename);
}

unsigned long SearchManager::stringToSeed(const std::string& filename) {
  unsigned long result[1];
  pcg_extras::generate_to<1>(std::seed_seq(filename.begin(), filename.end()), result);
  return result[0];
}

[[noreturn]] void SearchManager::search(const std::string& filename) const {
  std::ofstream out_file;
  out_file.open(filename);
  unsigned long original_seed = stringToSeed(filename);
  pcg32 rng(original_seed, 0);
  PcgDeck deck(pcg32(original_seed, 1));  // TODO test rng ?= pcg32(rng())
  Game games[kConcurrentGames / search_threads];
  for (Game& game : games) {
    ++original_seed;
    game.seed = original_seed;
    game.start(rng, deck);
    game.ticket = inference_manager->request(game.root->board.input_local, game.root->board.input_global);
  }

  while (true) {
    for (Game& game : games) {
      const int offset = game.ticket.cursor * kTensorLengths[kOutputPolicy];
      float* output_policy = game.ticket.future.get()->output_policy;

      if (game.sim) {
        const bool drawing = game.sim->board.placements_until_draw == 0;
        for (const int move : game.sim->moves) {
          game.sim->priors.push_back(expf(output_policy[offset + move]));
        }
        game.sim->normalizePriorsAndSortMoves(rng, true);

        if (game.sim->board.placements_remaining == 0) {
          std::vector<std::shared_ptr<Node>> stack{};
          do {
            if (stack.empty()) {
              if (game.sim->priors.empty()) {
                if (game.sim->moves.size() == 1) {
                  if (game.sim->children.empty()) {
                    game.sim->board.doDraw(game.sim->moves[0]);
                    game.sim->moves.clear();
                    game.sim->priors.clear();
                    game.sim->populateDraw();
                    //                    for (int i = 0; i < game.sim->moves.size(); ++i) {
                    //                      game.sim->priors.push_back(1);
                    //                    }
                    //                    game.sim->normalizePriorsAndSortMoves(rng, false);
                  } else {
                    game.sim = game.sim->children[0];
                  }
                  continue;
                }
                for (int i = 0; i < game.sim->moves.size(); ++i) {
                  game.sim->priors.push_back(1);
                }
                game.sim->normalizePriorsAndSortMoves(rng, false);
              }

              const int i = game.sim->selectChild(0);  // TODO should I use coef_unvisited?

              if (i == game.sim->children.size()) {
                std::shared_ptr<Node> child = std::make_shared<Node>(game.sim->board);
                child->move = game.sim->moves[i];
                child->board.doDraw(child->move);
                if (child->board.is_player_turn) {
                  child->populateDraw();
                  for (int j = 0; j < child->moves.size(); ++j) {
                    child->priors.push_back(1);
                  }
                  child->normalizePriorsAndSortMoves(rng, false);
                }
                game.sim->children.push_back(child);
                stack.push_back(child);
                continue;
              }

              std::shared_ptr<Node> child = game.sim->children[i];

              if (child->visits < kSearchThresholdLong) {
                stack.push_back(child);
                continue;
              }

              game.sim = child;
              continue;
            }  // end if (stack.empty())

            std::shared_ptr<Node> leaf = stack.back();

            if (leaf->board.is_player_turn) {
              if (leaf->moves.empty()) {
                leaf->board.calculateScore();
                const double ev = static_cast<double>(leaf->board.score[0] + leaf->board.score[8] - kStuckPenaltyInt) / kScoreDenom;
                for (auto& node : stack) {
                  node->sum += ev;
                  ++node->visits;
                }
                game.sim->sum += ev;
                ++game.sim->visits;
                stack.clear();
                continue;
              }

              const int i = leaf->moves.size() == 1 ? 0 : leaf->selectChild(kCoefUnvisited);

              if (i == leaf->children.size()) {
                std::shared_ptr<Node> child = std::make_shared<Node>(leaf->board);
                child->move = leaf->moves[i];
                child->board.doDraw(child->move);
                if (child->board.is_player_turn) {
                  child->populateDraw();
                  for (int j = 0; j < child->moves.size(); ++j) {
                    child->priors.push_back(1);
                  }
                  child->normalizePriorsAndSortMoves(rng, false);
                }
                leaf->children.push_back(child);
                stack.push_back(child);
                continue;
              }

              stack.push_back(leaf->children[i]);
              continue;
            }

//            if (rng(1000) == 0) {
//              leaf->board.display();
//            }
            leaf->board.calculateScore();
            const double ev = static_cast<double>(leaf->board.score[8]) / kScoreDenom;
            for (auto& node : stack) {
              node->sum += ev;
              ++node->visits;
            }
            game.sim->sum += ev;
            ++game.sim->visits;
            stack.clear();
          } while (game.sim->board.is_player_turn);

          game.sim->board.calculateScore();
          const double ev = static_cast<double>(game.sim->board.score[8]) / kScoreDenom;
          for (auto& node : game.stack) {
            node->sum += ev;
            ++node->visits;
          }
          game.root->sum += ev;
          ++game.root->visits;
          game.sim.reset();
          game.stack.clear();

        } else {
          const int n = game.sim->priors.size();
          int random_i = 0;
          double r = static_cast<double>(rng()) / kPcg32MaxDouble;
          for (int i = 1; i < n; ++i) {
            if (r < game.sim->priors[i]) {
              random_i = i;
              break;
            }
            r -= game.sim->priors[i];
          }
          game.sim->priors.clear();
          if (drawing) {
            game.sim->board.doDraw(game.sim->moves[random_i]);
          } else {
            game.sim->board.doPlace(game.sim->moves[random_i] % 81, game.sim->moves[random_i] / 81);
          }

          do {
            if (game.sim->board.drawings_completed == 3) {
              game.sim->board.calculateScore();
              const double ev = static_cast<double>(game.sim->board.score[8]) / kScoreDenom;
              for (auto& node : game.stack) {
                node->sum += ev;
                ++node->visits;
              }
              game.root->sum += ev;
              ++game.root->visits;
              game.sim.reset();
              game.stack.clear();
              break;
            }
            game.sim->moves.clear();
            if (game.sim->board.is_player_turn) {
              game.sim->populateDraw();
              if (game.sim->moves.size() == 1) {
                game.sim->board.doDraw(game.sim->moves[0]);  // then repeat
              }
            } else {
              game.sim->board.doOffer(game.sim_deck, &game.sim->moves);
              if (game.sim->moves.size() == 1) {
                game.sim->board.doPlace(game.sim->moves[0] % 81, game.sim->moves[0] / 81);  // then repeat
              }
            }
          } while (game.sim->moves.size() == 1);

          if (game.sim) {
            if (!game.sim->moves.empty()) {
              game.ticket = inference_manager->request(game.sim->board.input_local, game.sim->board.input_global);
              continue;
            }
            game.sim->board.calculateScore();
            const double ev = static_cast<double>(game.sim->board.score[0] + game.sim->board.score[8] - kStuckPenaltyInt) / kScoreDenom;
            for (auto& node : game.stack) {
              node->sum += ev;
              ++node->visits;
            }
            game.root->sum += ev;
            ++game.root->visits;
            game.sim.reset();
            game.stack.clear();
          }
        }
      } else {
        std::shared_ptr<Node> leaf = game.stack.empty() ? game.root : game.stack.back();
        for (const int move : leaf->moves) {
          leaf->priors.push_back(expf(output_policy[offset + move]));
        }
        leaf->normalizePriorsAndSortMoves(rng, !game.stack.empty());
      }

      // here begins the state machine from hell
      while (true) {
        if (game.stack.empty()) {
          if (!game.root->board.is_player_turn) {
            if (game.root->board.drawings_completed == 3) {
              game.restart(rng, deck, out_file, false);
              break;
            }
            game.doOffer(rng, deck);
            if (game.root->moves.size() > 1) {
              break;
            }
            // fall through
          }

          if (game.root->moves.empty()) {
            game.restart(rng, deck, out_file, true);
            break;
          }

          if (game.root->priors.empty()) {
            if (game.root->moves.size() != 1) {
              break;
            }
            if (game.root->children.empty()) {
              if (game.root->board.placements_until_draw == 0) {
                game.doDraw(game.root->moves[0]);
              } else {
                game.doPlace(game.root->moves[0]);
              }
            } else {
              if (game.root->board.placements_until_draw == 0) {
                game.doDraw(game.root->children[0]);
              } else {
                game.doPlace(game.root->children[0]);
              }
            }
            continue;
          }

          const int i = game.root->selectChild(0);

          if (i == game.root->children.size()) {
            std::shared_ptr<Node> child = std::make_shared<Node>(game.root->board);
            child->move = game.root->moves[i];
            if (game.root->board.placements_until_draw == 0) {
              child->board.doDraw(child->move);
            } else {
              child->board.doPlace(child->move % 81, child->move / 81);
            }
            if (child->board.is_player_turn) {
              child->populateDraw();  // child is not placement, because parent is player turn therefore not offer
            }
            game.root->children.push_back(child);
            game.stack.push_back(child);
            continue;
          }

          std::shared_ptr<Node> child = game.root->children[i];

          if (child->visits < (child->board.placements_remaining > 18 ? kSearchThresholdShort : child->board.placements_until_draw > 1 ? kSearchThresholdMedium
                                                                                                                                       : kSearchThresholdLong)) {
            game.stack.push_back(child);
            continue;
          }

          if (game.root->board.placements_until_draw == 0) {
            game.doDraw(child);
          } else {
            game.doPlace(child);
          }
          continue;
        }  // end if (game.stack.empty())

        std::shared_ptr<Node> leaf = game.stack.back();

        if (leaf->board.is_player_turn) {
          if (leaf->moves.empty()) {
            leaf->board.calculateScore();
            const double ev = static_cast<double>(leaf->board.score[0] + leaf->board.score[8] - kStuckPenaltyInt) / kScoreDenom;
            for (auto& node : game.stack) {
              node->sum += ev;
              ++node->visits;
            }
            game.root->sum += ev;
            ++game.root->visits;
            game.stack.clear();
            continue;
          }

          int i = 0;
          if (leaf->moves.size() != 1) {
            if (leaf->priors.empty()) {
              break;
            }
            i = leaf->selectChild(kCoefUnvisited);
          }

          if (i == leaf->children.size()) {
            std::shared_ptr<Node> child = std::make_shared<Node>(leaf->board);
            child->move = leaf->moves[i];
            if (leaf->board.placements_until_draw == 0) {
              child->board.doDraw(child->move);
            } else {
              child->board.doPlace(child->move % 81, child->move / 81);
            }
            if (child->board.is_player_turn) {
              child->populateDraw();  // child is not placement, because parent is player turn therefore not offer
            }
            leaf->children.push_back(child);
            game.stack.push_back(child);
            continue;
          }

          game.stack.push_back(leaf->children[i]);
          continue;
        }

        if (leaf->board.drawings_completed == 3) {
          leaf->board.calculateScore();
          const double ev = static_cast<double>(leaf->board.score[8]) / kScoreDenom;
          for (auto& node : game.stack) {
            node->sum += ev;
            ++node->visits;
          }
          game.root->sum += ev;
          ++game.root->visits;
          game.stack.clear();
          continue;
        }

        game.sim_deck = PcgDeck{pcg32(game.seed, game.sim_stream + game.stack.front()->visits)};
        game.sim = std::make_shared<Node>(leaf->board);
        game.sim->board.doOffer(game.sim_deck, &game.sim->moves);

        while (game.sim->moves.size() == 1) {
          if (game.sim->board.placements_until_draw == 0) {
            game.sim->board.doDraw(game.sim->moves[0]);
          } else {
            game.sim->board.doPlace(game.sim->moves[0] % 81, game.sim->moves[0] / 81);
          }
          game.sim->moves.clear();
          if (game.sim->board.is_player_turn) {
            game.sim->populateDraw();
          } else {
            if (game.sim->board.drawings_completed == 3) {
              break;
            }
            game.sim->board.doOffer(game.sim_deck, &game.sim->moves);
          }
        }

        if (game.sim->board.drawings_completed != 3 && !game.sim->moves.empty()) {
          break;
        }

        game.sim->board.calculateScore();
        const double ev = static_cast<double>(game.sim->board.score[8] + (game.sim->board.drawings_completed == 3 ? 0 : (game.sim->board.score[0] - kStuckPenaltyInt))) / kScoreDenom;
        for (auto& node : game.stack) {
          node->sum += ev;
          ++node->visits;
        }
        game.root->sum += ev;
        ++game.root->visits;
        game.stack.clear();
        game.sim.reset();
      }

      std::shared_ptr<Node> leaf = game.sim ? game.sim : game.stack.empty() ? game.root
                                                                            : game.stack.back();
      game.ticket = inference_manager->request(leaf->board.input_local, leaf->board.input_global);
    }
  }
}

void SearchManager::run() {
  auto lambda = [this](const std::string& filename) { search(filename); };
  for (int i = 0; i < search_threads; ++i) {
    char c = 'a' + i;
    std::stringstream ss;
    ss << std::time(nullptr) << c << ".txt";
//    if (i == 0) {
//      std::thread(lambda, "YOLO.txt").detach();
//    } else {
      std::thread(lambda, ss.str()).detach();
//    }
  }
}

}  // namespace tinybot
