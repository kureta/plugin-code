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
        void next_k(int nSamples);
        void infer();

        enum Inputs
        {
            Frequency,
            LoudnessDb,
        };
        // Member variables
        torch::jit::script::Module ctrl;

        // State variables
        at::Tensor f0;
        at::Tensor amp;
        at::Tensor hidden;
        at::Tensor harm_amp;
        at::Tensor overtones;
        at::Tensor noise;
    };

} // namespace Performer
