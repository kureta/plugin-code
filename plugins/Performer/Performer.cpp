#include "Performer.hpp"
#include <torch/script.h>

static InterfaceTable *ft;
namespace Performer

{

  Performer::Performer()
  {
    // Load pre-trained model
    Print("Loading pre-trained model...");
    try
    {
      ctrl = torch::jit::load("/home/kureta/Documents/repos/performer/notebooks/cello_controller-cpu.pt");
    }
    catch (const c10::Error &e)
    {
      Print("Failed to load model.");
    }

    // Initialize tensors. CPU works fine. Will try CUDA later.
    auto options =
        torch::TensorOptions()
            .dtype(torch::kFloat32)
            // .device(torch::kCUDA, 0)
            .requires_grad(false);

    f0 = torch::randn({1, 1, 1}, options);
    amp = torch::randn({1, 1, 1}, options);
    hidden = torch::randn({3, 1, 512}, options);
    harm_amp = torch::randn({1, 1, 1}, options);
    overtones = torch::randn({1, 1, 180, 1}, options);
    noise = torch::randn({1, 1, 195, 1}, options);

    // First run to warm things up
    infer();

    // Set the UGen's calculation function depending on the rate of the first
    // argument (frequency)
    if (inRate(Frequency) == calc_FullRate || inRate(LoudnessDb) == calc_FullRate)
    {
      throw std::invalid_argument("Only works at control rate!");
    }
    else
    {
      mCalcFunc = make_calc_function<Performer, &Performer::next_k>();
      // Calculate first value
      next_k(1);
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
  void Performer::next_k(int nSamples)
  {
    const float *frequency = in(Frequency);
    const float *loudnessDb = in(LoudnessDb);
    float *outbuf_harm_amp = out(0);

    for (int i = 0; i < nSamples; ++i)
    {
      f0.fill_(frequency[i]);
      amp.fill_(loudnessDb[i]);
      infer();
      outbuf_harm_amp[i] = harm_amp[0][0][0].item<float>();
    }
  }
} // namespace Performer

PluginLoad(PerformerUGens)
{
  // Plugin magic
  ft = inTable;
  registerUnit<Performer::Performer>(ft, "Performer", false);
}
