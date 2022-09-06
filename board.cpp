#include "board.h"

namespace tinybot {

void Board::display() {
  for (int y = 0; y < 9; ++y) {
    bool printing_land = false;
    for (int x = 0; x < 9; ++x) {
      const int z = y * 9 + x;
      if (land[z] == 0 || land[z] > kDrawFlag) {
        if (printing_land) {
          std::cout << "\u001B[0m";
          printing_land = false;
        }
      } else if (!printing_land) {
        std::cout << "\u001B[7m";
        printing_land = true;
      }
      std::cout << kTileChars[tile[z]];
    }
    if (printing_land) {
      std::cout << "\u001B[0m";
    }
    std::cout << "\n";
  }
  calculateScore();
  for (int i : score) {
    std::cout << i << ' ';
  }
  std::cout << "\n";
}

void Board::displayInputs() {
  for (const auto x : input_global) {
    std::cout << x << ' ';
  }
  std::cout << "\n";
  for (int i = 0; i < 21; ++i) {
    std::cout << i << ":\n";
    for (int y = 0; y < 9; ++y) {
      for (int x = 0; x < 9; ++x) {
        std::cout << input_local[(y * 9 + x) * 21 + i] << ' ';
      }
      std::cout << "\n";
    }
  }
}

void Board::doOffer(IDeck& d, std::vector<int>* moves) {
  d.offer(this);
  for (int i = 0; i < 2; ++i) {
    input_global[offer_tile[i]] = static_cast<float>(deck[offer_tile[i]]) / kDeckDenom[offer_tile[i]];
    const int zone_channel = offer_zone[i] / 9 + 16;
    for (const int z : kZoneZ[offer_zone[i]]) {
      input_local[z * 21 + zone_channel] = static_cast<float>(zone[offer_zone[i]]) / 2.0f;
      if (tile[z] == kBlank) {
        const int move = z * 21 + 8 + offer_tile[i];
        if (input_local[move] != 1.0f) {
          input_local[move] = 1.0f;
          if (moves) {
            moves->push_back(move);
          }
        }
      }
    }
  }
  is_player_turn = true;
}

void Board::initDraw() {
  ink = 2;
  draw_mode = kDrawModeInitial;
  input_global[0] = kInkInitial;
  //  is_player_turn = true;  // should already be true
  switch (drawings_completed) {
    case 0:
      for (int z = 0; z < 81; ++z) {
        input_local[z * 21 + 8] = kDrawPriority[0][z];
      }
      break;
    case 1:
      initDrawLimitedIndirect();
      break;
    default:
      initThirdDraw();
      break;
  }
}

void Board::initDrawLimitedIndirect() {
  float max_priority = 0;
  std::vector<int> best{};
  for (int z = 0; z < 81; ++z) {
    const int x = z % 9;
    if (land[z] == 0) {
      const int y = z / 9;
      const float p = kDrawPriority[0][z];
      if (canDirectWater(z, x, y)) {
        input_local[z * 21 + 8] = p;
      } else {
        input_local[z * 21 + 8] = kDrawForbidden;
        if (p > max_priority) {
          max_priority = p;
          best.clear();
          best.push_back(z);
        } else if (p == max_priority) {
          best.push_back(z);
        }
      }
    } else {
      if (x == 0 || x == 8) {
        input_local[z * 21 + 8] = kDrawForbidden;
        initDrawForbiddingIndirect(z);
        return;
      }
      const int y = z / 9;
      if (y == 0 || y == 8) {
        input_local[z * 21 + 8] = kDrawForbidden;
        initDrawForbiddingIndirect(z);
        return;
      }
      input_local[z * 21 + 8] = canDirectLand(z) ? kDrawPriority[0][z] : kDrawForbidden;
    }
  }
  if (enoughInkForSingleIndirect()) {
    for (int z : best) {
      input_local[z * 21 + 8] = 1.0;
    }
  }
}

void Board::initDrawSpecificIndirect(std::bitset<81>& outside) {
  float max_priority = 0;
  std::vector<int> best{};
  for (int z = 0; z < 81; ++z) {
    const float p = kDrawPriority[0][z];
    if (outside.test(z)) {
      input_local[z * 21 + 8] = p;
    } else {
      input_local[z * 21 + 8] = kDrawForbidden;
      if (p > max_priority) {
        max_priority = p;
        best.clear();
        best.push_back(z);
      } else if (p == max_priority) {
        best.push_back(z);
      }
    }
  }
  for (int z : best) {
    input_local[z * 21 + 8] = 1.0;
  }
}

void Board::initDrawForbiddingIndirect(int z) {
  for (++z; z < 81; ++z) {
    const int x = z % 9;
    if (land[z] == 0) {
      const int y = z / 9;
      input_local[z * 21 + 8] = canDirectWater(z, x, y) ? kDrawPriority[0][z] : kDrawForbidden;
    } else {
      if (x == 0 || x == 8) {
        input_local[z * 21 + 8] = kDrawForbidden;
        continue;
      }
      const int y = z / 9;
      if (y == 0 || y == 8) {
        input_local[z * 21 + 8] = kDrawForbidden;
        continue;
      }
      input_local[z * 21 + 8] = canDirectLand(z) ? kDrawPriority[0][z] : kDrawForbidden;
    }
  }
}

void Board::initThirdDraw() {
  // ink and draw mode already set
  std::stack<int> fill;

  int ink_1 = 0;
  std::bitset<81> outside_1;
  for (int z = 0; z < 8; ++z) {
    if (land[z] == 1 || land[z + 73] == 1 || land[z * 9 + 8] == 1 || land[z * 9 + 9] == 1) {
      ink_1 = -1;
      break;
    }
  }
  if (ink_1 == 0) {
    if (canDirectWaterSpecific(1, 0)) {
      outside_1.set(0);
      fill.push(0);
    } else {
      ink_1 += 2;
    }
    if (canDirectWaterSpecific(1, 8)) {
      outside_1.set(8);
      fill.push(8);
    } else {
      ink_1 += 2;
    }
    if (canDirectWaterSpecific(1, 72)) {
      outside_1.set(72);
      fill.push(72);
    } else {
      ink_1 += 2;
    }
    if (canDirectWaterSpecific(1, 80)) {
      outside_1.set(80);
      fill.push(80);
    } else {
      ink_1 += 2;
    }
    for (int z = 1; z <= 7; ++z) {
      const int b = z + 72;
      const int c = z * 9;
      const int d = z * 9 + 8;
      if (canDirectWaterSpecific(1, z)) {
        outside_1.set(z);
        fill.push(z);
      } else {
        ++ink_1;
      }
      if (canDirectWaterSpecific(1, b)) {
        outside_1.set(b);
        fill.push(b);
      } else {
        ++ink_1;
      }
      if (canDirectWaterSpecific(1, c)) {
        outside_1.set(c);
        fill.push(c);
      } else {
        ++ink_1;
      }
      if (canDirectWaterSpecific(1, d)) {
        outside_1.set(d);
        fill.push(d);
      } else {
        ++ink_1;
      }
    }
    while (!fill.empty()) {
      const int z = fill.top();
      fill.pop();
      const int x = z % 9;
      const int y = z / 9;
      if (x != 0 && !outside_1.test(z - 1)) {
        if (canDirectWaterSpecific(1, z - 1, x - 1, y)) {
          outside_1.set(z - 1);
          fill.push(z - 1);
        } else {
          ++ink_1;
        }
      }
      if (x != 8 && !outside_1.test(z + 1)) {
        if (canDirectWaterSpecific(1, z + 1, x + 1, y)) {
          outside_1.set(z + 1);
          fill.push(z + 1);
        } else {
          ++ink_1;
        }
      }
      if (y != 0 && !outside_1.test(z - 9)) {
        if (canDirectWaterSpecific(1, z - 9, x, y - 1)) {
          outside_1.set(z - 9);
          fill.push(z - 9);
        } else {
          ++ink_1;
        }
      }
      if (y != 8 && !outside_1.test(z + 9)) {
        if (canDirectWaterSpecific(1, z + 9, x, y + 1)) {
          outside_1.set(z + 9);
          fill.push(z + 9);
        } else {
          ++ink_1;
        }
      }
    }
  }

  int ink_2 = 0;
  std::bitset<81> outside_2;
  for (int z = 0; z < 8; ++z) {
    if (land[z] == 2 || land[z + 73] == 2 || land[z * 9 + 8] == 2 || land[z * 9 + 9] == 2) {
      ink_2 = -1;
      break;
    }
  }
  if (ink_2 == 0) {
    if (canDirectWaterSpecific(2, 0)) {
      outside_2.set(0);
      fill.push(0);
    } else {
      ink_2 += 2;
    }
    if (canDirectWaterSpecific(2, 8)) {
      outside_2.set(8);
      fill.push(8);
    } else {
      ink_2 += 2;
    }
    if (canDirectWaterSpecific(2, 72)) {
      outside_2.set(72);
      fill.push(72);
    } else {
      ink_2 += 2;
    }
    if (canDirectWaterSpecific(2, 80)) {
      outside_2.set(80);
      fill.push(80);
    } else {
      ink_2 += 2;
    }
    for (int z = 1; z <= 7; ++z) {
      const int b = z + 72;
      const int c = z * 9;
      const int d = z * 9 + 8;
      if (canDirectWaterSpecific(2, z)) {
        outside_2.set(z);
        fill.push(z);
      } else {
        if (ink_1 != -1 && !outside_1.test(z)) {
          indirect_overlap = true;
          initDrawLimitedIndirect();
          return;
        }
        ++ink_2;
      }
      if (canDirectWaterSpecific(2, b)) {
        outside_2.set(b);
        fill.push(b);
      } else {
        if (ink_1 != -1 && !outside_1.test(b)) {
          indirect_overlap = true;
          initDrawLimitedIndirect();
          return;
        }
        ++ink_2;
      }
      if (canDirectWaterSpecific(2, c)) {
        outside_2.set(c);
        fill.push(c);
      } else {
        if (ink_1 != -1 && !outside_1.test(c)) {
          indirect_overlap = true;
          initDrawLimitedIndirect();
          return;
        }
        ++ink_2;
      }
      if (canDirectWaterSpecific(2, d)) {
        outside_2.set(d);
        fill.push(d);
      } else {
        if (ink_1 != -1 && !outside_1.test(d)) {
          indirect_overlap = true;
          initDrawLimitedIndirect();
          return;
        }
        ++ink_2;
      }
    }
    while (!fill.empty()) {
      const int z = fill.top();
      fill.pop();
      const int x = z % 9;
      const int y = z / 9;
      if (x != 0 && !outside_2.test(z - 1)) {
        if (canDirectWaterSpecific(2, z - 1, x - 1, y)) {
          outside_2.set(z - 1);
          fill.push(z - 1);
        } else {
          if (ink_1 != -1 && !outside_1.test(z - 1)) {
            indirect_overlap = true;
            initDrawLimitedIndirect();
            return;
          }
          ++ink_2;
        }
      }
      if (x != 8 && !outside_2.test(z + 1)) {
        if (canDirectWaterSpecific(2, z + 1, x + 1, y)) {
          outside_2.set(z + 1);
          fill.push(z + 1);
        } else {
          if (ink_1 != -1 && !outside_1.test(z + 1)) {
            indirect_overlap = true;
            initDrawLimitedIndirect();
            return;
          }
          ++ink_2;
        }
      }
      if (y != 0 && !outside_2.test(z - 9)) {
        if (canDirectWaterSpecific(2, z - 9, x, y - 1)) {
          outside_2.set(z - 9);
          fill.push(z - 9);
        } else {
          if (ink_1 != -1 && !outside_1.test(z - 9)) {
            indirect_overlap = true;
            initDrawLimitedIndirect();
            return;
          }
          ++ink_2;
        }
      }
      if (y != 8 && !outside_2.test(z + 9)) {
        if (canDirectWaterSpecific(2, z + 9, x, y + 1)) {
          outside_2.set(z + 9);
          fill.push(z + 9);
        } else {
          if (ink_1 != -1 && !outside_1.test(z + 9)) {
            indirect_overlap = true;
            initDrawLimitedIndirect();
            return;
          }
          ++ink_2;
        }
      }
    }
    if (ink_2 != 0) {
      switch (ink_1) {
        case -1:
          if (ink_2 <= 24) {
            for (int z = 0; z < 81; ++z) {
              if (!outside_2.test(z) && !canDirectWaterSpecific(1, z)) {
                indirect_overlap = true;
                break;
              }
            }
            if (indirect_overlap) {
              initDrawForbiddingIndirect(-1);
            } else {
              initDrawSpecificIndirect(outside_2);
            }
          } else {
            initDrawForbiddingIndirect(-1);
          }
          return;
        case 0:  // TODO implement indirect around lake
          if (ink_2 <= 24) {
            initDrawSpecificIndirect(outside_2);
          } else {
            initDrawForbiddingIndirect(-1);
          }
          return;
          //        case 0:
          //          break;
        default:
          if (ink_1 <= 24) {
            if (ink_2 <= 24) {
              float max_priority_1 = 0;
              float max_priority_2 = 0;
              std::vector<int> best_1{};
              std::vector<int> best_2{};
              for (int z = 0; z < 81; ++z) {
                const float p = kDrawPriority[0][z];
                if (outside_1.test(z)) {
                  if (outside_2.test(z)) {
                    input_local[z * 21 + 8] = p;
                  } else {
                    input_local[z * 21 + 8] = kDrawForbidden;
                    if (p > max_priority_2) {
                      max_priority_2 = p;
                      best_2.clear();
                      best_2.push_back(z);
                    } else if (p == max_priority_2) {
                      best_2.push_back(z);
                    }
                  }
                } else {
                  input_local[z * 21 + 8] = kDrawForbidden;
                  if (p > max_priority_1) {
                    max_priority_1 = p;
                    best_1.clear();
                    best_1.push_back(z);
                  } else if (p == max_priority_1) {
                    best_1.push_back(z);
                  }
                }
              }
              for (int z : best_1) {
                input_local[z * 21 + 8] = 1.0;
              }
              for (int z : best_2) {
                input_local[z * 21 + 8] = 1.0;
              }
            } else {
              initDrawSpecificIndirect(outside_1);
            }
          } else {
            if (ink_2 <= 24) {
              initDrawSpecificIndirect(outside_2);
            } else {
              initDrawForbiddingIndirect(-1);
            }
          }
          return;
      }
    } else {
      // TODO implement indirect around lake
      if (ink_1 != -1 && ink_1 <= 24) {
        initDrawSpecificIndirect(outside_1);
      } else {
        initDrawForbiddingIndirect(-1);
      }
    }
  } else {
    // ink_2 == -1
    switch (ink_1) {
      case -1:
      case 0:  // TODO implement indirect around lake
        initDrawForbiddingIndirect(-1);
        return;
        //      case 0:
        //        break;
      default:
        if (ink_1 <= 24) {
          for (int z = 0; z < 81; ++z) {
            if (!outside_1.test(z) && !canDirectWaterSpecific(2, z)) {
              indirect_overlap = true;
              break;
            }
          }
          if (indirect_overlap) {
            initDrawForbiddingIndirect(-1);
          } else {
            initDrawSpecificIndirect(outside_1);
          }
        } else {
          initDrawForbiddingIndirect(-1);
        }
        return;
    }
  }

  // TODO implement indirect around lake
  // ink_n interpretation: if ink_n == 0, n is a lake, otherwise it's an island; if 0 < ink_n <= 24, indirect is possible; note 1 and 2 can't both be lake
  // without having implemented indirect around lake, currently treating 0 as indirect impossible
}

bool Board::canDirectWater(const int z, const int x, const int y) {
  return (y == 0 || (x == 0 || land[z - 10] == 0) && land[z - 9] == 0 && (x == 8 || land[z - 8] == 0)) && (x == 0 || land[z - 1] == 0) && (x == 8 || land[z + 1] == 0) && (y == 8 || (x == 0 || land[z + 8] == 0) && land[z + 9] == 0 && (x == 8 || land[z + 10] == 0));
}

bool Board::canDirectLand(const int z) {
  return land[z] == land[z - 10] && land[z] == land[z - 9] && land[z] == land[z - 8] && land[z] == land[z - 1] && land[z] == land[z + 1] && land[z] == land[z + 8] && land[z] == land[z + 9] && land[z] == land[z + 10];
}

bool Board::canDirectWaterSpecific(const int l, const int z) {
  return canDirectWaterSpecific(l, z, z % 9, z / 9);
}

bool Board::canDirectWaterSpecific(const int l, const int z, const int x, const int y) {
  return (y == 0 || (x == 0 || land[z - 10] != l) && land[z - 9] != l && (x == 8 || land[z - 8] != l)) && (x == 0 || land[z - 1] != l) && (x == 8 || land[z + 1] != l) && (y == 8 || (x == 0 || land[z + 8] != l) && land[z + 9] != l && (x == 8 || land[z + 10] != l));
}

bool Board::enoughInkForSingleIndirect() {
  int indirect_ink = 0;
  std::bitset<81> outside;
  std::stack<int> fill;
  if (input_local[8] == kDrawForbidden) {
    indirect_ink += 2;
  } else {
    outside.set(0);
    fill.push(0);
  }
  if (input_local[8 * 21 + 8] == kDrawForbidden) {
    indirect_ink += 2;
  } else {
    outside.set(8);
    fill.push(8);
  }
  if (input_local[72 * 21 + 8] == kDrawForbidden) {
    indirect_ink += 2;
  } else {
    outside.set(72);
    fill.push(72);
  }
  if (input_local[80 * 21 + 8] == kDrawForbidden) {
    indirect_ink += 2;
  } else {
    outside.set(80);
    fill.push(80);
  }
  for (int z = 1; z <= 7; ++z) {
    const int b = z + 72;
    const int c = z * 9;
    const int d = z * 9 + 8;
    if (input_local[z * 21 + 8] == kDrawForbidden) {
      ++indirect_ink;
    } else {
      outside.set(z);
      fill.push(z);
    }
    if (input_local[b * 21 + 8] == kDrawForbidden) {
      ++indirect_ink;
    } else {
      outside.set(b);
      fill.push(b);
    }
    if (input_local[c * 21 + 8] == kDrawForbidden) {
      ++indirect_ink;
    } else {
      outside.set(c);
      fill.push(c);
    }
    if (input_local[d * 21 + 8] == kDrawForbidden) {
      ++indirect_ink;
    } else {
      outside.set(d);
      fill.push(d);
    }
  }
  while (!fill.empty()) {
    const int z = fill.top();
    fill.pop();
    const int x = z % 9;
    if (x != 0 && !outside.test(z - 1)) {
      if (input_local[z * 21 - 13] == kDrawForbidden) {
        ++indirect_ink;
        if (indirect_ink == 25) {
          return false;
        }
      } else {
        outside.set(z - 1);
        fill.push(z - 1);
      }
    }
    if (x != 8 && !outside.test(z + 1)) {
      if (input_local[z * 21 + 29] == kDrawForbidden) {
        ++indirect_ink;
        if (indirect_ink == 25) {
          return false;
        }
      } else {
        outside.set(z + 1);
        fill.push(z + 1);
      }
    }
    const int y = z / 9;
    if (y != 0 && !outside.test(z - 9)) {
      if (input_local[z * 21 - 181] == kDrawForbidden) {
        ++indirect_ink;
        if (indirect_ink == 25) {
          return false;
        }
      } else {
        outside.set(z - 9);
        fill.push(z - 9);
      }
    }
    if (y != 8 && !outside.test(z + 9)) {
      if (input_local[z * 21 + 197] == kDrawForbidden) {
        ++indirect_ink;
        if (indirect_ink == 25) {
          return false;
        }
      } else {
        outside.set(z + 9);
        fill.push(z + 9);
      }
    }
  }
  return true;
}

void Board::doDraw(const int z) {
  if (z == kMovePass) {
    is_player_turn = false;
    ++drawings_completed;
    for (int& zz : land) {
      if (zz == kDrawFlag) {
        zz = drawings_completed;
      } else if (zz > kDrawFlag) {
        zz = 0;
      }
    }
    if (drawings_completed == 3) {
      return;
    }
    ink = 0;
    placements_until_draw = drawings_completed == 2 ? 8 : 9;
    input_global[0] = 0;
    input_global[9] = static_cast<float>(placements_until_draw) / 9.0f;
    input_global[10] = static_cast<float>(drawings_completed) / 2.0f;
    for (int zz = 0; zz < 81; ++zz) {
      input_local[zz * 21 + 8] = kDrawForbidden;
    }
    return;
  }

  const int x = z % 9;
  const int y = z / 9;
  if (ink == 2) {
    // TODO implement indirect around lake
    if (land[z] == 0 && input_local[z * 21 + 8] == 1.0f) {
      doDrawIndirect(drawings_completed != 2 || indirect_overlap ? 0 : findIndirect(z, x, y), x, y);
      return;
    } else {
      ink = 4;
      input_global[0] = kInkMinimal;
    }
  } else {
    ink += (x == 0 || land[z - 1] < kDrawFlag ? 1 : -1) + (x == 8 || land[z + 1] < kDrawFlag ? 1 : -1) + (y == 0 || land[z - 9] < kDrawFlag ? 1 : -1) + (y == 8 || land[z + 9] < kDrawFlag ? 1 : -1);
    input_global[0] = static_cast<float>(ink) / 24.0f;
  }
  input_local[z * 21] = land[z] == 0 ? 1 : 0;
  land[z] += kDrawFlag;
  const float priority_plus_epsilon = input_local[z * 21 + 8] + 0.00390625f;  // 1/256 is a nice round number smaller than the smallest priority increment of 1/164
  input_local[z * 21 + 8] = kDrawForbidden;
  updateDrawMode(x, y);
  if (ink == 24) {
    for (int zz = 0; zz < 81; ++zz) {
      const float p = input_local[zz * 21 + 8];
      if (p == kDrawUnavailable) {
        if (isDrawableWithoutInk(land[z], zz)) {
          input_local[zz * 21 + 8] = kDrawPriority[draw_mode][zz];
        }
      } else if (p > kDrawUnavailable) {
        if (p < priority_plus_epsilon) {
          input_local[zz * 21 + 8] = isDrawableWithoutInk(land[z], zz) ? kDrawPriority[draw_mode][zz] : kDrawUnavailable;
        } else {
          input_local[zz * 21 + 8] = kDrawForbidden;
        }
      }
    }
  } else {
    for (int zz = 0; zz < 81; ++zz) {
      const float p = input_local[zz * 21 + 8];
      if (p == kDrawUnavailable) {
        if (isDrawable(land[z], zz)) {
          input_local[zz * 21 + 8] = kDrawPriority[draw_mode][zz];
        }
      } else if (p > kDrawUnavailable) {
        if (p < priority_plus_epsilon) {
          input_local[zz * 21 + 8] = isDrawable(land[z], zz) ? kDrawPriority[draw_mode][zz] : kDrawUnavailable;
        } else {
          input_local[zz * 21 + 8] = kDrawForbidden;
        }
      }
    }
  }
}

void Board::doDrawIndirect(const int l, const int xx, const int yy) {
  // TODO implement indirect around lake
  ink = 0;
  std::bitset<81> outside;
  std::stack<int> fill;
  if ((input_local[8] == kDrawForbidden || input_local[8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, 0))) {
    ink += 2;
  } else {
    outside.set(0);
    fill.push(0);
  }
  if ((input_local[8 * 21 + 8] == kDrawForbidden || input_local[8 * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, 8))) {
    ink += 2;
  } else {
    outside.set(8);
    fill.push(8);
  }
  if ((input_local[72 * 21 + 8] == kDrawForbidden || input_local[72 * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, 72))) {
    ink += 2;
  } else {
    outside.set(72);
    fill.push(72);
  }
  if ((input_local[80 * 21 + 8] == kDrawForbidden || input_local[80 * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, 80))) {
    ink += 2;
  } else {
    outside.set(80);
    fill.push(80);
  }
  for (int z = 1; z <= 7; ++z) {
    const int b = z + 72;
    const int c = z * 9;
    const int d = z * 9 + 8;
    if ((input_local[z * 21 + 8] == kDrawForbidden || input_local[z * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, z))) {
      ++ink;
    } else {
      outside.set(z);
      fill.push(z);
    }
    if ((input_local[b * 21 + 8] == kDrawForbidden || input_local[b * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, b))) {
      ++ink;
    } else {
      outside.set(b);
      fill.push(b);
    }
    if ((input_local[c * 21 + 8] == kDrawForbidden || input_local[c * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, c))) {
      ++ink;
    } else {
      outside.set(c);
      fill.push(c);
    }
    if ((input_local[d * 21 + 8] == kDrawForbidden || input_local[d * 21 + 8] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, d))) {
      ++ink;
    } else {
      outside.set(d);
      fill.push(d);
    }
  }
  while (!fill.empty()) {
    const int z = fill.top();
    fill.pop();
    const int x = z % 9;
    const int y = z / 9;
    if (x != 0 && !outside.test(z - 1)) {
      if ((input_local[z * 21 - 13] == kDrawForbidden || input_local[z * 21 - 13] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, z - 1, x - 1, y))) {
        ++ink;
      } else {
        outside.set(z - 1);
        fill.push(z - 1);
      }
    }
    if (x != 8 && !outside.test(z + 1)) {
      if ((input_local[z * 21 + 29] == kDrawForbidden || input_local[z * 21 + 29] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, z + 1, x + 1, y))) {
        ++ink;
      } else {
        outside.set(z + 1);
        fill.push(z + 1);
      }
    }
    if (y != 0 && !outside.test(z - 9)) {
      if ((input_local[z * 21 - 181] == kDrawForbidden || input_local[z * 21 - 181] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, z - 9, x, y - 1))) {
        ++ink;
      } else {
        outside.set(z - 9);
        fill.push(z - 9);
      }
    }
    if (y != 8 && !outside.test(z + 9)) {
      if ((input_local[z * 21 + 197] == kDrawForbidden || input_local[z * 21 + 197] == 1.0f) && (l == 0 || !canDirectWaterSpecific(l, z + 9, x, y + 1))) {
        ++ink;
      } else {
        outside.set(z + 9);
        fill.push(z + 9);
      }
    }
  }

  for (int z = 0; z < 81; ++z) {
    if (!outside.test(z)) {
      input_local[z * 21] = land[z] == 0 ? 1 : 0;
      land[z] += kDrawFlag;
      input_local[z * 21 + 8] = kDrawForbidden;
    }
  }
  input_global[0] = static_cast<float>(ink) / 24.0f;
  updateDrawMode(xx, yy);
  if (ink == 24) {
    for (int z = 0; z < 81; ++z) {
      if (outside.test(z)) {
        input_local[z * 21 + 8] = isDrawableWithoutInk(kDrawFlag, z) ? kDrawPriority[draw_mode][z] : kDrawUnavailable;
      }
    }
  } else {
    for (int z = 0; z < 81; ++z) {
      if (outside.test(z)) {
        input_local[z * 21 + 8] = isDrawable(kDrawFlag, z) ? kDrawPriority[draw_mode][z] : kDrawUnavailable;
      }
    }
  }
}

