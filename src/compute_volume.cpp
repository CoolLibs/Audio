#include "compute_volume.hpp"
#include <cstdint>
#include "RtAudioWrapper/RtAudioWrapper.hpp"

namespace Cool {

auto compute_volume(std::span<float const> data) -> float
{
    float sum_of_squares{0.f};
    for (float const sample : data)
        sum_of_squares += sample * sample;
    return std::sqrt(sum_of_squares / static_cast<float>(data.size())); // TODO(Audio) Return the result in decibels ?
}

} // namespace Cool