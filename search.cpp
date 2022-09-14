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

void Node::uniformPriors(pcg32& rng) {
  const auto n = moves.size();
  const double p = 1.0 / static_cast<double>(n);
  priors.insert(priors.end(), n, p);
  pcg_extras::shuffle(moves.begin(), moves.end(), rng);
}

void Node::logitsToPriors(pcg32& rng, const bool is_root) {
  if (is_root) {
    const double temperature = board.drawings_completed == 0 ? 1.25 : board.drawings_completed == 1 ? 1.15
                                                                                                    : 1.1;
    for (double& d : priors) {
      d = exp(d / temperature);
    }
    normalizePriors(false);
    noisifyPriors(rng, false);
  } else {
    for (double& d : priors) {
      d = exp(d);
    }
    normalizePriors(true);
  }

  // randomize ties
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

void Node::normalizePriors(const bool do_sort) {
  double total = 0;
  for (double& d : priors) {
    total += d;
  }
  for (double& d : priors) {
    d /= total;
  }
  if (do_sort) {
    sortMoves();
  }
}

void Node::noisifyPriors(pcg32& rng, const bool do_temperature) {
  const auto n = moves.size();
  const double n_double = static_cast<double>(n);
  const double uniform_p = 1.0 / n_double;

  if (do_temperature) {
    const double e_t = 1 / (board.drawings_completed == 0 ? 1.25 : board.drawings_completed == 1 ? 1.15
                                                                                                 : 1.1);
    for (double& d : priors) {
      d = pow(d, e_t);
    }
    normalizePriors(false);
  }

  std::vector<double> alpha;
  // We're going to generate a gamma draw on each move with alphas that sum up to kNoiseTotal.
  // Half of the alpha weight are uniform.
  // The other half are shaped based on the log of the existing policy.
  double mean_log_p = 0.0;
  for (double& p : priors) {
    const double log_p = log(std::min(0.01, p + 1e-20));  // cap priors at 0.01 so we can distinguish 0.01-ish priors from 0.00-ish priors
    alpha.push_back(log_p);
    mean_log_p += log_p;
  }
  mean_log_p /= n_double;
  double alpha_sum = 0.0;
  for (double& a : alpha) {
    if (a > mean_log_p) {
      a -= mean_log_p;
      alpha_sum += a;
    } else {
      a = 0.0;
    }
  }
  if (alpha_sum <= 0.0) {
    const double d = uniform_p * kNoiseTotal;
    for (double& a : alpha) {
      a = d;
    }
  } else {
    const double d = 0.5 * kNoiseTotal;
    for (double& a : alpha) {
      a = d * (a / alpha_sum + uniform_p);
    }
  }
  // alpha_dist now contains the proportions with which we would like to split the alpha
  dirichlet_distribution<pcg32> dist(alpha);
  alpha = dist(rng);
  for (int i = 0; i < n; ++i) {
    priors[i] = priors[i] * 0.75 + alpha[i] * 0.25;
  }
  sortMoves();
}

void Node::sortMoves() {
  const int begin_index = children.size();
  const int end_index = priors.size();
  if (begin_index == end_index) {
    return;
  }
  const int count = end_index - begin_index;
  std::vector<int> perm;
  for (int i = begin_index; i < end_index; ++i) {
    perm.push_back(i);
  }
  auto compare = std::greater<>{};
  std::stable_sort(perm.begin(), perm.end(), [&](int i, int j) { return compare(priors[i], priors[j]); });

  double p_buffer[count];
  int m_buffer[count];
  for (int i = 0; i < count; ++i) {
    p_buffer[i] = priors[perm[i]];
    m_buffer[i] = moves[perm[i]];
  }
  for (int i = 0; i < count; ++i) {
    priors[begin_index + i] = p_buffer[i];
    moves[begin_index + i] = m_buffer[i];
  }
}

int Node::selectChild(const bool apply_coef_unvisited, double* coefs_explore) {
  //  if (moves.size() == 1) {
  //    return 0;
  //  }
  const double base_coef_explore = getCoefExplore(coefs_explore);
  const double sqrt_rv = sqrt(static_cast<double>(visits));
  const double coef_explore = base_coef_explore * sqrt_rv;
  const double coef_unvisited = apply_coef_unvisited ? getCoefUnvisited(base_coef_explore) : 0;
  const double coef_forced_playout = kCoefForcedPlayout * sqrt_rv;
  double p_explored = 0;
  int best_i = 0;
  double best_ucb = kNegativeInfinity;
  for (int i = 0; i < priors.size(); ++i) {
    if (i == children.size() || children[i]->visits == 0) {
      return (visits == 0 || sum / visits + coef_explore * priors[i] + coef_unvisited * sqrt(p_explored) > best_ucb) ? i : best_i;
    }
    if (int(coef_forced_playout * sqrt(priors[i])) > children[i]->visits) {
      return i;
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

int Node::getSearchThreshold(const int* search_thresholds) const {
  return search_thresholds[board.phase()];
}

double Node::getCoefExplore(const double* coefs_explore) const {
  return coefs_explore[board.phase()] / kScoreDenom;
}

double Node::getCoefUnvisited(const double* coefs_explore) const {
  return getCoefUnvisited(getCoefExplore(coefs_explore));
}

double Node::getCoefUnvisited(const double coef_explore) const {
  return -0.2 * coef_explore;
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

void Game::doPlace(pcg32& rng, std::shared_ptr<Node> child, double* coefs_explore) {
  reportVisits(child->move, coefs_explore);
  root = std::move(child);
  const int z = root->move % 81;
  const int t = root->move / 81;
  record.push_back(kTileChars[t]);
  record.push_back(kZoneChars[z % 9 + 9]);
  record.push_back(kZoneChars[z / 9]);
  record.push_back(' ');
  if (root->board.is_player_turn && root->moves.size() > 1) {
    root->noisifyPriors(rng, true);
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

void Game::doDraw(pcg32& rng, std::shared_ptr<Node> child, double* coefs_explore) {
  reportVisits(child->move, coefs_explore);
  root = std::move(child);
  if (root->move == kMovePass) {
    record.push_back(' ');
  } else {
    record.push_back(kZoneChars[root->move % 9 + 9]);
    record.push_back(kZoneChars[root->move / 9]);
    if (root->moves.size() > 1) {
      root->noisifyPriors(rng, true);
    }
  }
  sim_stream += root->visits;
}

void Game::reportVisits(const int selected_move, double* coefs_explore) {
  const int n = root->children.size();
  if (n <= 1) {
    return;
  }
  const double base_coef_explore = root->getCoefExplore(coefs_explore);
  const double sqrt_rv = sqrt(static_cast<double>(root->visits));
  const double coef_explore = base_coef_explore * sqrt_rv;
  const double coef_forced_playout = kCoefForcedPlayout * sqrt_rv;

  int selected_index;
  double selected_cpuct;
  for (int i = 0; i < n; ++i) {
    const auto& child = root->children[i];
    if (child->move == selected_move) {
      selected_index = i;
      selected_cpuct = child->sum / child->visits + coef_explore * root->priors[i] / (1.0 + child->visits);
      break;
    }
  }

  for (int i = 0; i < n; ++i) {
    if (i == selected_index) {
      continue;
    }
    const auto& child = root->children[i];
    const double ev = child->sum / child->visits;
    const double cep = coef_explore * root->priors[i];
    const double vpp = 1.0 + child->visits;
    if (ev + cep / vpp >= selected_cpuct) {
      continue;
    }
    const double prune_cap = vpp - cep / (selected_cpuct - ev);
    const double forced_playouts = coef_forced_playout * sqrt(root->priors[i]);
    child->visits -= int(std::min(prune_cap, forced_playouts));
  }

  record.push_back('{');
  for (int i = 0; i < n; ++i) {
    const int v = root->children[i]->visits;
    if (v > 1) {
      record += intToString(root->moves[i]);
      record.push_back(':');
      record += intToString(v);
      record.push_back(',');
    }
  }
  record.push_back('}');
}

void Game::start(pcg32& rng, IDeck& deck) {
  reset();
  random_moves = rng(8);
  for (int i = 0; i < random_moves; ++i) {
    doOffer(rng, deck);
    doPlace(root->moves[rng(root->moves.size())]);
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

std::string SearchManager::threadInfo(const std::string& filename, const int thread_id, const int* search_thresholds, const double* coefs_explore) const {
  std::stringstream ss;
  ss << thread_id << ' ' << filename;
  for (int i = 0; i < 12; ++i) {
    if (i % 4 <= 1) {
      ss << ' ' << search_thresholds[i];
    }
  }
  for (int i = 0; i < 12; ++i) {
    if (i % 4 <= 1) {
      ss << ' ' << coefs_explore[i];
    }
  }
  ss << "\n";
  return ss.str();
}

[[noreturn]] void SearchManager::search(const std::string& filename, const int thread_id) const {
  unsigned long original_seed = stringToSeed(filename);
  std::ofstream out_file;
  out_file.open(filename);

  int search_thresholds[12];
  double coefs_explore[12];
  for (int i = 0; i < 12; ++i) {
    search_thresholds[i] = kSearchThresholds[i];
    coefs_explore[i] = kCoefsExplore[i];
  }

  searchExperiment(thread_id, search_thresholds, coefs_explore);
  std::cout << threadInfo(filename, thread_id, search_thresholds, coefs_explore);

  std::exponential_distribution<> expd{1};
  pcg32 rng(original_seed, 0);
  PcgDeck deck(pcg32(original_seed, 1));  // TODO test rng ?= pcg32(rng())
  Game games[kConcurrentGames / search_threads];
  for (Game& game : games) {
    ++original_seed;
    game.seed = original_seed;
    game.start(rng, deck);
    game.ticket = inference_manager->request(game.root->board.input_local, game.root->board.input_global);
  }

  //  int debug_counter = 0;
  //  int debug_score = 0;
  while (true) {
    //    bool first_flag = true;
    for (Game& game : games) {
      //      const bool is_first = first_flag;
      //      first_flag = false;

      const int offset = game.ticket.cursor * kTensorLengths[kOutputPolicy];
      float* output_policy = game.ticket.future.get()->output_policy;

      if (game.sim) {
        for (const int move : game.sim->moves) {
          game.sim->priors.push_back(output_policy[offset + move]);
        }
        game.sim->logitsToPriors(rng, false);

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
                    //                    game.sim->uniformPriors(rng);
                  } else {
                    game.sim = game.sim->children[0];
                  }
                  continue;
                }
                game.sim->uniformPriors(rng);
              }

              const int i = game.sim->selectChild(false, coefs_explore);  // TODO should I use coef_unvisited?

              if (i == game.sim->children.size()) {
                std::shared_ptr<Node> child = std::make_shared<Node>(game.sim->board);
                child->move = game.sim->moves[i];
                child->board.doDraw(child->move);
                if (child->board.is_player_turn) {
                  child->populateDraw();
                  child->uniformPriors(rng);
                }
                game.sim->children.push_back(child);
                stack.push_back(child);
                continue;
              }

              std::shared_ptr<Node> child = game.sim->children[i];

              if (child->visits < game.sim->getSearchThreshold(search_thresholds)) {
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

              const int i = leaf->moves.size() == 1 ? 0 : leaf->selectChild(true, coefs_explore);

              if (i == leaf->children.size()) {
                std::shared_ptr<Node> child = std::make_shared<Node>(leaf->board);
                child->move = leaf->moves[i];
                child->board.doDraw(child->move);
                if (child->board.is_player_turn) {
                  child->populateDraw();
                  child->uniformPriors(rng);
                }
                leaf->children.push_back(child);
                stack.push_back(child);
                continue;
              }

              stack.push_back(leaf->children[i]);
              continue;
            }

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
          //          if (game.root->board.placements_remaining == 26) {
          //            ++debug_counter;
          //            debug_score += game.sim->board.score[8];
          ////            if (rng(1024) == 0) {
          //              std::cout << game.sim_stream << " + " << game.stack.front()->visits << ":\n";
          //              game.sim->board.display();
          //              std::cout << debug_counter << ": " << static_cast<float>(debug_score) / static_cast<float>(debug_counter) << "\n";
          ////            }
          //          }
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
          const bool drawing = game.sim->board.placements_until_draw == 0;
          const int n = game.sim->priors.size();
          int random_i = 0;
          //          if (drawing) {
          //            double r = static_cast<double>(game.sim_rng_draw()) / kPcg32MaxDouble;
          //            for (int i = 0; i < n; ++i) {
          //              if (r < game.sim->priors[i]) {
          //                random_i = i;
          //                break;
          //              }
          //              r -= game.sim->priors[i];
          //            }
          //          } else {
          //            //                        double r = static_cast<double>(game.sim_rng_place()) / kPcg32MaxDouble;
          //            //                        for (int i = 0; i < n; ++i) {
          //            //                          if (r < game.sim->priors[i]) {
          //            //                            random_i = i;
          //            //                            break;
          //            //                          }
          //            //                          r -= game.sim->priors[i];
          //            //                        }
          //            double best = kPositiveInfinity;
          //            for (int i = 0; i < n; ++i) {
          //              pcg32 move_rng{game.seed, (game.sim_stream + game.stack.front()->visits) * 1024 + game.sim->moves[i]};
          //              const double proj = expd(move_rng) / game.sim->priors[i];
          //              if (proj < best) {
          //                best = proj;
          //                random_i = i;
          //              }
          //            }
          //          }
          game.sim->priors.clear();
          if (drawing) {
            game.sim->board.doDraw(game.sim->moves[random_i]);
            if (game.sim->moves[random_i] == kMovePass) {
              game.sim_rng_draw = pcg32(game.seed, (game.sim_stream + game.stack.front()->visits) * 1024 + 651 + game.sim->board.drawings_completed);
            }
          } else {
            game.sim->board.doPlace(game.sim->moves[random_i] % 81, game.sim->moves[random_i] / 81);
          }

          do {
            if (game.sim->board.drawings_completed == 3) {  // vestigial branch
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
                if (game.sim->moves[0] == kMovePass) {
                  game.sim_rng_draw = pcg32(game.seed, (game.sim_stream + game.stack.front()->visits) * 1024 + 651 + game.sim->board.drawings_completed);
                }
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
      } else if (game.stack.empty()) {
        for (const int move : game.root->moves) {
          game.root->priors.push_back(output_policy[offset + move]);
        }
        game.root->logitsToPriors(rng, true);
      } else {
        std::shared_ptr<Node> leaf = game.stack.back();
        for (const int move : leaf->moves) {
          leaf->priors.push_back(output_policy[offset + move]);
        }
        leaf->logitsToPriors(rng, false);
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
                game.doDraw(rng, game.root->children[0], coefs_explore);
              } else {
                game.doPlace(rng, game.root->children[0], coefs_explore);
              }
            }
            continue;
          }

          const int i = game.root->selectChild(false, coefs_explore);

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

          if (child->visits < game.root->getSearchThreshold(search_thresholds)) {
            game.stack.push_back(child);
            continue;
          }

          if (game.root->board.placements_until_draw == 0) {
            game.doDraw(rng, child, coefs_explore);
          } else {
            game.doPlace(rng, child, coefs_explore);
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
            i = leaf->selectChild(true, coefs_explore);
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

        const auto sim_stream_offset = (game.sim_stream + game.stack.front()->visits) * 1024;
        game.sim_deck = PcgDeck{pcg32(game.seed, sim_stream_offset + 649)};
        game.sim_rng_place = pcg32(game.seed, sim_stream_offset + 650);
        game.sim_rng_draw = pcg32(game.seed, sim_stream_offset + 651);
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
  auto lambda = [this](const std::string& filename, const int i) { search(filename, i); };
  for (int i = 0; i < search_threads; ++i) {
    char c = 'a' + i;
    std::stringstream ss;
    ss << std::time(nullptr) << c << ".txt";
    //    if (i == 0) {
    //      std::thread(lambda, "YOLO.txt").detach();
    //    } else {
    std::thread(lambda, ss.str(), i).detach();
    //    }
  }
}

}  // namespace tinybot