void Board::updateDrawMode(const int x, const int y) {
  switch (draw_mode) {
    case kDrawModeInitial:
      if (x < 4) {
        if (y < 4) {
          const int ax = abs(x - 4);
          const int ay = abs(y - 4);
          if (ax < ay) {
            draw_mode = kDrawModeNW;
          } else if (ax == ay) {
            draw_mode = kDrawModeUL;
          } else {
            draw_mode = kDrawModeWN;
          }
        } else if (y == 4) {
          draw_mode = kDrawModeW;
        } else {
          const int ax = abs(x - 4);
          const int ay = abs(y - 4);
          if (ax < ay) {
            draw_mode = kDrawModeSW;
          } else if (ax == ay) {
            draw_mode = kDrawModeLL;
          } else {
            draw_mode = kDrawModeWS;
          }
        }
      } else if (x == 4) {
        if (y < 4) {
          draw_mode = kDrawModeN;
        } else if (y != 4) {
          draw_mode = kDrawModeS;
        }
      } else {
        if (y < 4) {
          const int ax = abs(x - 4);
          const int ay = abs(y - 4);
          if (ax < ay) {
            draw_mode = kDrawModeNE;
          } else if (ax == ay) {
            draw_mode = kDrawModeUR;
          } else {
            draw_mode = kDrawModeEN;
          }
        } else if (y == 4) {
          draw_mode = kDrawModeE;
        } else {
          const int ax = abs(x - 4);
          const int ay = abs(y - 4);
          if (ax < ay) {
            draw_mode = kDrawModeSE;
          } else if (ax == ay) {
            draw_mode = kDrawModeLR;
          } else {
            draw_mode = kDrawModeES;
          }
        }
      }
      break;
    case kDrawModeUL:
      if (x < y) {
        draw_mode = kDrawModeWN;
      } else if (x != y) {
        draw_mode = kDrawModeNW;
      }
      break;
    case kDrawModeUR:
      if (8 - x < y) {
        draw_mode = kDrawModeEN;
      } else if (8 - x != y) {
        draw_mode = kDrawModeNE;
      }
      break;
    case kDrawModeLL:
      if (x < 8 - y) {
        draw_mode = kDrawModeWS;
      } else if (x != 8 - y) {
        draw_mode = kDrawModeSW;
      }
      break;
    case kDrawModeLR:
      if (x < y) {
        draw_mode = kDrawModeSE;
      } else if (x != y) {
        draw_mode = kDrawModeES;
      }
      break;
    case kDrawModeW:
      if (y < 4) {
        draw_mode = kDrawModeWN;
      } else if (y != 4) {
        draw_mode = kDrawModeWS;
      }
      break;
    case kDrawModeE:
      if (y < 4) {
        draw_mode = kDrawModeEN;
      } else if (y != 4) {
        draw_mode = kDrawModeES;
      }
      break;
    case kDrawModeN:
      if (x < 4) {
        draw_mode = kDrawModeNW;
      } else if (x != 4) {
        draw_mode = kDrawModeNE;
      }
      break;
    case kDrawModeS:
      if (x < 4) {
        draw_mode = kDrawModeSW;
      } else if (x != 4) {
        draw_mode = kDrawModeSE;
      }
      break;
    default:
      break;
  }
}

