#ifndef TINYBOT_NPY_H
#define TINYBOT_NPY_H

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "cnpy/cnpy.h"

#include "board.h"
#include "constants.h"
#include "deck.h"
#include "misc.h"

namespace tinybot {

int npy(bool legacy_data);
void insert_input_local(std::vector<float>& input_local_tmp, float* input_local, int& symmetry);
void insert_output_policy(std::vector<float>& output_policy_tmp, int x, int y, int t, int& symmetry);
void insert_output_policy_pass(std::vector<float>& output_policy_tmp, int& symmetry);
void insert_output_land(std::vector<float>& output_land_tmp, float* input_local, int& symmetry);

}  // namespace tinybot

#endif  // TINYBOT_NPY_H
