#include "infer.h"

namespace tinybot {

class Logger : public nvinfer1::ILogger {
  void log(Severity severity, const char* msg) noexcept override {
    if (severity <= nvinfer1::ILogger::Severity::kWARNING) {
      std::cout << msg << "\n";
    }
  }
};

InferenceManager::InferenceManager(const char* trt_filename) {
  std::vector<char> trt_blob;
  std::ifstream file(trt_filename, std::ios::binary);
  if (file.good()) {
    file.seekg(0, std::ifstream::end);
    auto size = file.tellg();
    file.seekg(0, std::ifstream::beg);
    trt_blob.resize(size);
    file.read(trt_blob.data(), size);
    file.close();
  }

  nvinfer1::IRuntime* runtime = nvinfer1::createInferRuntime(*new Logger);
  nvinfer1::ICudaEngine* engine = runtime->deserializeCudaEngine(trt_blob.data(), trt_blob.size());

  for (int i = 0; i < kNumTensors; ++i) {
    binding_indices[i] = engine->getBindingIndex(kTensorNames[i]);
  }

  for (int j = 0; j < kConcurrentInferences; ++j) {
    auto context = new InferenceContext;
    context->exec = engine->createExecutionContext();
    for (int i = 0; i < kNumTensors; ++i) {
      nvinfer1::Dims d;
      switch (i) {
        case kInputLocal:
          d.d[0] = kBatchSize;
          d.d[1] = 21;
          d.d[2] = 9;
          d.d[3] = 9;
          d.nbDims = 4;
          context->exec->setBindingDimensions(binding_indices[i], d);
          break;
        case kInputGlobal:
          d.d[0] = kBatchSize;
          d.d[1] = kTensorLengths[i];
          d.nbDims = 2;
          context->exec->setBindingDimensions(binding_indices[i], d);
          break;
        default:
          break;
      }
      CUDA_ERR("InferenceManager", cudaMalloc(&context->buffers[binding_indices[i]], kBufferSizes[i]));
    }
    context_pool.push(context);
  }

  current_input = std::make_shared<InferenceInput>();
  current_input->future = current_input->promise.get_future().share();

  std::cout << binding_indices[0] << ' ' << binding_indices[1] << ' ' << binding_indices[2] << ' ' << binding_indices[3] << ' ' << binding_indices[4] << "\n";
  std::cout << kBufferSizes[0] << ' ' << kBufferSizes[1] << ' ' << kBufferSizes[2] << ' ' << kBufferSizes[3] << ' ' << kBufferSizes[4] << "\n";
}

InferenceTicket InferenceManager::request(float* input_local, float* input_global) {
  std::unique_lock<std::mutex> lock(request_mutex);
  float* dest_local = &current_input->input_local[kTensorLengths[kInputLocal] * current_input->cursor];
  float* dest_global = &current_input->input_global[kTensorLengths[kInputGlobal] * current_input->cursor];
  for (int c = 0; c < 21; ++c) {
    for (int z = 0; z < 81; ++z) {
      dest_local[c * 81 + z] = input_local[z * 21 + c];
    }
  }
  memcpy(dest_global, input_global, kTensorSizes[kInputGlobal]);
  InferenceTicket result(current_input->future, current_input->cursor);
  ++current_input->cursor;
  if (current_input->cursor == kBatchSize) {
    std::thread(&InferenceManager::infer, this, std::move(current_input)).detach();
    current_input = std::make_shared<InferenceInput>();
    current_input->future = current_input->promise.get_future().share();
  }
  return result;
}

void InferenceManager::infer(std::shared_ptr<InferenceInput> input) {
  InferenceContext* context = context_pool.pop();

  // Transfers from host memory to device memory are asynchronous with respect to the host
  CUDA_ERR(
      "infer",
      cudaMemcpyAsync(
          context->buffers[binding_indices[kInputLocal]],
          input->input_local,
          kBufferSizes[kInputLocal],
          cudaMemcpyHostToDevice));
  CUDA_ERR(
      "infer",
      cudaMemcpyAsync(
          context->buffers[binding_indices[kInputGlobal]],
          input->input_global,
          kBufferSizes[kInputGlobal],
          cudaMemcpyHostToDevice));

  context->exec->enqueueV2(context->buffers, cudaStreamPerThread, nullptr);

  std::shared_ptr<InferenceOutput> result = std::make_shared<InferenceOutput>();

  CUDA_ERR(
      "infer",
      cudaMemcpy(
          result->output_policy,
          context->buffers[binding_indices[kOutputPolicy]],
          kBufferSizes[kOutputPolicy],
          cudaMemcpyDeviceToHost));
  /*
    CUDA_ERR(
        "infer",
        cudaMemcpy(
            result->output_value,
            context->buffers[binding_indices[kOutputValue]],
            kBufferSizes[kOutputValue],
            cudaMemcpyDeviceToHost));
  */

  context_pool.push(context);

  input->promise.set_value(result);
}

}  // namespace tinybot
