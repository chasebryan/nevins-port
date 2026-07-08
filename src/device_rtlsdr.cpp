#include "nevins/device.h"

#include "nevins/core_api.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

#if NEVINS_ENABLE_RTLSDR
#include <rtl-sdr.h>
#endif

namespace nevins {

#if NEVINS_ENABLE_RTLSDR
namespace {

class RtlDeviceHandle {
 public:
  explicit RtlDeviceHandle(std::uint32_t index) {
    if (rtlsdr_open(&device_, index) != 0 || device_ == nullptr) {
      throw std::runtime_error("unable to open RTL-SDR device");
    }
  }

  RtlDeviceHandle(const RtlDeviceHandle&) = delete;
  RtlDeviceHandle& operator=(const RtlDeviceHandle&) = delete;

  ~RtlDeviceHandle() {
    if (device_ != nullptr) {
      rtlsdr_close(device_);
    }
  }

  rtlsdr_dev_t* get() const {
    return device_;
  }

 private:
  rtlsdr_dev_t* device_ = nullptr;
};

int nearest_supported_gain(rtlsdr_dev_t* device, std::int32_t requested_gain_db_tenths) {
  const int count = rtlsdr_get_tuner_gains(device, nullptr);
  if (count <= 0) {
    return requested_gain_db_tenths;
  }

  std::vector<int> gains(static_cast<std::size_t>(count));
  const int actual_count = rtlsdr_get_tuner_gains(device, gains.data());
  if (actual_count <= 0) {
    return requested_gain_db_tenths;
  }

  gains.resize(static_cast<std::size_t>(actual_count));
  return *std::min_element(gains.begin(), gains.end(), [requested_gain_db_tenths](int lhs, int rhs) {
    return std::abs(lhs - requested_gain_db_tenths) < std::abs(rhs - requested_gain_db_tenths);
  });
}

void require_rtlsdr_ok(int result, const std::string& operation) {
  if (result != 0) {
    throw std::runtime_error(operation + " failed with librtlsdr error " + std::to_string(result));
  }
}

}  // namespace
#endif

DeviceStatus RtlSdrSource::status() const {
#if NEVINS_ENABLE_RTLSDR
  const auto count = rtlsdr_get_device_count();
  std::string name = "RTL-SDR receiver";
  if (count > 0U) {
    name = rtlsdr_get_device_name(0);
  }
  return DeviceStatus{
      name,
      "rtlsdr",
      false,
      count > 0,
      true,
      24000000,
      1766000000,
      250000,
      3200000,
  };
#else
  return DeviceStatus{
      "RTL-SDR receiver support not compiled",
      "rtlsdr-disabled",
      false,
      false,
      true,
      24000000,
      1766000000,
      250000,
      3200000,
  };
#endif
}

std::vector<std::uint8_t> RtlSdrSource::read_iq(const CaptureSettings& settings) {
#if NEVINS_ENABLE_RTLSDR
  if (!nv_validate_frequency_hz(settings.center_frequency_hz) ||
      !nv_validate_sample_rate_hz(settings.sample_rate_hz) ||
      !nv_validate_duration_ms(settings.duration_ms) ||
      !nv_validate_gain_db_tenths(settings.gain_db_tenths)) {
    throw std::invalid_argument("RTL-SDR capture settings are outside verified policy bounds");
  }

  const auto device_count = rtlsdr_get_device_count();
  if (device_count == 0U) {
    throw std::runtime_error("no supported RTL-SDR device found");
  }

  RtlDeviceHandle handle(0);
  rtlsdr_dev_t* device = handle.get();

  require_rtlsdr_ok(rtlsdr_set_sample_rate(device, settings.sample_rate_hz), "set sample rate");
  require_rtlsdr_ok(rtlsdr_set_center_freq(device, static_cast<std::uint32_t>(settings.center_frequency_hz)),
                    "set center frequency");

  if (settings.gain_db_tenths > 0) {
    require_rtlsdr_ok(rtlsdr_set_tuner_gain_mode(device, 1), "enable manual gain");
    const int selected_gain = nearest_supported_gain(device, settings.gain_db_tenths);
    require_rtlsdr_ok(rtlsdr_set_tuner_gain(device, selected_gain), "set tuner gain");
  } else {
    require_rtlsdr_ok(rtlsdr_set_tuner_gain_mode(device, 0), "enable automatic gain");
  }

  require_rtlsdr_ok(rtlsdr_reset_buffer(device), "reset RTL-SDR buffer");

  const std::uint64_t sample_count =
      (static_cast<std::uint64_t>(settings.sample_rate_hz) * settings.duration_ms) / 1000U;
  const std::uint64_t byte_count = sample_count * 2U;
  if (byte_count > static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    throw std::runtime_error("requested RTL-SDR capture is too large for this platform");
  }

  std::vector<std::uint8_t> iq(static_cast<std::size_t>(byte_count));
  constexpr int kReadChunkBytes = 16 * 16384;
  std::size_t offset = 0;
  while (offset < iq.size()) {
    const auto remaining = static_cast<int>(std::min<std::size_t>(iq.size() - offset, kReadChunkBytes));
    int bytes_read = 0;
    const int result = rtlsdr_read_sync(device, iq.data() + offset, remaining, &bytes_read);
    if (result != 0) {
      throw std::runtime_error("RTL-SDR synchronous read failed with librtlsdr error " + std::to_string(result));
    }
    if (bytes_read <= 0) {
      throw std::runtime_error("RTL-SDR synchronous read returned no IQ data");
    }
    offset += static_cast<std::size_t>(bytes_read);
  }

  return iq;
#else
  (void)settings;
  throw std::runtime_error("Real RTL-SDR capture is scaffolded but not enabled for this build path.");
#endif
}

}  // namespace nevins
