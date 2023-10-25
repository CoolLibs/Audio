#include <exe_path/exe_path.h>
#include <Audio/Audio.hpp>
#include <algorithm>
#include <complex>
#include <iterator>
#include <quick_imgui/quick_imgui.hpp>
#include "imgui.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

// Learn how to use Dear ImGui: https://coollibs.github.io/contribute/Programming/dear-imgui

auto main(int argc, char* argv[]) -> int
{
    const int  exit_code              = doctest::Context{}.run(); // Run all unit tests
    const bool should_run_imgui_tests = argc < 2 || strcmp(argv[1], "-nogpu") != 0;
    if (
        should_run_imgui_tests
        && exit_code == 0 // Only open the window if the tests passed; this makes it easier to notice when some tests fail
    )
    {
        // Load the audio file
        Cool::load_audio_file(RtAudioW::player(), exe_path::dir() / "../tests/res/Monteverdi - L'Orfeo, Toccata.mp3");
        RtAudioW::player().play();

        static constexpr size_t          N = 512;   // input size NB: Must be a power of 2 for dj::fft1d
        std::vector<std::complex<float>> myData(N); // input data

        quick_imgui::loop("Audio tests", [&]() { // Open a window and run all the ImGui-related code
            for (size_t i = 0; i < N; i++)
                myData[i] = RtAudioW::player().sample_unaltered_volume(i + RtAudioW::player().current_frame_index(), 0);
            auto const fftData = dj::fft1d(myData, dj::fft_dir::DIR_FWD);
            auto       data    = std::vector<float>{};
            std::transform(fftData.begin(), fftData.end(), std::back_inserter(data), [](auto const x) {
                return std::abs(x);
            });

            ImGui::Begin("Audio tests");
            ImGui::PlotHistogram(
                "Spectrum",
                data.data(),
                static_cast<int>(data.size()) / 2, // The second half is a mirror of the first half, so ne need to display it.
                0, nullptr,
                0.f, 1.f, // Values are between 0 and 1
                {0.f, 100.f}
            );
            ImGui::End();
            ImGui::ShowDemoWindow();
        });
    }
    RtAudioW::shut_down();
    return exit_code;
}

TEST_CASE("Loading a .wav file")
{
    Cool::load_audio_file(RtAudioW::player(), exe_path::dir() / "../tests/res/10-1000-10000-20000.wav");

    CHECK(RtAudioW::player().audio_data().channels_count == 1);
    CHECK(RtAudioW::player().audio_data().sample_rate == 41000);
    CHECK(RtAudioW::player().audio_data().samples.size() == 164000);
}

TEST_CASE("Loading a .mp3 file")
{
    Cool::load_audio_file(RtAudioW::player(), exe_path::dir() / "../tests/res/Monteverdi - L'Orfeo, Toccata.mp3");

    CHECK(RtAudioW::player().audio_data().channels_count == 2);
    CHECK(RtAudioW::player().audio_data().sample_rate == 44100);
    CHECK(RtAudioW::player().audio_data().samples.size() == 9819648);
}

TEST_CASE("dj_fft test : Opening a .wav file, reading its content in a struct, computing the FFT on it")
{
    // Load the audio file
    Cool::load_audio_file(RtAudioW::player(), exe_path::dir() / "../tests/res/10-1000-10000-20000.wav");

    size_t                           N = 65536; // input size NB: Must be a power of 2
    std::vector<std::complex<float>> myData;    // input data

    // Prepare data, using a loop because the source vector is of different size
    for (size_t i = 0; i < N; i++)
        myData.emplace_back(RtAudioW::player().sample_unaltered_volume(i, 0));

    auto const fftData = dj::fft1d(myData, dj::fft_dir::DIR_FWD);

    CHECK(fftData.size() == N);
    CHECK(std::abs(fftData[16]) == doctest::Approx(38.669884));
    CHECK(std::abs(fftData[1598]) == doctest::Approx(27.571739));
    CHECK(std::abs(fftData[1599]) == doctest::Approx(21.486385));
    CHECK(std::abs(fftData[15984]) == doctest::Approx(29.728823));
    CHECK(std::abs(fftData[15985]) == doctest::Approx(18.963114));
    CHECK(std::abs(fftData[31968]) == doctest::Approx(10.106586));
    CHECK(std::abs(fftData[31969]) == doctest::Approx(35.716843));
    CHECK(std::abs(fftData[33567]) == doctest::Approx(35.765961));
    CHECK(std::abs(fftData[33568]) == doctest::Approx(10.012813));
    CHECK(std::abs(fftData[49551]) == doctest::Approx(19.058596));
    CHECK(std::abs(fftData[49552]) == doctest::Approx(29.651283));
    CHECK(std::abs(fftData[63937]) == doctest::Approx(21.579424));
    CHECK(std::abs(fftData[63938]) == doctest::Approx(27.487740));
    CHECK(std::abs(fftData[65520]) == doctest::Approx(38.676113));
}