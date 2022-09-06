#include "npy.h"

namespace tinybot {

int npy(const bool legacy_data) {
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
    std::vector<std::string> tokens = tinybot::split(line);
    if (tokens.size() != 64 && tokens.size() != 65 && !legacy_data) {
      ++problems;
      std::cerr << line << "\n";
      continue;
    }
    tinybot::FixedDeck deck(tokens);
    tinybot::Board board{};
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
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        } else {
          const int x = tinybot::parseX(tokens[i][1]);
          const int y = tinybot::parseY(tokens[i][2]);
          const int t = tinybot::parseTile(tokens[i][0]);
          board.doPlace(y * 9 + x, t);
          insert_output_policy(output_policy_tmp, x, y, t, symmetry_output);
        }
      } else if (i == 18) {
        if (!legacy_data) {
          const int first_x = tinybot::parseX(tokens[i][0]);
          const int first_y = tinybot::parseY(tokens[i][1]);
          const int first_z = first_y * 9 + first_x;
          if (board.input_local[first_z * 21 + 8] < 0.5f) {
            found_problem = true;
            break;
          }
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
          board.doDraw(first_z);
          insert_output_policy(output_policy_tmp, first_x, first_y, 0, symmetry_output);
        }
        std::vector<int> zz;
        for (int j = legacy_data ? 0 : 2; j < tokens[i].size(); j += 2) {
          zz.push_back(tinybot::parseY(tokens[i][j + 1]) * 9 + tinybot::parseX(tokens[i][j]));
        }
        while (!zz.empty()) {
          float best = board.input_local[zz[0] * 21 + 8];
          int best_j = 0;
          for (int j = 1; j < zz.size(); ++j) {
            const float p = board.input_local[zz[j] * 21 + 8];
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
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
          board.doDraw(zz[best_j]);
          insert_output_policy(output_policy_tmp, zz[best_j] % 9, zz[best_j] / 9, 0, symmetry_output);
          zz[best_j] = zz.back();
          zz.pop_back();
        }
        if (found_problem && !legacy_data) {
          break;
        }
        insert_input_local(input_local_tmp, board.input_local, symmetry_input);
        input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        board.doDraw(tinybot::kMovePass);
        insert_input_local(input_local_tmp, board.input_local, symmetry_input);
        input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        insert_output_policy_pass(output_policy_tmp, symmetry_output);
        insert_output_policy_pass(output_policy_tmp, symmetry_output);
      } else if (i < 37) {
        if (i % 2 == 1) {
          board.doOffer(deck, nullptr);
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        } else {
          const int x = tinybot::parseX(tokens[i][1]);
          const int y = tinybot::parseY(tokens[i][2]);
          const int t = tinybot::parseTile(tokens[i][0]);
          board.doPlace(y * 9 + x, t);
          insert_output_policy(output_policy_tmp, x, y, t, symmetry_output);
        }
      } else if (i == 37) {
        if (!legacy_data) {
          const int first_x = tinybot::parseX(tokens[i][0]);
          const int first_y = tinybot::parseY(tokens[i][1]);
          const int first_z = first_y * 9 + first_x;
          if (board.input_local[first_z * 21 + 8] < 0.5f) {
            found_problem = true;
            if (board.land[first_z] != 0 && (first_x == 0 || first_x == 8 || first_y == 0 || first_y == 8)) {
              illegal_lake = true;
            }
            break;
          }
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
          board.doDraw(first_z);
          insert_output_policy(output_policy_tmp, first_x, first_y, 0, symmetry_output);
        }
        std::vector<int> zz;
        for (int j = legacy_data ? 0 : 2; j < tokens[i].size(); j += 2) {
          zz.push_back(tinybot::parseY(tokens[i][j + 1]) * 9 + tinybot::parseX(tokens[i][j]));
        }
        while (!zz.empty()) {
          float best = board.input_local[zz[0] * 21 + 8];
          int best_j = 0;
          for (int j = 1; j < zz.size(); ++j) {
            const float p = board.input_local[zz[j] * 21 + 8];
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
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
          board.doDraw(zz[best_j]);
          insert_output_policy(output_policy_tmp, zz[best_j] % 9, zz[best_j] / 9, 0, symmetry_output);
          zz[best_j] = zz.back();
          zz.pop_back();
        }
        if (found_problem && !legacy_data) {
          break;
        }
        insert_input_local(input_local_tmp, board.input_local, symmetry_input);
        input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        board.doDraw(tinybot::kMovePass);
        insert_input_local(input_local_tmp, board.input_local, symmetry_input);
        input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        insert_output_policy_pass(output_policy_tmp, symmetry_output);
        insert_output_policy_pass(output_policy_tmp, symmetry_output);
      } else if (i < 54) {
        if (i % 2 == 0) {
          board.doOffer(deck, nullptr);
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        } else {
          const int x = tinybot::parseX(tokens[i][1]);
          const int y = tinybot::parseY(tokens[i][2]);
          const int t = tinybot::parseTile(tokens[i][0]);
          board.doPlace(y * 9 + x, t);
          insert_output_policy(output_policy_tmp, x, y, t, symmetry_output);
        }
      } else if (i == 54) {
        if (!legacy_data) {
          const int first_x = tinybot::parseX(tokens[i][0]);
          const int first_y = tinybot::parseY(tokens[i][1]);
          const int first_z = first_y * 9 + first_x;
          if (board.input_local[first_z * 21 + 8] < 0.5f) {
            found_problem = true;
            if (board.land[first_z] != 0 && (first_x == 0 || first_x == 8 || first_y == 0 || first_y == 8)) {
              illegal_lake = true;
            }
            break;
          }
          insert_input_local(input_local_tmp, board.input_local, symmetry_input);
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
          board.doDraw(first_z);
          insert_output_policy(output_policy_tmp, first_x, first_y, 0, symmetry_output);
        }
        std::vector<int> zz;
        for (int j = legacy_data ? 0 : 2; j < tokens[i].size(); j += 2) {
          zz.push_back(tinybot::parseY(tokens[i][j + 1]) * 9 + tinybot::parseX(tokens[i][j]));
        }
        while (!zz.empty()) {
          float best = board.input_local[zz[0] * 21 + 8];
          int best_j = 0;
          for (int j = 1; j < zz.size(); ++j) {
            const float p = board.input_local[zz[j] * 21 + 8];
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
          input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
          board.doDraw(zz[best_j]);
          insert_output_policy(output_policy_tmp, zz[best_j] % 9, zz[best_j] / 9, 0, symmetry_output);
          zz[best_j] = zz.back();
          zz.pop_back();
        }
        if (found_problem && !legacy_data) {
          break;
        }
        insert_input_local(input_local_tmp, board.input_local, symmetry_input);
        input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        board.doDraw(tinybot::kMovePass);
        insert_input_local(input_local_tmp, board.input_local, symmetry_input);
        input_global_tmp.insert(input_global_tmp.end(), board.input_global, board.input_global + tinybot::kTensorLengths[tinybot::kInputGlobal]);
        insert_output_policy_pass(output_policy_tmp, symmetry_output);
        insert_output_policy_pass(output_policy_tmp, symmetry_output);
      } else {
        if (legacy_data) {
          score[8] = atoi(tokens[i].c_str());
          break;
        }
        score[i - 55] = atoi(tokens[i].c_str());
      }
    }
    int rows = input_global_tmp.size() / tinybot::kTensorLengths[tinybot::kInputGlobal];
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
        //        for (int k = 0; k < 81; ++k) {
        //          std::cerr << board.input_local[k * 21 + 8] << ", ";
        //          if (k % 9 == 8)
        //            std::cerr << "\n";
        //        }
        switch (board.draw_mode) {
          case tinybot::kDrawModeUL:
            std::cerr << "kDrawModeUL\n";
            break;
          case tinybot::kDrawModeUR:
            std::cerr << "kDrawModeUR\n";
            break;
          case tinybot::kDrawModeLL:
            std::cerr << "kDrawModeLL\n";
            break;
          case tinybot::kDrawModeLR:
            std::cerr << "kDrawModeLR\n";
            break;
          case tinybot::kDrawModeW:
            std::cerr << "kDrawModeW\n";
            break;
          case tinybot::kDrawModeE:
            std::cerr << "kDrawModeE\n";
            break;
          case tinybot::kDrawModeN:
            std::cerr << "kDrawModeN\n";
            break;
          case tinybot::kDrawModeS:
            std::cerr << "kDrawModeS\n";
            break;
          case tinybot::kDrawModeWN:
            std::cerr << "kDrawModeWN\n";
            break;
          case tinybot::kDrawModeWS:
            std::cerr << "kDrawModeWS\n";
            break;
          case tinybot::kDrawModeEN:
            std::cerr << "kDrawModeEN\n";
            break;
          case tinybot::kDrawModeES:
            std::cerr << "kDrawModeES\n";
            break;
          case tinybot::kDrawModeNW:
            std::cerr << "kDrawModeNW\n";
            break;
          case tinybot::kDrawModeNE:
            std::cerr << "kDrawModeNE\n";
            break;
          case tinybot::kDrawModeSW:
            std::cerr << "kDrawModeSW\n";
            break;
          case tinybot::kDrawModeSE:
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
        const int skip = atoi(tokens[64].c_str());
        input_local.insert(input_local.end(), input_local_tmp.begin() + skip * tinybot::kTensorLengths[tinybot::kInputLocal], input_local_tmp.end());
        input_global.insert(input_global.end(), input_global_tmp.begin() + skip * tinybot::kTensorLengths[tinybot::kInputGlobal], input_global_tmp.end());
        output_policy.insert(output_policy.end(), output_policy_tmp.begin() + skip * tinybot::kTensorLengths[tinybot::kOutputPolicy], output_policy_tmp.end());
        output_land.insert(output_land.end(), output_land_tmp.begin() + skip * 81, output_land_tmp.end());
        float float_score[8];
        for (int i = 0; i < 8; ++i) {
          float_score[i] = static_cast<float>(score[i]) / tinybot::kScoreDenom;
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
          float_score[i] = static_cast<float>(score[i]) / tinybot::kScoreDenom;
        }
        for (int i = 0; i < rows; ++i) {
          output_value.insert(output_value.end(), float_score, float_score + 8);
        }
      }
    }
  }
  std::cerr << problems << " / " << total << "\n";
  std::cerr << symmetry_input << ' ' << symmetry_output << ' ' << symmetry_land << "\n";
  std::cerr << input_local.size() << ' ' << input_global.size() << ' ' << output_policy.size() << ' ' << output_value.size() << ' ' << output_land.size() << "\n";
  cnpy::npy_save("input_local.npy", &input_local[0], {input_local.size() / tinybot::kTensorLengths[tinybot::kInputLocal], 21, 9, 9}, "w");
  cnpy::npy_save("input_global.npy", &input_global[0], {input_global.size() / tinybot::kTensorLengths[tinybot::kInputGlobal], tinybot::kTensorLengths[tinybot::kInputGlobal]}, "w");
  cnpy::npy_save("output_policy.npy", &output_policy[0], {output_policy.size() / tinybot::kTensorLengths[tinybot::kOutputPolicy], tinybot::kTensorLengths[tinybot::kOutputPolicy]}, "w");
  cnpy::npy_save("output_value.npy", &output_value[0], {output_value.size() / tinybot::kTensorLengths[tinybot::kOutputValue], tinybot::kTensorLengths[tinybot::kOutputValue]}, "w");
  cnpy::npy_save("output_land.npy", &output_land[0], {output_land.size() / 81, 1, 9, 9}, "w");
  return problems;
}

