#pragma once

#include <torch/torch.h>
#include "SC_PlugIn.hpp"

namespace Performer
{

    class Performer : public SCUnit
    {
    public:
        Performer();

    private:
        // Calc function
        void next(int nSamples);
        void infer();

        enum Inputs
        {
            Frequency,
            LoudnessDb
        };
        // Member variables
        torch::jit::script::Module ctrl;
        torch::jit::script::Module harmonics;
        torch::jit::script::Module noise_gen;

        // State variables
        at::Tensor f0;
        at::Tensor amp;
        at::Tensor hidden;
        at::Tensor harm_amp;
        at::Tensor overtones;
        at::Tensor noise;

        at::Tensor osc;

        float old_freq = 110.0;
        float old_amp = -90.0;

        int consumed = 192;
    };

} // namespace Performer