bool Board::isDrawable(const int drawn, const int z) {
  const int x = z % 9;
  const int y = z / 9;
  bool found_drawn = false;
  int mask = 0;
  if (y != 0) {
    if (land[z - 9] == drawn) {
      found_drawn = true;
    }
    if (x != 0 && isLand(land[z - 10])) {
      mask += 1;
    }
    if (isLand(land[z - 9])) {
      mask += 2;
    }
    if (x != 8 && isLand(land[z - 8])) {
      mask += 4;
    }
  }
  if (x != 8) {
    if (land[z + 1] == drawn) {
      found_drawn = true;
    }
    if (isLand(land[z + 1])) {
      mask += 8;
    }
  }
  if (y != 8) {
    if (land[z + 9] == drawn) {
      found_drawn = true;
    }
    if (x != 8 && isLand(land[z + 10])) {
      mask += 16;
    }
    if (isLand(land[z + 9])) {
      mask += 32;
    }
    if (x != 0 && isLand(land[z + 8])) {
      mask += 64;
    }
  }
  if (x != 0) {
    if (land[z - 1] == drawn) {
      found_drawn = true;
    }
    if (isLand(land[z - 1])) {
      mask += 128;
    }
  }
  return found_drawn && isCoast(mask);
}

bool Board::isDrawableWithoutInk(const int drawn, const int z) {
  const int x = z % 9;
  const int y = z / 9;
  int found_drawn = 0;
  int mask = 0;
  if (y != 0) {
    if (land[z - 9] == drawn) {
      ++found_drawn;
    }
    if (x != 0 && isLand(land[z - 10])) {
      mask += 1;
    }
    if (isLand(land[z - 9])) {
      mask += 2;
    }
    if (x != 8 && isLand(land[z - 8])) {
      mask += 4;
    }
  }
  if (x != 8) {
    if (land[z + 1] == drawn) {
      ++found_drawn;
    }
    if (isLand(land[z + 1])) {
      mask += 8;
    }
  }
  if (y != 8) {
    if (land[z + 9] == drawn) {
      ++found_drawn;
    }
    if (x != 8 && isLand(land[z + 10])) {
      mask += 16;
    }
    if (isLand(land[z + 9])) {
      mask += 32;
    }
    if (x != 0 && isLand(land[z + 8])) {
      mask += 64;
    }
  }
  if (x != 0) {
    if (land[z - 1] == drawn) {
      ++found_drawn;
    }
    if (isLand(land[z - 1])) {
      mask += 128;
    }
  }
  return found_drawn >= 2 && isCoast(mask);
}

