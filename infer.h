#ifndef TINYBOT_INFER_H
#define TINYBOT_INFER_H

#include <cstring>
#include <fstream>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "NvInfer.h"

#include "constants.h"
#include "misc.h"
#include "pool.h"

static void checkCudaError(const cudaError_t status, const char* opName, const char* file, const char* func, int line) {
  if (status != cudaSuccess) {
    throw tinybot::StringError(std::string("CUDA Error, for ") + opName + " file " + file + ", func " + func + ", line " + tinybot::intToString(line) + ", error " + cudaGetErrorString(status));
  }
}
#define CUDA_ERR(opName, x) \
  { checkCudaError((x), opName, __FILE__, #x, __LINE__); }

namespace tinybot {

struct InferenceOutput {
  float output_policy[kBufferLengths[kOutputPolicy]];
  float output_land[kBufferLengths[kOutputLand]];
};

struct InferenceInput {
  float input_local[kBufferLengths[kInputLocal]];
  float input_global[kBufferLengths[kInputGlobal]];
  std::promise<std::shared_ptr<InferenceOutput>> promise{};
  std::shared_future<std::shared_ptr<InferenceOutput>> future{};
  int cursor{};
};

struct InferenceTicket {
  std::shared_future<std::shared_ptr<InferenceOutput>> future;
  int cursor;

  InferenceTicket(std::shared_future<std::shared_ptr<InferenceOutput>> f, const int c)
      : future(f), cursor(c){};
  explicit InferenceTicket(const int c)
      : cursor(c){};  // dummy future
};

struct InferenceContext {
  nvinfer1::IExecutionContext* exec;
  void* buffers[kNumTensors];
};

class InferenceManager {
 public:
  explicit InferenceManager(const char* trt_filename);
  InferenceTicket request(float* input_local, float* input_global);

 private:
  void infer(std::shared_ptr<InferenceInput> input, std::shared_ptr<InferenceOutput> result, InferenceContext* context);

  int binding_indices[kNumTensors]{};
  Pool<InferenceContext> context_pool{};
  std::mutex request_mutex{};
  std::shared_ptr<InferenceInput> current_input{};
};

}  // namespace tinybot

#endif  // TINYBOT_INFER_H
