#include "nevins/device.h"

#include "nevins/core_api.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>

namespace nevins {

namespace {

constexpr double kPi = 3.14159265358979323846;

std::uint8_t clamp_byte(double value) {
  const auto rounded = static_cast<int>(std::lround(value));
  return static_cast<std::uint8_t>(std::clamp(rounded, 0, 255));
}

}  // namespace

DeviceStatus MockSdrSource::status() const {
  return DeviceStatus{
      "Nevins deterministic mock SDR",
      "mock",
      true,
      true,
      true,
      24000000,
      1766000000,
      250000,
      3200000,
  };
}

std::vector<std::uint8_t> MockSdrSource::read_iq(const CaptureSettings& settings) {
  return generate_mock_iq(settings);
}

std::vector<std::uint8_t> generate_mock_iq(const CaptureSettings& settings) {
  if (!nv_validate_frequency_hz(settings.center_frequency_hz) ||
      !nv_validate_sample_rate_hz(settings.sample_rate_hz) ||
      !nv_validate_duration_ms(settings.duration_ms)) {
    throw std::invalid_argument("mock capture settings are outside verified policy bounds");
  }

  const std::uint64_t sample_count =
      (static_cast<std::uint64_t>(settings.sample_rate_hz) * settings.duration_ms) / 1000U;
  std::vector<std::uint8_t> iq;
  iq.resize(static_cast<std::size_t>(sample_count * 2U));

  const double normalized_tone = 0.03125;
  for (std::uint64_t n = 0; n < sample_count; ++n) {
    const bool burst_gate = ((n / 2048U) % 9U) < 2U;
    const double envelope = burst_gate ? 34.0 : 7.0;
    const double phase = 2.0 * kPi * normalized_tone * static_cast<double>(n);
    const double slow = std::sin(2.0 * kPi * static_cast<double>(n % 32768U) / 32768.0);
    const double shaped_noise = static_cast<double>((n * 37U + 19U) % 23U) - 11.0;

    iq[static_cast<std::size_t>(2U * n)] = clamp_byte(128.0 + envelope * std::sin(phase) + slow * 3.0 + shaped_noise);
    iq[static_cast<std::size_t>(2U * n + 1U)] =
        clamp_byte(128.0 + envelope * std::cos(phase) - slow * 2.0 - shaped_noise);
  }

  return iq;
}

}  // namespace nevins