int Board::findIndirect(const int z, const int x, const int y) {
  if (y != 8) {
    if (x != 0 && land[z + 8] != 0) {
      return land[z + 8];
    }
    if (x != 8 && land[z + 10] != 0) {
      return land[z + 10];
    }
  }
  if (y != 0 && x != 8 && land[z - 8] != 0) {
    return land[z - 8];
  }
  return land[z - 10];
}

void Board::doPlace(const int z, const int t) {
  tile[z] = t;
  --placements_remaining;
  --placements_until_draw;
  input_global[8] = static_cast<float>(placements_remaining) / 26.0f;
  input_global[9] = static_cast<float>(placements_until_draw) / 9.0f;
  input_local[z * 21 + t] = 1.0;
  for (int i = 0; i < 2; ++i) {
    for (const int zz : kZoneZ[offer_zone[i]]) {
      input_local[zz * 21 + 8 + offer_tile[i]] = 0.0;
    }
  }
  if (t == kWave) {
    const int x = z % 9;
    const int y = z / 9;
    for (int i = 0; i < 9; ++i) {
      const int i19 = (i * 9 + x) * 21 + 19;
      const int i20 = (y * 9 + i) * 21 + 20;
      if (input_local[i19] == 0.0f) {
        input_local[i19] = 0.5f;
      } else if (input_local[i19] == 0.5f) {
        input_local[i19] = 1.0;
      }
      if (input_local[i20] == 0.0f) {
        input_local[i20] = 0.5f;
      } else if (input_local[i20] == 0.5f) {
        input_local[i20] = 1.0;
      }
    }
  }

  if (placements_until_draw == 0) {
    initDraw();
  } else {
    is_player_turn = false;  // time for offer
  }
}

