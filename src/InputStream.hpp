#pragma once
#include <deque>
#include <mutex>
#include <variant>
#include "rtaudio/RtAudio.h"

namespace Audio {

struct UseDefaultDevice {};
struct UseGivenDevice {
    std::string name;
};
using SelectedDevice = std::variant<UseDefaultDevice, UseGivenDevice>;

class InputStream {
public:
    explicit InputStream(RtAudioErrorCallback);
    ~InputStream();
    InputStream(InputStream const&)                        = delete; //
    auto operator=(InputStream const&) -> InputStream&     = delete; // Can't copy nor move
    InputStream(InputStream&&) noexcept                    = delete; // because we pass the address of this object to the audio callback.
    auto operator=(InputStream&&) noexcept -> InputStream& = delete; //

    /// Must be called every frame.
    void update();

    /// Calls the callback for each of the `samples_count` latest samples received through the device.
    /// This data is always mono-channel, 1 sample == 1 frame.
    void for_each_sample(int64_t samples_count, std::function<void(float)> const& callback);
    /// You MUST call this function at least once at the beginning to tell us the maximum numbers of samples you will query with `for_each_sample`.
    /// If that max number changes over time, you can call this function again to update it.
    void set_nb_of_retained_samples(size_t samples_count);

    /// Returns the list of all the ids of input devices.
    auto device_ids() const -> std::vector<unsigned int>;
    ///
    auto default_device_id() const -> unsigned int;
    /// Returns all the info about a given device.
    auto device_info(unsigned int device_id) const -> RtAudio::DeviceInfo;
    /// Returns 0 if the device is not found.
    auto find_device_id_by_name(std::string const& name) const -> unsigned int;
    auto find_device_info_by_name(std::string const& name) const -> RtAudio::DeviceInfo;
    ///
    auto current_device() const -> SelectedDevice const& { return _selected_device; }
    /// Returns the sample rate of the currently used device.
    auto sample_rate() const -> unsigned int { return _current_input_device_sample_rate; }
    /// Sets the device to use.
    /// By default, when an InputStream is created it uses the default input device selected by the OS.
    void use_given_device(RtAudio::DeviceInfo const& info);
    /// Sets the device to use.
    /// By default, when an InputStream is created it uses the default input device selected by the OS.
    void use_default_device();
    /// Sets the device to use.
    /// By default, when an InputStream is created it uses the default input device selected by the OS.
    void use_device(SelectedDevice);
    ///
    auto current_device_is_valid() const -> bool;
    /// Closes the current stream, disconnects from the current device.
    /// Does nothing if the stream was not open / no device was set.
    void close();

private:
    friend auto audio_input_callback(void* output_buffer, void* input_buffer, unsigned int frames_count, double stream_time, RtAudioStreamStatus status, void* user_data) -> int;

    void open_device(RtAudio::DeviceInfo const& info);
    void open_selected_device();
    /// /!\ YOU MUST LOCK `_samples_mutex` before using this function
    void shrink_samples_to_fit();

private:
    std::deque<float> _samples{};
    size_t            _nb_of_retained_samples{256};
    std::mutex        _samples_mutex{};

    mutable RtAudio _backend{};
    SelectedDevice  _selected_device{UseDefaultDevice{}};
    unsigned int    _current_input_device_sample_rate{};
    unsigned int    _current_device_id{};
};

} // namespace Audio