void insert_input_local(std::vector<float>& input_local_tmp, float* input_local, int& symmetry) {
  switch (symmetry % 8) {
    case 1:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[((8 - yy) * 9 + 8 - xx) * 21 + c]);
          }
        }
      }
      break;
    case 2:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[(xx * 9 + 8 - yy) * 21 + c]);
          }
        }
      }
      break;
    case 3:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[((8 - xx) * 9 + yy) * 21 + c]);
          }
        }
      }
      break;
    case 4:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[(xx * 9 + yy) * 21 + c]);
          }
        }
      }
      break;
    case 5:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[((8 - xx) * 9 + 8 - yy) * 21 + c]);
          }
        }
      }
      break;
    case 6:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[((8 - yy) * 9 + xx) * 21 + c]);
          }
        }
      }
      break;
    case 7:
      for (int c = 0; c < 21; ++c) {
        for (int yy = 0; yy < 9; ++yy) {
          for (int xx = 0; xx < 9; ++xx) {
            input_local_tmp.push_back(input_local[(yy * 9 + 8 - xx) * 21 + c]);
          }
        }
      }
      break;
    default:
      for (int c = 0; c < 21; ++c) {
        for (int z = 0; z < 81; ++z) {
          input_local_tmp.push_back(input_local[z * 21 + c]);
        }
      }
      break;
  }
  ++symmetry;
}