void Board::calculateScore() {
  if (is_final_score) {
    return;  // is_final_score means final score already calculated
  }
  if (drawings_completed == 3) {
    is_final_score = true;
  }
  memset(score, 0, sizeof(score));

  std::unordered_map<int, int> forest_ids;
  UnionFind forest_uf = UnionFind(13);
  int waves[28]{};     // up to 9 locations, then 9 rows, then 9 cols, then waves[27] is the count
  int buildings[6]{};  // house counts then church (location+1)s, where -1 signifies multiple churches

  for (int z = 0; z < 81; ++z) {
    switch (tile[z]) {
      case kForest:
        if (land[z] == 0) {
          score[0] += 5;
          break;
        }
        calcForest(forest_uf, forest_ids, z);
        break;
      case kHouse:
        if (land[z] == 0) {
          score[0] += 5;
          break;
        }
        calcHouse(z);
        ++buildings[land[z] - 1];
        break;
      case kSand:
        if (land[z] != 0) {
          score[0] += 5;
          break;
        }
        calcSand(z);
        break;
      case kWave:
        if (land[z] != 0) {
          score[0] += 5;
          break;
        }
        calcWave(waves, z);
        break;
      case kMountain:
        if (land[z] == 0) {
          score[0] += 5;
          break;
        }
        score[kMountain] += calcNear(kForest, false, z) << 1;
        break;
      case kChurch:
        if (land[z] == 0) {
          score[0] += 5;
          break;
        }
        buildings[land[z] + 2] = buildings[land[z] + 2] == 0 ? z + 1 : -1;
        break;
      case kBoat:
        if (land[z] != 0) {
          score[0] += 5;
          break;
        }
        calcBoat(z);
        break;
      default:
        break;
    }
  }

  score[kForest] += (13 - forest_uf.getCount()) << 1;
  const int w = waves[27];
  for (int i = 0; i < w; ++i) {
    const int z = waves[i];
    if (waves[z / 9 + 9] == 1 && waves[z % 9 + 18] == 1) {
      score[kWave] += 2;
    }
  }
  for (int island = 1; island <= 3; ++island) {
    const int zz = buildings[island + 2];
    if (zz > 0) {
      score[kChurch] += buildings[island - 1] + calcNear(kHouse, false, zz - 1);
    }
  }
  score[8] = score[1] + score[2] + score[3] + score[4] + score[5] + score[6] + score[7] - score[0];
}

