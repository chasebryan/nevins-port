#include "nevins/signal_card.h"

#include "nevins/core_api.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace nevins {

namespace {

bool in_range(std::uint64_t value, std::uint64_t begin, std::uint64_t end) {
  return value >= begin && value <= end;
}

SignalCard base_card(const SignalObservation& observation) {
  SignalCard card;
  card.frequency_hz = observation.center_frequency_hz;
  card.bandwidth_estimate_hz = std::max<std::uint32_t>(1U, observation.spectrum.bandwidth_estimate_hz);
  card.power_estimate_db = observation.spectrum.peak_power_db;
  card.noise_floor_db = observation.spectrum.noise_floor_db;
  card.duration_ms = observation.duration_ms;
  card.why.push_back("No content decoding attempted");
  card.safe_next_steps.push_back("Save IQ capture");
  card.safe_next_steps.push_back("Replay capture locally");
  card.do_not_assume.push_back("Do not infer device owner or private content");
  card.policy = "receive-only analysis; content decoding not attempted";
  return card;
}

std::string category_display(NvSignalCategory category) {
  if (category == NV_SIGNAL_RESTRICTED_PRIVATE) {
    return "Restricted/private";
  }
  return nv_signal_category_name(category);
}

}  // namespace

SignalCard classify_signal(const SignalObservation& observation) {
  SignalCard card = base_card(observation);
  const auto frequency = observation.center_frequency_hz;

  if (in_range(frequency, 88000000, 108000000)) {
    card.modulation = ModulationClass::WideFmLike;
    card.type_guess = "Wide FM-like public broadcast signal";
    card.bandwidth_estimate_hz = 180000;
    card.confidence = 72;
    card.category = NV_SIGNAL_PUBLIC_BROADCAST;
    card.why.insert(card.why.begin(), "Wide occupied bandwidth in the FM broadcast range");
    card.why.insert(card.why.begin() + 1, "Continuous high-power carrier shape");
    card.safe_next_steps.push_back("Compare spectrum shape against public broadcast examples");
    card.do_not_assume.push_back("Do not assume program content from spectrum shape alone");
  } else if (in_range(frequency, 162400000, 162550000)) {
    card.modulation = ModulationClass::NarrowbandLike;
    card.type_guess = "Narrowband weather-radio-like public signal";
    card.bandwidth_estimate_hz = 12500;
    card.confidence = 74;
    card.category = NV_SIGNAL_WEATHER;
    card.why.insert(card.why.begin(), "Frequency is within the common weather radio allocation");
    card.why.insert(card.why.begin() + 1, "Narrow occupied bandwidth estimate");
    card.safe_next_steps.push_back("Compare against lawful local weather-radio references");
  } else if (in_range(frequency, 433050000, 434790000) || in_range(frequency, 902000000, 928000000)) {
    card.modulation = ModulationClass::OokAskBurstLike;
    card.type_guess = "Bursty OOK/ASK-like ISM signal";
    card.bandwidth_estimate_hz = 25000;
    card.confidence = 68;
    card.category = NV_SIGNAL_ISM;
    card.why.insert(card.why.begin(), "Short repeated burst pattern");
    card.why.insert(card.why.begin() + 1, "Narrow occupied bandwidth");
    card.why.insert(card.why.begin() + 2, "Common ISM-band frequency range");
    card.safe_next_steps.push_back("Compare timing profile against known public examples");
  } else if (in_range(frequency, 144000000, 148000000) || in_range(frequency, 420000000, 450000000)) {
    card.modulation = ModulationClass::NarrowbandLike;
    card.type_guess = "Narrowband amateur-band-like signal";
    card.bandwidth_estimate_hz = 12500;
    card.confidence = 58;
    card.category = NV_SIGNAL_AMATEUR;
    card.why.insert(card.why.begin(), "Frequency is inside a common amateur allocation");
    card.why.insert(card.why.begin() + 1, "Narrowband signal shape");
    card.safe_next_steps.push_back("Check public band plans and local repeater directories");
  } else if (in_range(frequency, 1089000000, 1091000000)) {
    card.modulation = ModulationClass::AdsbPulseLike;
    card.type_guess = "Public aviation pulse-like signal";
    card.bandwidth_estimate_hz = 1000000;
    card.confidence = 64;
    card.category = NV_SIGNAL_AVIATION_PUBLIC;
    card.why.insert(card.why.begin(), "Pulse-like activity near a public aviation frequency");
    card.why.insert(card.why.begin() + 1, "Short high-energy bursts");
    card.safe_next_steps.push_back("Use only lawful public aviation references");
  } else if (in_range(frequency, 137000000, 138000000)) {
    card.modulation = ModulationClass::SatellitePassLike;
    card.type_guess = "Satellite-pass-like public signal";
    card.bandwidth_estimate_hz = 34000;
    card.confidence = 55;
    card.category = NV_SIGNAL_SATELLITE;
    card.why.insert(card.why.begin(), "Frequency range is used by some public satellite downlinks");
    card.why.insert(card.why.begin() + 1, "Pass-like interpretation remains tentative");
    card.safe_next_steps.push_back("Compare against published satellite pass predictions");
  } else if (in_range(frequency, 700000000, 900000000)) {
    card.modulation = ModulationClass::RestrictedPrivatePossible;
    card.type_guess = "Restricted/private signal possible";
    card.bandwidth_estimate_hz = std::max<std::uint32_t>(card.bandwidth_estimate_hz, 12500);
    card.confidence = 35;
    card.category = NV_SIGNAL_RESTRICTED_PRIVATE;
    card.why.insert(card.why.begin(), "Frequency range may include restricted or private services");
    card.why.insert(card.why.begin() + 1, "The system cannot safely classify this as public content");
    card.safe_next_steps.clear();
    card.safe_next_steps.push_back("Keep analysis to signal shape and metadata");
    card.safe_next_steps.push_back("Do not route to content decoding");
    card.do_not_assume.push_back("Do not claim message content, device identity, or user identity");
    card.policy = "restricted/private possible; explain-only receive analysis";
  } else {
    card.modulation = ModulationClass::Unknown;
    card.type_guess = "Unknown narrowband signal";
    card.confidence = 32;
    card.category = NV_SIGNAL_UNKNOWN;
    card.why.insert(card.why.begin(), "Signal shape does not match a supported public class with enough confidence");
    card.why.insert(card.why.begin() + 1, "Conservative unknown classification");
    card.safe_next_steps.clear();
    card.safe_next_steps.push_back("Record IQ only if lawful in your location");
    card.safe_next_steps.push_back("Replay locally for signal-shape comparison");
    card.safe_next_steps.push_back("Do not route to content decoding");
    card.policy = "unknown signal; conservative receive-only explanation";
  }

  card.confidence = std::clamp(card.confidence, 0, 100);
  if (!nv_policy_allows_action(card.category, NV_ACTION_DECODE_CONTENT)) {
    card.policy += "; decoding is not allowed by policy";
  }
  return card;
}

