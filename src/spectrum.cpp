#include "nevins/signal_card.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numeric>
#include <vector>

namespace nevins {

namespace {

double iq_power(const std::vector<std::uint8_t>& iq, std::size_t sample_index) {
  const auto i_raw = static_cast<double>(iq[2U * sample_index]);
  const auto q_raw = static_cast<double>(iq[2U * sample_index + 1U]);
  const double i = (i_raw - 128.0) / 128.0;
  const double q = (q_raw - 128.0) / 128.0;
  return (i * i) + (q * q);
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
  const std::size_t bin_count = std::clamp<std::size_t>(sample_count / 512U, 8U, 128U);
  const std::size_t bin_width_samples = std::max<std::size_t>(1U, sample_count / bin_count);

  std::vector<double> bin_powers;
  bin_powers.reserve(bin_count);

  double total_power = 0.0;
  for (std::size_t bin = 0; bin < bin_count; ++bin) {
    const std::size_t begin = bin * bin_width_samples;
    const std::size_t end = std::min(sample_count, begin + bin_width_samples);
    double bin_total = 0.0;
    for (std::size_t n = begin; n < end; ++n) {
      bin_total += iq_power(iq, n);
    }
    const double mean = bin_total / static_cast<double>(std::max<std::size_t>(1U, end - begin));
    bin_powers.push_back(mean);
    total_power += mean;
  }

  const auto minmax = std::minmax_element(bin_powers.begin(), bin_powers.end());
  const auto peak_it = minmax.second;
  const std::size_t peak_index = static_cast<std::size_t>(std::distance(bin_powers.begin(), peak_it));
  const double average = total_power / static_cast<double>(bin_powers.size());
  const double noise = *minmax.first;
  const double peak = *peak_it;

  summary.average_power_db = to_db(average);
  summary.noise_floor_db = to_db(noise);
  summary.peak_power_db = to_db(peak);
  summary.burst_like = (summary.peak_power_db - summary.noise_floor_db) >= 8.0;

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

  return summary;
}

}  // namespace nevins