int Board::calcNear(const int t, const bool is_water_tile, const int z) {
  std::vector<int> neighbors = getNeighbors(z);
  int found = 0;
  for (const auto zz : neighbors) {
    if (tile[zz] == t && (land[zz] == 0) == is_water_tile) {
      ++found;
    }
  }
  return found;
}

void Board::calcForest(UnionFind& forest_uf, std::unordered_map<int, int>& forest_ids, const int z) {
  const int id = calcForestId(forest_ids, z);
  const int x = z % 9;
  const int y = z / 9;
  if (x != 0) {
    if (tile[z - 1] == kForest && land[z - 1] != 0) {
      forest_uf.merge(id, calcForestId(forest_ids, z - 1));
    }
  }
  if (x != 8) {
    if (tile[z + 1] == kForest && land[z + 1] != 0) {
      forest_uf.merge(id, calcForestId(forest_ids, z + 1));
    }
  }
  if (y != 0) {
    if (tile[z - 9] == kForest && land[z - 9] != 0) {
      forest_uf.merge(id, calcForestId(forest_ids, z - 9));
    }
  }
  if (y != 8) {
    if (tile[z + 9] == kForest && land[z + 9] != 0) {
      forest_uf.merge(id, calcForestId(forest_ids, z + 9));
    }
  }
}

