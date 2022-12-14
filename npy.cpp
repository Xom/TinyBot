#include "npy.h"

namespace tinybot {

int npy(const bool legacy_data) {
  std::vector<float> input_local;
  std::vector<float> input_global;
  std::vector<float> output_policy;
  std::vector<float> output_value;
  std::vector<float> output_land;
  int problems = 0;
  int total = 0;
  int symmetry_input = 0;
  int symmetry_output = 0;
  int symmetry_land = 0;
  for (std::string line; std::getline(std::cin, line);) {
    if (total % 1000 == 0) {
      std::cerr << problems << " / " << total << "\n";
    }
    ++total;
    std::vector<std::string> tokens = split(line);
    if (tokens.size() != 64 && tokens.size() != 65 && !legacy_data) {
      ++problems;
      std::cerr << line << "\n";
      continue;
    }
    FixedDeck deck(tokens);
    Board board{};
    bool found_problem = false;
    bool illegal_lake = false;
    int score[9]{};
    std::vector<float> input_local_tmp;
    std::vector<float> input_global_tmp;
    std::vector<float> output_policy_tmp;
    std::vector<float> output_land_tmp;
    for (int i = 0; i < 64; ++i) {
      if (i < 18) {
        if (i % 2 == 0) {
          board.doOffer(deck, nullptr);
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          insert_input_global(input_global_tmp, board.input_global);
        } else {
          int cursor = 0;
          const bool has_distribution = tokens[i][cursor] == '{';
          if (has_distribution) {
            insert_output_policy(output_policy_tmp, tokens[i], cursor, symmetry_output);
          }
          const int x = parseX(tokens[i][cursor + 1]);
          const int y = parseY(tokens[i][cursor + 2]);
          const int t = parseTile(tokens[i][cursor]);
          board.doPlace(y * 9 + x, t);
          if (!has_distribution) {
            insert_output_policy(output_policy_tmp, x, y, t, symmetry_output);
          }
        }
      } else if (i == 18) {
        if (legacy_data) {
          std::vector<int> zz;
          for (int j = 0; j < tokens[i].size(); j += 2) {
            zz.push_back(parseY(tokens[i][j + 1]) * 9 + parseX(tokens[i][j]));
          }
          while (!zz.empty()) {
            float best = board.input_local[zz[0] + 648];
            int best_j = 0;
            for (int j = 1; j < zz.size(); ++j) {
              const float p = board.input_local[zz[j] + 648];
              if (p > best) {
                best = p;
                best_j = j;
              }
            }
            if (best < 0.5f) {
              found_problem = true;
              break;
            }
            insert_input_local(input_local_tmp, board.input_local, symmetry_input);
            insert_input_global(input_global_tmp, board.input_global);
            board.doDraw(zz[best_j]);
            insert_output_policy(output_policy_tmp, zz[best_j] % 9, zz[best_j] / 9, 0, symmetry_output);
            zz[best_j] = zz.back();
            zz.pop_back();
          }
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          insert_input_global(input_global_tmp, board.input_global);
          board.doDraw(kMovePass);
          insert_output_policy_pass(output_policy_tmp, symmetry_output);
        } else {
          int cursor = 0;
          while (true) {
            const bool has_distribution = tokens[i][cursor] == '{';
            if (has_distribution) {
              insert_output_policy(output_policy_tmp, tokens[i], cursor, symmetry_output);
            }
            if (cursor == tokens[i].size()) {
              insert_input_local(input_local_tmp, board.input_local, symmetry_input);
              insert_input_global(input_global_tmp, board.input_global);
              board.doDraw(kMovePass);
              if (!has_distribution) {
                insert_output_policy_pass(output_policy_tmp, symmetry_output);
              }
              break;
            }
            const int x = parseX(tokens[i][cursor]);
            const int y = parseY(tokens[i][cursor + 1]);
            const int z = y * 9 + x;
            if (board.input_local[z + 648] < 0.5f) {
              found_problem = true;
              break;
            }
            insert_input_local(input_local_tmp, board.input_local, symmetry_input);
            insert_input_global(input_global_tmp, board.input_global);
            board.doDraw(z);
            if (!has_distribution) {
              insert_output_policy(output_policy_tmp, x, y, 0, symmetry_output);
            }
            cursor += 2;
          }
        }
        if (total == 1) {
          board.display();
        }
      } else if (i < 37) {
        if (i % 2 == 1) {
          board.doOffer(deck, nullptr);
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          insert_input_global(input_global_tmp, board.input_global);
        } else {
          int cursor = 0;
          const bool has_distribution = tokens[i][cursor] == '{';
          if (has_distribution) {
            insert_output_policy(output_policy_tmp, tokens[i], cursor, symmetry_output);
          }
          const int x = parseX(tokens[i][cursor + 1]);
          const int y = parseY(tokens[i][cursor + 2]);
          const int t = parseTile(tokens[i][cursor]);
          board.doPlace(y * 9 + x, t);
          if (!has_distribution) {
            insert_output_policy(output_policy_tmp, x, y, t, symmetry_output);
          }
        }
      } else if (i == 37) {
        if (legacy_data) {
          std::vector<int> zz;
          for (int j = 0; j < tokens[i].size(); j += 2) {
            zz.push_back(parseY(tokens[i][j + 1]) * 9 + parseX(tokens[i][j]));
          }
          while (!zz.empty()) {
            float best = board.input_local[zz[0] + 648];
            int best_j = 0;
            for (int j = 1; j < zz.size(); ++j) {
              const float p = board.input_local[zz[j] + 648];
              if (p > best) {
                best = p;
                best_j = j;
              }
            }
            if (best < 0.5f) {
              found_problem = true;
              break;
            }
            insert_input_local(input_local_tmp, board.input_local, symmetry_input);
            insert_input_global(input_global_tmp, board.input_global);
            board.doDraw(zz[best_j]);
            insert_output_policy(output_policy_tmp, zz[best_j] % 9, zz[best_j] / 9, 0, symmetry_output);
            zz[best_j] = zz.back();
            zz.pop_back();
          }
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          insert_input_global(input_global_tmp, board.input_global);
          board.doDraw(kMovePass);
          insert_output_policy_pass(output_policy_tmp, symmetry_output);
        } else {
          int cursor = 0;
          while (true) {
            const bool has_distribution = tokens[i][cursor] == '{';
            if (has_distribution) {
              insert_output_policy(output_policy_tmp, tokens[i], cursor, symmetry_output);
            }
            if (cursor == tokens[i].size()) {
              insert_input_local(input_local_tmp, board.input_local, symmetry_input);
              insert_input_global(input_global_tmp, board.input_global);
              board.doDraw(kMovePass);
              if (!has_distribution) {
                insert_output_policy_pass(output_policy_tmp, symmetry_output);
              }
              break;
            }
            const int x = parseX(tokens[i][cursor]);
            const int y = parseY(tokens[i][cursor + 1]);
            const int z = y * 9 + x;
            if (board.input_local[z + 648] < 0.5f) {
              found_problem = true;
              break;
            }
            insert_input_local(input_local_tmp, board.input_local, symmetry_input);
            insert_input_global(input_global_tmp, board.input_global);
            board.doDraw(z);
            if (!has_distribution) {
              insert_output_policy(output_policy_tmp, x, y, 0, symmetry_output);
            }
            cursor += 2;
          }
        }
        if (total == 1) {
          board.display();
        }
      } else if (i < 54) {
        if (i % 2 == 0) {
          board.doOffer(deck, nullptr);
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          insert_input_global(input_global_tmp, board.input_global);
        } else {
          int cursor = 0;
          const bool has_distribution = tokens[i][cursor] == '{';
          if (has_distribution) {
            insert_output_policy(output_policy_tmp, tokens[i], cursor, symmetry_output);
          }
          const int x = parseX(tokens[i][cursor + 1]);
          const int y = parseY(tokens[i][cursor + 2]);
          const int t = parseTile(tokens[i][cursor]);
          board.doPlace(y * 9 + x, t);
          if (!has_distribution) {
            insert_output_policy(output_policy_tmp, x, y, t, symmetry_output);
          }
        }
      } else if (i == 54) {
        if (legacy_data) {
          std::vector<int> zz;
          for (int j = 0; j < tokens[i].size(); j += 2) {
            zz.push_back(parseY(tokens[i][j + 1]) * 9 + parseX(tokens[i][j]));
          }
          while (!zz.empty()) {
            float best = board.input_local[zz[0] + 648];
            int best_j = 0;
            for (int j = 1; j < zz.size(); ++j) {
              const float p = board.input_local[zz[j] + 648];
              if (p > best) {
                best = p;
                best_j = j;
              }
            }
            if (best < 0.5f) {
              found_problem = true;
              break;
            }
            insert_input_local(input_local_tmp, board.input_local, symmetry_input);
            insert_input_global(input_global_tmp, board.input_global);
            board.doDraw(zz[best_j]);
            insert_output_policy(output_policy_tmp, zz[best_j] % 9, zz[best_j] / 9, 0, symmetry_output);
            zz[best_j] = zz.back();
            zz.pop_back();
          }
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          insert_input_global(input_global_tmp, board.input_global);
          board.doDraw(kMovePass);
          insert_output_policy_pass(output_policy_tmp, symmetry_output);
        } else {
          int cursor = 0;
          while (true) {
            const bool has_distribution = tokens[i][cursor] == '{';
            if (has_distribution) {
              insert_output_policy(output_policy_tmp, tokens[i], cursor, symmetry_output);
            }
            if (cursor == tokens[i].size()) {
              insert_input_local(input_local_tmp, board.input_local, symmetry_input);
              insert_input_global(input_global_tmp, board.input_global);
              board.doDraw(kMovePass);
              if (!has_distribution) {
                insert_output_policy_pass(output_policy_tmp, symmetry_output);
              }
              break;
            }
            const int x = parseX(tokens[i][cursor]);
            const int y = parseY(tokens[i][cursor + 1]);
            const int z = y * 9 + x;
            if (board.input_local[z + 648] < 0.5f) {
              found_problem = true;
              break;
            }
            insert_input_local(input_local_tmp, board.input_local, symmetry_input);
            insert_input_global(input_global_tmp, board.input_global);
            board.doDraw(z);
            if (!has_distribution) {
              insert_output_policy(output_policy_tmp, x, y, 0, symmetry_output);
            }
            cursor += 2;
          }
        }
      } else {
        if (legacy_data) {
          score[8] = atoi(tokens[i].c_str());
          break;
        }
        score[i - 55] = atoi(tokens[i].c_str());
      }
    }
    int rows = input_global_tmp.size() / kTensorLengths[kInputGlobal];
    for (int i = 0; i < rows; ++i) {
      insert_output_land(output_land_tmp, board.input_local, symmetry_land);
    }
    if (found_problem) {
      ++problems;
      if (illegal_lake) {
        std::cerr << line << " (illegal lake)\n";
      } else {
        std::cerr << line << "\n";
        board.display();
        switch (board.draw_mode) {
          case kDrawModeUL:
            std::cerr << "kDrawModeUL\n";
            break;
          case kDrawModeUR:
            std::cerr << "kDrawModeUR\n";
            break;
          case kDrawModeLL:
            std::cerr << "kDrawModeLL\n";
            break;
          case kDrawModeLR:
            std::cerr << "kDrawModeLR\n";
            break;
          case kDrawModeW:
            std::cerr << "kDrawModeW\n";
            break;
          case kDrawModeE:
            std::cerr << "kDrawModeE\n";
            break;
          case kDrawModeN:
            std::cerr << "kDrawModeN\n";
            break;
          case kDrawModeS:
            std::cerr << "kDrawModeS\n";
            break;
          case kDrawModeWN:
            std::cerr << "kDrawModeWN\n";
            break;
          case kDrawModeWS:
            std::cerr << "kDrawModeWS\n";
            break;
          case kDrawModeEN:
            std::cerr << "kDrawModeEN\n";
            break;
          case kDrawModeES:
            std::cerr << "kDrawModeES\n";
            break;
          case kDrawModeNW:
            std::cerr << "kDrawModeNW\n";
            break;
          case kDrawModeNE:
            std::cerr << "kDrawModeNE\n";
            break;
          case kDrawModeSW:
            std::cerr << "kDrawModeSW\n";
            break;
          case kDrawModeSE:
            std::cerr << "kDrawModeSE\n";
            break;
          default:
            std::cerr << board.draw_mode << "\n";
            break;
        }
        std::cerr << problems << " / " << total << "\n";
      }
    } else {
      board.calculateScore();
      for (int i = legacy_data ? 8 : 0; i < 9; ++i) {
        if (score[i] != board.score[i]) {
          ++problems;
          std::cerr << line << "\n";
          board.display();
          std::cerr << problems << " / " << total << "\n";
          break;
        }
      }
      if (tokens.size() == 65) {
        const int skip = atoi(tokens[64].c_str()) * kRowsPerTurn;
        input_local.insert(input_local.end(), input_local_tmp.begin() + skip * kTensorLengths[kInputLocal], input_local_tmp.end());
        input_global.insert(input_global.end(), input_global_tmp.begin() + skip * kTensorLengths[kInputGlobal], input_global_tmp.end());
        output_policy.insert(output_policy.end(), output_policy_tmp.begin() + skip * kTensorLengths[kOutputPolicy], output_policy_tmp.end());
        output_land.insert(output_land.end(), output_land_tmp.begin() + skip * kTensorLengths[kOutputLand], output_land_tmp.end());
        float float_score[8];
        for (int i = 0; i < 8; ++i) {
          float_score[i] = static_cast<float>(score[i]) / kScoreDenom;
        }
        rows -= skip;
        for (int i = 0; i < rows; ++i) {
          output_value.insert(output_value.end(), float_score, float_score + 8);
        }
      } else {
        input_local.insert(input_local.end(), input_local_tmp.begin(), input_local_tmp.end());
        input_global.insert(input_global.end(), input_global_tmp.begin(), input_global_tmp.end());
        output_policy.insert(output_policy.end(), output_policy_tmp.begin(), output_policy_tmp.end());
        output_land.insert(output_land.end(), output_land_tmp.begin(), output_land_tmp.end());
        float float_score[8];
        for (int i = 0; i < 8; ++i) {
          float_score[i] = static_cast<float>(score[i]) / kScoreDenom;
        }
        for (int i = 0; i < rows; ++i) {
          output_value.insert(output_value.end(), float_score, float_score + 8);
        }
      }
    }
    if (total == 1) {
      board.display();
      {
        std::ifstream ifile;
        ifile.open("input_local.npy");
        if (ifile) {
          std::cerr << "Error: input_local.npy already exists\n";
          return 1;
        }
      }
      {
        std::ifstream ifile;
        ifile.open("input_global.npy");
        if (ifile) {
          std::cerr << "Error: input_global.npy already exists\n";
          return 1;
        }
      }
      {
        std::ifstream ifile;
        ifile.open("output_policy.npy");
        if (ifile) {
          std::cerr << "Error: output_policy.npy already exists\n";
          return 1;
        }
      }
      {
        std::ifstream ifile;
        ifile.open("output_land.npy");
        if (ifile) {
          std::cerr << "Error: output_land.npy already exists\n";
          return 1;
        }
      }
      {
        std::ifstream ifile;
        ifile.open("output_value.npy");
        if (ifile) {
          std::cerr << "Error: output_value.npy already exists\n";
          return 1;
        }
      }
    }
  }
  std::cerr << problems << " / " << total << "\n";
  std::cerr << symmetry_input << ' ' << symmetry_output << ' ' << symmetry_land << "\n";
  std::cerr << input_local.size() << ' ' << input_global.size() << ' ' << output_policy.size() << ' ' << output_value.size() << ' ' << output_land.size() << "\n";
  cnpy::npy_save("input_local.npy", &input_local[0], {input_local.size() / kTensorLengths[kInputLocal], 21, 9, 9}, "w");
  cnpy::npy_save("input_global.npy", &input_global[0], {input_global.size() / kTensorLengths[kInputGlobal], kTensorLengths[kInputGlobal]}, "w");
  cnpy::npy_save("output_policy.npy", &output_policy[0], {output_policy.size() / kTensorLengths[kOutputPolicy], kTensorLengths[kOutputPolicy]}, "w");
  cnpy::npy_save("output_value.npy", &output_value[0], {output_value.size() / kTensorLengths[kOutputValue], kTensorLengths[kOutputValue]}, "w");
  cnpy::npy_save("output_land.npy", &output_land[0], {output_land.size() / kTensorLengths[kOutputLand], 1, 9, 9}, "w");
  return problems;
}

void insert_input_local(std::vector<float>& input_local_tmp, float* input_local, int& symmetry) {
  for (int r = 0; r < kRowsPerTurn; ++r) {
    switch (symmetry % 8) {
      case 1:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + (8 - yy) * 9 + 8 - xx]);
            }
          }
        }
        break;
      case 2:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + xx * 9 + 8 - yy]);
            }
          }
        }
        break;
      case 3:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + (8 - xx) * 9 + yy]);
            }
          }
        }
        break;
      case 4:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + xx * 9 + yy]);
            }
          }
        }
        break;
      case 5:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + (8 - xx) * 9 + 8 - yy]);
            }
          }
        }
        break;
      case 6:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + (8 - yy) * 9 + xx]);
            }
          }
        }
        break;
      case 7:
        for (int c = 0; c < 21; ++c) {
          for (int yy = 0; yy < 9; ++yy) {
            for (int xx = 0; xx < 9; ++xx) {
              input_local_tmp.push_back(input_local[c * 81 + yy * 9 + 8 - xx]);
            }
          }
        }
        break;
      default:
        input_local_tmp.insert(input_local_tmp.end(), input_local, input_local + kTensorLengths[kInputLocal]);
        break;
    }
    ++symmetry;
  }
}

