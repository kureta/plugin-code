// PluginRampUpGen.cpp
// Mads Kjeldgaard (mail@madskjeldgaard.dk)

#include "RampUpGen.hpp"
#include <torch/script.h>
#include "SC_PlugIn.hpp"

torch::jit::script::Module ctrl;

auto options =
    torch::TensorOptions()
        .dtype(torch::kFloat32)
        .device(torch::kCUDA, 0)
        .requires_grad(false);

at::Tensor f0 = torch::randn({1, 1, 1}, options);
at::Tensor amp = torch::randn({1, 1, 1}, options);
at::Tensor hidden = torch::randn({3, 1, 512}, options);
at::Tensor harm_amp = torch::randn({1, 1, 1}, options);
at::Tensor overtones = torch::randn({1, 1, 180, 1}, options);
at::Tensor noise = torch::randn({1, 1, 195, 1}, options);

static InterfaceTable *ft;
void infer(at::Tensor &f0, at::Tensor &amp, at::Tensor &hidden, at::Tensor &harm_amp, at::Tensor &overtones, at::Tensor &noise)
{
  c10::InferenceMode inference;
  at::IValue output = ctrl.get_method("forward_live")({f0, amp, hidden}).toIValue();

  harm_amp = output.toTuple()->elements()[0].toTuple()->elements()[1].toTensor();
  overtones = output.toTuple()->elements()[0].toTuple()->elements()[2].toTensor();
  noise = output.toTuple()->elements()[1].toTensor();
  hidden = output.toTuple()->elements()[2].toTensor();
}
namespace RampUpGen
{

  RampUpGen::RampUpGen()
  {
    try
    {
      ctrl = torch::jit::load("/home/kureta/Documents/repos/performer/notebooks/cello_controller.pt");
    }
    catch (const c10::Error &e)
    {
    }

    infer(f0, amp, hidden, harm_amp, overtones, noise);
    // Set the UGen's calculation function depending on the rate of the first
    // argument (frequency)
    if (inRate(Frequency) == calc_FullRate)
    {
      mCalcFunc = make_calc_function<RampUpGen, &RampUpGen::next_a>();

      // Calculate first value
      next_a(1);
    }
    else
    {
      mCalcFunc = make_calc_function<RampUpGen, &RampUpGen::next_k>();

      // Calculate first value
      next_k(1);
    };
  }

  // The guts of our ramp generator
  inline float RampUpGen::progressPhasor(float frequency)
  {
    // Calculate increment value.
    // Double precision is important in phase values
    // because division errors are accumulated as well
    double increment = static_cast<double>(frequency) / sampleRate();

    m_phase += increment;

    const double minvalue = 0.0;
    const double maxvalue = 1.0;

    // Wrap the phasor if it goes beyond the boundaries
    if (m_phase > maxvalue)
    {
      m_phase = minvalue + (m_phase - maxvalue);
    }
    else if (m_phase < minvalue)
    {
      // in case phase is below minimum value
      m_phase = maxvalue - std::fabs(m_phase);
    }

    return m_phase;
  }

  // Calculation function for audio rate frequency input
  void RampUpGen::next_a(int nSamples)
  {
    const float *frequency = in(Frequency);
    float *outbuf = out(0);

    for (int i = 0; i < nSamples; ++i)
    {
      outbuf[i] = progressPhasor(frequency[i]);
    }
  }

  // Calculation function for control rate frequency input
  void RampUpGen::next_k(int nSamples)
  {
    infer(f0, amp, hidden, harm_amp, overtones, noise);
    const float frequencyParam = in(Frequency)[0];
    SlopeSignal<float> slopedFrequency =
        makeSlope(frequencyParam, m_frequency_past);
    float *outbuf = out(0);

    for (int i = 0; i < nSamples; ++i)
    {
      const float freq = slopedFrequency.consume();

      outbuf[i] = progressPhasor(freq);
    }

    // Store final value of frequency slope to be used next time the calculation
    // function runs
    m_frequency_past = slopedFrequency.value;
  }
} // namespace RampUpGen

PluginLoad(RampUpGenUGens)
{
  // Plugin magic
  ft = inTable;
  registerUnit<RampUpGen::RampUpGen>(ft, "RampUpGen", false);
}