void Board::calcHouse(const int z) {
  std::vector<int> neighbors = getNeighbors(z);
  bool found[7]{};
  for (const auto zz : neighbors) {
    switch (tile[zz]) {
      case kForest:
      case kMountain:
      case kChurch:
        if (land[zz] == 0) {
          continue;
        }
        break;
      case kSand:
      case kWave:
      case kBoat:
        if (land[zz] != 0) {
          continue;
        }
        break;
      default:
        continue;
    }
    if (!found[tile[zz] - 1]) {
      found[tile[zz] - 1] = true;
      ++score[kHouse];
    }
  }
}

void Board::calcSand(const int z) {
  const int x = z % 9;
  const int y = z / 9;
  if (x != 0 && land[z - 1] != 0) {
    ++score[kSand];
  }
  if (x != 8 && land[z + 1] != 0) {
    ++score[kSand];
  }
  if (y != 0 && land[z - 9] != 0) {
    ++score[kSand];
  }
  if (y != 8 && land[z + 9] != 0) {
    ++score[kSand];
  }
}

void Board::calcWave(int (&waves)[28], const int z) {
  const int x = z % 9;
  const int y = z / 9;
  waves[waves[27]] = z;
  ++waves[27];
  ++waves[9 + y];
  ++waves[18 + x];
  if (y == 0) {
    return;
  }
  if (x != 0) {
    if (tile[z - 10] == kWave && land[z - 10] == 0) {
      ++waves[8 + y];
      ++waves[9 + y];
      ++waves[17 + x];
      ++waves[18 + x];
      return;
    }
  }
  if (x != 8) {
    if (tile[z - 8] == kWave && land[z - 8] == 0) {
      ++waves[9 + y];
      ++waves[10 + y];
      ++waves[18 + x];
      ++waves[19 + x];
    }
  }
}

