#include <ATen/InferSize.h>

#include <torch/extension.h>
#include "ORTOps.h"
#include "ORTUtil.h"

namespace at {
namespace native {
namespace ort {
namespace detail {

OrtValue reshape_copy(
  onnxruntime::ORTInvoker& invoker,
  const OrtValue& input,
  std::vector<int64_t> shape) {
  
  // TODO: actual reshape on buffer
  const onnxruntime::Tensor& input_tensor = input.Get<onnxruntime::Tensor>();
  auto new_shape = infer_size(shape, input_tensor.Shape().Size());
  OrtValue shape_tensor;
  //todo: avoid the copy on this small shape vector;
  CreateMLValue<int64_t>(invoker.GetCurrentExecutionProvider().GetAllocator(0, OrtMemTypeDefault),
                       {(int64_t)new_shape.size(),}, new_shape, &shape_tensor);
  std::vector<OrtValue> result(1);
  ORT_LOG << "Invoke ORT reshape kernel";
  auto status = invoker.Invoke("Reshape", {input, shape_tensor}, result, nullptr);
  if (!status.IsOK())
    throw std::runtime_error("ORT return failure status: " + status.ErrorMessage());
  ORT_LOG << "Invoke ORT reshape kernel successfully";
  return result[0];
}

OrtValue add(onnxruntime::ORTInvoker& invoker,
             const OrtValue& A,
             const OrtValue& B){
  std::vector<OrtValue> result(1);
  ORT_LOG << "Invoke ORT Add kernel";
  auto status = invoker.Invoke("Add", {A, B}, result, nullptr);
  if (!status.IsOK())
    throw std::runtime_error("ORT return failure status: " + status.ErrorMessage());
  ORT_LOG << "Invoke ORT Add kernel successfully";
  return result[0];
}

void copy(onnxruntime::ORTInvoker& invoker, 
          const OrtValue& src, OrtValue& dst){
  ORT_LOG << "Invoke ORT Copy ";
  auto& ort_ep = invoker.GetCurrentExecutionProvider();
  
  const auto& src_tensor = src.Get<onnxruntime::Tensor>();
  auto* dst_tensor = dst.GetMutable<onnxruntime::Tensor>();
  if (!dst_tensor)
    throw std::runtime_error("ORT copy: dst is not a tensor");
  ort_ep.GetDataTransfer()->CopyTensor(src_tensor, *dst_tensor);
}

} // namespace detail
} // namespace ort
} // namespace native
} // namespace at