SignalCard identify_frequency(std::uint64_t frequency_hz, std::uint32_t duration_ms) {
  CaptureSettings settings;
  settings.center_frequency_hz = frequency_hz;
  settings.duration_ms = duration_ms;
  const auto iq = generate_mock_iq(settings);
  SignalObservation observation;
  observation.center_frequency_hz = frequency_hz;
  observation.sample_rate_hz = settings.sample_rate_hz;
  observation.duration_ms = duration_ms;
  observation.spectrum = analyze_iq_power(iq, settings);
  return classify_signal(observation);
}

std::string format_frequency_mhz(std::uint64_t frequency_hz) {
  std::ostringstream output;
  output << std::fixed << std::setprecision(3) << (static_cast<double>(frequency_hz) / 1000000.0) << " MHz";
  return output.str();
}

std::string modulation_class_id(ModulationClass modulation) {
  switch (modulation) {
    case ModulationClass::WideFmLike:
      return "wide_fm_like";
    case ModulationClass::NarrowbandLike:
      return "narrowband_like";
    case ModulationClass::OokAskBurstLike:
      return "ook_ask_burst_like";
    case ModulationClass::FskLike:
      return "fsk_like";
    case ModulationClass::AdsbPulseLike:
      return "adsb_pulse_like";
    case ModulationClass::SatellitePassLike:
      return "satellite_pass_like";
    case ModulationClass::RestrictedPrivatePossible:
      return "restricted_private_possible";
    case ModulationClass::Unknown:
      return "unknown";
  }
  return "unknown";
}

std::string format_signal_card(const SignalCard& card) {
  std::ostringstream output;
  output << "SIGNAL CARD\n\n";
  output << "Frequency: " << format_frequency_mhz(card.frequency_hz) << '\n';
  output << "Bandwidth estimate: " << card.bandwidth_estimate_hz << " Hz\n";
  output << "Power estimate: " << std::fixed << std::setprecision(1) << card.power_estimate_db << " dB\n";
  output << "Noise floor estimate: " << std::fixed << std::setprecision(1) << card.noise_floor_db << " dB\n";
  output << "Analysis window: " << card.duration_ms << " ms\n";
  output << "Type guess: " << card.type_guess << '\n';
  output << "Modulation class: " << modulation_class_id(card.modulation) << '\n';
  output << "Confidence: " << card.confidence << "%\n\n";

  output << "Why:\n\n";
  for (const auto& line : card.why) {
    output << "* " << line << '\n';
  }

  output << "\nSafe next step:\n\n";
  for (const auto& line : card.safe_next_steps) {
    output << "* " << line << '\n';
  }

  output << "\nWhat not to assume:\n\n";
  for (const auto& line : card.do_not_assume) {
    output << "* " << line << '\n';
  }

  output << "\nCategory: " << category_display(card.category) << '\n';
  output << "Policy: " << card.policy << '\n';
  return output.str();
}

std::string format_signal_card_markdown(const SignalCard& card) {
  return format_signal_card(card);
}

}  // namespace nevins
