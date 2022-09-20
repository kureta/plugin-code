#include "Performer.hpp"
#include <torch/script.h>

static InterfaceTable *ft;
namespace Performer

{
  using namespace torch::indexing;
  Performer::Performer()
  {
    // Load pre-trained model
    Print("Loading pre-trained model...");
    try
    {
      c10::InferenceMode inference;
      ctrl = torch::jit::load("/home/kureta/Documents/repos/performer/out/cello-cuda-controller.pt");
      harmonics = torch::jit::load("/home/kureta/Documents/repos/performer/out/cello-cuda-harmonics.pt");
      noise_gen = torch::jit::load("/home/kureta/Documents/repos/performer/out/cello-cuda-noise.pt");
    }
    catch (const c10::Error &e)
    {
      Print("Failed to load model.");
    }

    // Initialize tensors. CPU works fine. Will try CUDA later.
    c10::InferenceMode inference;
    auto options =
        torch::TensorOptions()
            .dtype(torch::kFloat32)
            .device(torch::kCUDA, 0)
            .requires_grad(false);

    f0 = torch::randn({1, 1, 2}, options);
    f0.index_put_({0, 0, 0}, old_freq);
    amp = torch::randn({1, 1, 2}, options);
    amp.index_put_({0, 0, 0}, old_amp);
    hidden = torch::randn({3, 1, 512}, options);
    harm_amp = torch::randn({1, 1, 2}, options);
    overtones = torch::randn({1, 1, 180, 2}, options);
    noise = torch::randn({1, 1, 195, 2}, options);

    // Set the UGen's calculation function depending on the rate of the first
    // argument (frequency)
    if (inRate(Frequency) == calc_FullRate || inRate(LoudnessDb) == calc_FullRate)
    {
      throw std::invalid_argument("Only works at control rate!");
    }
    else
    {
      mCalcFunc = make_calc_function<Performer, &Performer::next>();
      // Calculate first value
      next(1);
    };
  }

  // Inference happens here
  void Performer::infer()
  {
    c10::InferenceMode inference;
    at::IValue output = ctrl.get_method("forward_live")({f0, amp, hidden}).toIValue();

    harm_amp = output.toTuple()->elements()[0].toTuple()->elements()[1].toTensor();
    overtones = output.toTuple()->elements()[0].toTuple()->elements()[2].toTensor();
    noise = output.toTuple()->elements()[1].toTensor();
    hidden = output.toTuple()->elements()[2].toTensor();
  }

  // Calculation function for control rate frequency input
  void Performer::next(int nSamples)
  {
    const float frequency = in0(Frequency);
    const float loudnessDb = in0(LoudnessDb);
    float *outbuf = out(0);

    for (int i = 0; i < nSamples; ++i)
    {
      if (consumed == 192)
      {
        consumed = 0;
        c10::InferenceMode inference;
        f0.index_put_({0, 0, 1}, frequency);
        amp.index_put_({0, 0, 1}, loudnessDb);

        infer();

        osc = harmonics.get_method("forward_live")({f0, harm_amp, overtones}).toTensor().to(torch::kCPU)[0][0] + noise_gen({noise}).toTensor().to(torch::kCPU)[0][0];

        f0 = f0.flip(-1);
        amp = amp.flip(-1);
      }
      outbuf[i] = osc[consumed].item<float>();
      consumed += 1;
    }
  }
} // namespace Performer

PluginLoad(PerformerUGens)
{
  // Plugin magic
  ft = inTable;
  registerUnit<Performer::Performer>(ft, "Performer", false);
}