void Board::calcBoat(const int boatZ) {
  if (drawings_completed == 0) {
    return;
  }
  ++score[kBoat];
  std::unordered_set<int> seen;
  std::unordered_set<int> next;
  seen.insert(boatZ);
  next.insert(boatZ);
  while (true) {
    std::unordered_set<int> current;
    swap(current, next);
    for (const auto z : current) {
      const int x = z % 9;
      const int y = z / 9;
      if (x != 0 && seen.count(z - 1) == 0) {
        if (land[z - 1] != 0 || tile[z - 1] == kBoat) {
          return;
        }
        seen.insert(z - 1);
        next.insert(z - 1);
      }
      if (x != 8 && seen.count(z + 1) == 0) {
        if (land[z + 1] != 0 || tile[z + 1] == kBoat) {
          return;
        }
        seen.insert(z + 1);
        next.insert(z + 1);
      }
      if (y != 0 && seen.count(z - 9) == 0) {
        if (land[z - 9] != 0 || tile[z - 9] == kBoat) {
          return;
        }
        seen.insert(z - 9);
        next.insert(z - 9);
      }
      if (y != 8 && seen.count(z + 9) == 0) {
        if (land[z + 9] != 0 || tile[z + 9] == kBoat) {
          return;
        }
        seen.insert(z + 9);
        next.insert(z + 9);
      }
    }
    ++score[kBoat];
  }
}

int Board::calcForestId(std::unordered_map<int, int>& forest_ids, const int z) {
  auto key_value = forest_ids.find(z);
  if (key_value != forest_ids.end()) {
    return key_value->second;
  }
  const int id = forest_ids.size();
  forest_ids[z] = id;
  return id;
}

std::vector<int> Board::getNeighbors(const int z) {
  std::vector<int> result;
  const int x = z % 9;
  const int y = z / 9;
  if (y != 0) {
    if (x != 0) {
      result.push_back(z - 10);
    }
    result.push_back(z - 9);
    if (x != 8) {
      result.push_back(z - 8);
    }
  }
  if (x != 0) {
    result.push_back(z - 1);
  }
  if (x != 8) {
    result.push_back(z + 1);
  }
  if (y != 8) {
    if (x != 0) {
      result.push_back(z + 8);
    }
    result.push_back(z + 9);
    if (x != 8) {
      result.push_back(z + 10);
    }
  }
  return result;
}

bool Board::isLand(const int l) {
  return l != 0 && l <= kDrawFlag;
}

bool Board::isCoast(const int mask) {
  switch (mask) {
    case 2:
    case 3:
    case 6:
    case 7:
    case 8:
    case 12:
    case 14:
    case 15:
    case 24:
    case 28:
    case 30:
    case 31:
    case 32:
    case 48:
    case 56:
    case 60:
    case 62:
    case 63:
    case 96:
    case 112:
    case 120:
    case 124:
    case 126:
    case 127:
    case 128:
    case 129:
    case 131:
    case 135:
    case 143:
    case 159:
    case 192:
    case 193:
    case 195:
    case 199:
    case 207:
    case 223:
    case 224:
    case 225:
    case 227:
    case 231:
    case 240:
    case 241:
    case 243:
    case 247:
    case 248:
    case 249:
    case 252:
    case 253:
      return true;
    default:
      return false;
  }
}

}  // namespace tinybot