void insert_output_policy(std::vector<float>& output_policy_tmp, const int x, const int y, const int t, int& symmetry) {
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
  output_policy_tmp.insert(output_policy_tmp.end(), tinybot::kTensorLengths[tinybot::kOutputPolicy], 0.0f);
  output_policy_tmp[offset + t * 81 + yy * 9 + xx] = 1.0f;
  ++symmetry;
}

void insert_output_policy_pass(std::vector<float>& output_policy_tmp, int& symmetry) {
  const auto offset = output_policy_tmp.size();
  output_policy_tmp.insert(output_policy_tmp.end(), tinybot::kTensorLengths[tinybot::kOutputPolicy], 0.0f);
  output_policy_tmp[offset + tinybot::kMovePass * 8] = 1.0f;
  ++symmetry;
}

void insert_output_land(std::vector<float>& output_land_tmp, float* input_local, int& symmetry) {
  switch (symmetry % 8) {
    case 1:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[((8 - yy) * 9 + 8 - xx) * 21]);
        }
      }
      break;
    case 2:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(xx * 9 + 8 - yy) * 21]);
        }
      }
      break;
    case 3:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[((8 - xx) * 9 + yy) * 21]);
        }
      }
      break;
    case 4:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(xx * 9 + yy) * 21]);
        }
      }
      break;
    case 5:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[((8 - xx) * 9 + 8 - yy) * 21]);
        }
      }
      break;
    case 6:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[((8 - yy) * 9 + xx) * 21]);
        }
      }
      break;
    case 7:
      for (int yy = 0; yy < 9; ++yy) {
        for (int xx = 0; xx < 9; ++xx) {
          output_land_tmp.push_back(input_local[(yy * 9 + 8 - xx) * 21]);
        }
      }
      break;
    default:
      for (int z = 0; z < 81; ++z) {
        output_land_tmp.push_back(input_local[z * 21]);
      }
      break;
  }
  ++symmetry;
}

}  // namespace tinybot