void insert_input_global(std::vector<float>& input_global_tmp, float* input_global) {
  for (int r = 0; r < kRowsPerTurn; ++r) {
    input_global_tmp.insert(input_global_tmp.end(), input_global, input_global + kTensorLengths[kInputGlobal]);
  }
}

void insert_output_policy(std::vector<float>& output_policy_tmp, std::string& str, int& cursor, int& symmetry) {
  std::vector<int> moves;
  std::vector<int> priors;
  int sum_p = 0;
  int n = 0;
  bool more = true;
  do {
    ++cursor;
    switch (str[cursor]) {
      case ':':
        moves.push_back(n);
        n = 0;
        break;
      case ',':
        sum_p += n;
        priors.push_back(n);
        n = 0;
        break;
      case '0':
        n *= 10;
        break;
      case '1':
        n = n * 10 + 1;
        break;
      case '2':
        n = n * 10 + 2;
        break;
      case '3':
        n = n * 10 + 3;
        break;
      case '4':
        n = n * 10 + 4;
        break;
      case '5':
        n = n * 10 + 5;
        break;
      case '6':
        n = n * 10 + 6;
        break;
      case '7':
        n = n * 10 + 7;
        break;
      case '8':
        n = n * 10 + 8;
        break;
      case '9':
        n = n * 10 + 9;
        break;
      default:
        more = false;
        break;
    }
  } while (more);
  ++cursor;

  n = moves.size();
  const float multiplicand = 1.0f / static_cast<float>(sum_p);
  for (int r = 0; r < kRowsPerTurn; ++r) {
    const auto offset = output_policy_tmp.size();
    output_policy_tmp.insert(output_policy_tmp.end(), kTensorLengths[kOutputPolicy], 0.0f);
    for (int i = 0; i < n; ++i) {
      int m = moves[i];
      if (m != kMovePass) {
        const int x = m % 9;
        const int y = (m % 81) / 9;
        const int t = m / 81;
        int xx, yy;
        switch (symmetry % 8) {
          case 1:
            xx = 8 - x;
            yy = 8 - y;
            break;
          case 2:
            xx = y;
            yy = 8 - x;
            break;
          case 3:
            xx = 8 - y;
            yy = x;
            break;
          case 4:
            xx = y;
            yy = x;
            break;
          case 5:
            xx = 8 - y;
            yy = 8 - x;
            break;
          case 6:
            xx = x;
            yy = 8 - y;
            break;
          case 7:
            xx = 8 - x;
            yy = y;
            break;
          default:
            xx = x;
            yy = y;
            break;
        }
        m = t * 81 + yy * 9 + xx;
      }
      output_policy_tmp[offset + m] = static_cast<float>(priors[i]) * multiplicand;
    }
    ++symmetry;
  }
}

