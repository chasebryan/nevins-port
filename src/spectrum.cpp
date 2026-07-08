#include "nevins/signal_card.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <vector>

namespace nevins {

namespace {

struct ComplexSample {
  double i;
  double q;
};

ComplexSample iq_sample(const std::vector<std::uint8_t>& iq, std::size_t sample_index) {
  const auto i_raw = static_cast<double>(iq[2U * sample_index]);
  const auto q_raw = static_cast<double>(iq[2U * sample_index + 1U]);
  return ComplexSample{
      (i_raw - 127.5) / 127.5,
      (q_raw - 127.5) / 127.5,
  };
}

double to_db(double linear) {
  return 10.0 * std::log10(std::max(linear, 1.0e-12));
}

}  // namespace

SpectrumSummary analyze_iq_power(const std::vector<std::uint8_t>& iq, const CaptureSettings& settings) {
  SpectrumSummary summary;
  summary.peak_frequency_hz = settings.center_frequency_hz;

  if (iq.size() < 2U) {
    return summary;
  }

  const std::size_t sample_count = iq.size() / 2U;
  const std::size_t window_samples = std::min<std::size_t>(sample_count, 4096U);
  const std::size_t bin_count = 128U;
  constexpr double kPi = 3.14159265358979323846;

  std::vector<double> bin_powers;
  bin_powers.reserve(bin_count);

  double total_power = 0.0;
  for (std::size_t bin = 0; bin < bin_count; ++bin) {
    const double normalized_frequency =
        -0.5 + (static_cast<double>(bin) + 0.5) / static_cast<double>(bin_count);
    double real = 0.0;
    double imag = 0.0;
    for (std::size_t n = 0; n < window_samples; ++n) {
      const auto sample = iq_sample(iq, n);
      const double window = 0.5 - 0.5 * std::cos((2.0 * kPi * static_cast<double>(n)) /
                                                  static_cast<double>(std::max<std::size_t>(1U, window_samples - 1U)));
      const double angle = -2.0 * kPi * normalized_frequency * static_cast<double>(n);
      const double cosine = std::cos(angle);
      const double sine = std::sin(angle);
      real += window * ((sample.i * cosine) - (sample.q * sine));
      imag += window * ((sample.i * sine) + (sample.q * cosine));
    }
    const double power = (real * real + imag * imag) / static_cast<double>(std::max<std::size_t>(1U, window_samples));
    bin_powers.push_back(power);
    total_power += power;
  }

  const auto minmax = std::minmax_element(bin_powers.begin(), bin_powers.end());
  const auto peak_it = minmax.second;
  const std::size_t peak_index = static_cast<std::size_t>(std::distance(bin_powers.begin(), peak_it));
  const double average = total_power / static_cast<double>(bin_powers.size());
  const double peak = *peak_it;

  std::vector<double> sorted_powers = bin_powers;
  std::sort(sorted_powers.begin(), sorted_powers.end());
  const double noise = sorted_powers[std::min<std::size_t>(sorted_powers.size() - 1U, sorted_powers.size() / 5U)];

  summary.average_power_db = to_db(average);
  summary.noise_floor_db = to_db(noise);
  summary.peak_power_db = to_db(peak);

  const double bin_hz = static_cast<double>(settings.sample_rate_hz) / static_cast<double>(bin_powers.size());
  const double offset_hz = (static_cast<double>(peak_index) + 0.5) * bin_hz -
                           (static_cast<double>(settings.sample_rate_hz) / 2.0);
  const auto peak_frequency =
      static_cast<std::int64_t>(settings.center_frequency_hz) + static_cast<std::int64_t>(std::llround(offset_hz));
  summary.peak_frequency_hz = peak_frequency > 0 ? static_cast<std::uint64_t>(peak_frequency) : settings.center_frequency_hz;

  const double threshold_db = summary.noise_floor_db + 3.0;
  std::uint32_t occupied_bins = 0;
  for (double power : bin_powers) {
    if (to_db(power) >= threshold_db) {
      ++occupied_bins;
    }
  }
  const auto estimated_bandwidth = static_cast<std::uint32_t>(
      std::clamp<double>(static_cast<double>(occupied_bins) * bin_hz, 1.0, static_cast<double>(settings.sample_rate_hz)));
  summary.bandwidth_estimate_hz = estimated_bandwidth;

  const std::size_t chunk_count = std::min<std::size_t>(32U, std::max<std::size_t>(1U, sample_count / 512U));
  const std::size_t chunk_width = std::max<std::size_t>(1U, sample_count / chunk_count);
  double min_chunk = std::numeric_limits<double>::max();
  double max_chunk = 0.0;
  for (std::size_t chunk = 0; chunk < chunk_count; ++chunk) {
    const std::size_t begin = chunk * chunk_width;
    const std::size_t end = std::min(sample_count, begin + chunk_width);
    double chunk_power = 0.0;
    for (std::size_t n = begin; n < end; ++n) {
      const auto sample = iq_sample(iq, n);
      chunk_power += (sample.i * sample.i) + (sample.q * sample.q);
    }
    const double mean = chunk_power / static_cast<double>(std::max<std::size_t>(1U, end - begin));
    min_chunk = std::min(min_chunk, mean);
    max_chunk = std::max(max_chunk, mean);
  }
  summary.burst_like = (summary.peak_power_db - summary.noise_floor_db) >= 8.0 ||
                       (to_db(max_chunk) - to_db(min_chunk)) >= 6.0;

  return summary;
}

}  // namespace nevins
