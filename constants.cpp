#include "constants.h"

namespace tinybot {

const char* kTensorNames[kNumTensors]{"input_local", "input_global", "output_policy", "output_value", "output_land"};

void searchExperiment(const int thread_id, int* search_thresholds, double* coefs_explore) {
  switch (thread_id) {
    case 1:
      search_thresholds[kPhase1a] = 10;
      break;
    case 2:
      search_thresholds[kPhase0b] = 10;
      search_thresholds[kPhase0c] = 10;
      search_thresholds[kPhase0d] = 10;
      break;
    case 3:
      search_thresholds[kPhase1b] = 28;
      search_thresholds[kPhase1c] = 28;
      search_thresholds[kPhase1d] = 28;
      break;
    case 4:
      for (int i = 0; i < 12; ++i) {
        coefs_explore[i] = 20.0;
      }
      break;
    case 5:
      for (int i = 0; i < 12; ++i) {
        coefs_explore[i] = 23.0;
      }
      break;
    case 6:
      for (int i = 0; i < 12; ++i) {
        coefs_explore[i] = 29.0;
      }
      break;
    case 7:
      for (int i = 0; i < 12; ++i) {
        coefs_explore[i] = 32.0;
      }
      break;
    default:
      break;
  }
}

int parseX(const char c) {
  return c - 'a';
}

int parseY(const char c) {
  return '9' - c;
}

int parseTile(const char c) {
  switch (c) {
    case 'f':
      return 1;
    case 'h':
      return 2;
    case 's':
      return 3;
    case 'w':
      return 4;
    case 'M':
      return 5;
    case 'C':
      return 6;
    case 'B':
      return 7;
    default:
      return 0;
  }
}

int parseZone(const char c) {
  switch (c) {
    case '9':
      return 0;
    case '8':
      return 1;
    case '7':
      return 2;
    case '6':
      return 3;
    case '5':
      return 4;
    case '4':
      return 5;
    case '3':
      return 6;
    case '2':
      return 7;
    case '1':
      return 8;
    case 'a':
      return 9;
    case 'b':
      return 10;
    case 'c':
      return 11;
    case 'd':
      return 12;
    case 'e':
      return 13;
    case 'f':
      return 14;
    case 'g':
      return 15;
    case 'h':
      return 16;
    case 'i':
      return 17;
    case 'Q':
      return 18;
    case 'W':
      return 19;
    case 'E':
      return 20;
    case 'A':
      return 21;
    case 'S':
      return 22;
    case 'D':
      return 23;
    case 'Z':
      return 24;
    case 'X':
      return 25;
    default:
      return 26;
  }
}

}  // namespace tinybot