void insert_output_policy(std::vector<float>& output_policy_tmp, const int x, const int y, const int t, int& symmetry) {
  for (int r = 0; r < kRowsPerTurn; ++r) {
    int xx, yy;
    switch (symmetry % 8) {
      case 1:
        xx = 8 - x;
        yy = 8 - y;
        break;
      case 2:
        xx = y;
        yy = 8 - x;
        break;
      case 3:
        xx = 8 - y;
        yy = x;
        break;
      case 4:
        xx = y;
        yy = x;
        break;
      case 5:
        xx = 8 - y;
        yy = 8 - x;
        break;
      case 6:
        xx = x;
        yy = 8 - y;
        break;
      case 7:
        xx = 8 - x;
        yy = y;
        break;
      default:
        xx = x;
        yy = y;
        break;
    }
    const auto offset = output_policy_tmp.size();
    output_policy_tmp.insert(output_policy_tmp.end(), kTensorLengths[kOutputPolicy], 0.0f);
    output_policy_tmp[offset + t * 81 + yy * 9 + xx] = 1.0f;
    ++symmetry;
  }
}

void insert_output_policy_pass(std::vector<float>& output_policy_tmp, int& symmetry) {
  for (int r = 0; r < kRowsPerTurn; ++r) {
    const auto offset = output_policy_tmp.size();
    output_policy_tmp.insert(output_policy_tmp.end(), kTensorLengths[kOutputPolicy], 0.0f);
    output_policy_tmp[offset + kMovePass] = 1.0f;
    ++symmetry;
  }
}

void insert_output_land(std::vector<float>& output_land_tmp, float* input_local, int& symmetry) {
  switch (symmetry % 8) {
    case 1:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(8 - yy) * 9 + 8 - xx]);
        }
      }
      break;
    case 2:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[xx * 9 + 8 - yy]);
        }
      }
      break;
    case 3:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(8 - xx) * 9 + yy]);
        }
      }
      break;
    case 4:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[xx * 9 + yy]);
        }
      }
      break;
    case 5:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(8 - xx) * 9 + 8 - yy]);
        }
      }
      break;
    case 6:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(8 - yy) * 9 + xx]);
        }
      }
      break;
    case 7:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[yy * 9 + 8 - xx]);
        }
      }
      break;
    default:
      output_land_tmp.insert(output_land_tmp.end(), input_local, input_local + kTensorLengths[kOutputLand]);
      break;
  }
  ++symmetry;
}

}  // namespace tinybot
