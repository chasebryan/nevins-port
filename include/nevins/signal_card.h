#pragma once

#include "nevins/core_api.h"
#include "nevins/device.h"

#include <cstdint>
#include <string>
#include <vector>

namespace nevins {

enum class ModulationClass {
  WideFmLike,
  NarrowbandLike,
  OokAskBurstLike,
  FskLike,
  AdsbPulseLike,
  SatellitePassLike,
  Unknown,
  RestrictedPrivatePossible
};

struct SpectrumSummary {
  double average_power_db = -120.0;
  double noise_floor_db = -120.0;
  double peak_power_db = -120.0;
  std::uint64_t peak_frequency_hz = 0;
  std::uint32_t bandwidth_estimate_hz = 1;
  bool burst_like = false;
};

struct SignalObservation {
  std::uint64_t center_frequency_hz = 433920000;
  std::uint32_t sample_rate_hz = 2048000;
  std::uint32_t duration_ms = 5000;
  SpectrumSummary spectrum;
};

struct SignalCard {
  std::uint64_t frequency_hz = 0;
  std::uint32_t bandwidth_estimate_hz = 1;
  double power_estimate_db = -120.0;
  double noise_floor_db = -120.0;
  std::uint32_t duration_ms = 0;
  ModulationClass modulation = ModulationClass::Unknown;
  std::string type_guess;
  int confidence = 0;
  NvSignalCategory category = NV_SIGNAL_UNKNOWN;
  std::vector<std::string> why;
  std::vector<std::string> safe_next_steps;
  std::vector<std::string> do_not_assume;
  std::string policy;
};

SpectrumSummary analyze_iq_power(const std::vector<std::uint8_t>& iq, const CaptureSettings& settings);
SignalCard classify_signal(const SignalObservation& observation);
SignalCard identify_frequency(std::uint64_t frequency_hz, std::uint32_t duration_ms = 5000);

std::string format_frequency_mhz(std::uint64_t frequency_hz);
std::string modulation_class_id(ModulationClass modulation);
std::string format_signal_card(const SignalCard& card);
std::string format_signal_card_markdown(const SignalCard& card);

}  // namespace nevins
