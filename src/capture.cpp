#include "nevins/capture.h"

#include "nevins/core_api.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace nevins {

namespace {

ModulationClass modulation_from_id(const std::string& value) {
  if (value == "wide_fm_like") {
    return ModulationClass::WideFmLike;
  }
  if (value == "narrowband_like") {
    return ModulationClass::NarrowbandLike;
  }
  if (value == "ook_ask_burst_like") {
    return ModulationClass::OokAskBurstLike;
  }
  if (value == "fsk_like") {
    return ModulationClass::FskLike;
  }
  if (value == "adsb_pulse_like") {
    return ModulationClass::AdsbPulseLike;
  }
  if (value == "satellite_pass_like") {
    return ModulationClass::SatellitePassLike;
  }
  if (value == "restricted_private_possible") {
    return ModulationClass::RestrictedPrivatePossible;
  }
  return ModulationClass::Unknown;
}

std::uint64_t parse_u64(const std::unordered_map<std::string, std::string>& fields, const std::string& key) {
  const auto it = fields.find(key);
  if (it == fields.end()) {
    throw std::runtime_error("manifest missing required field: " + key);
  }
  std::size_t consumed = 0;
  const auto value = std::stoull(it->second, &consumed, 10);
  if (consumed != it->second.size()) {
    throw std::runtime_error("manifest field is not an unsigned integer: " + key);
  }
  return value;
}

std::uint32_t parse_u32(const std::unordered_map<std::string, std::string>& fields, const std::string& key) {
  const auto value = parse_u64(fields, key);
  if (value > UINT32_MAX) {
    throw std::runtime_error("manifest field is too large: " + key);
  }
  return static_cast<std::uint32_t>(value);
}

int parse_int(const std::unordered_map<std::string, std::string>& fields, const std::string& key) {
  const auto it = fields.find(key);
  if (it == fields.end()) {
    throw std::runtime_error("manifest missing required field: " + key);
  }
  std::size_t consumed = 0;
  const auto value = std::stoi(it->second, &consumed, 10);
  if (consumed != it->second.size()) {
    throw std::runtime_error("manifest field is not an integer: " + key);
  }
  return value;
}

std::string field_or_default(const std::unordered_map<std::string, std::string>& fields,
                             const std::string& key,
                             const std::string& fallback) {
  const auto it = fields.find(key);
  return it == fields.end() ? fallback : it->second;
}

std::string read_text_file(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) {
    throw std::runtime_error("unable to open text file: " + path.string());
  }
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void write_text_file(const std::filesystem::path& path, const std::string& text) {
  std::ofstream output(path);
  if (!output) {
    throw std::runtime_error("unable to write text file: " + path.string());
  }
  output << text;
}

}  // namespace

CaptureManifest make_manifest(const CaptureSettings& settings, const SignalCard& card, std::string name) {
  if (!nv_validate_frequency_hz(settings.center_frequency_hz) ||
      !nv_validate_sample_rate_hz(settings.sample_rate_hz) ||
      !nv_validate_duration_ms(settings.duration_ms) ||
      !nv_validate_confidence(card.confidence)) {
    throw std::invalid_argument("capture manifest values are outside policy bounds");
  }

  CaptureManifest manifest;
  manifest.name = std::move(name);
  manifest.center_frequency_hz = settings.center_frequency_hz;
  manifest.sample_rate_hz = settings.sample_rate_hz;
  manifest.duration_ms = settings.duration_ms;
  manifest.confidence = card.confidence;
  manifest.category = card.category;
  manifest.modulation = card.modulation;
  return manifest;
}

void write_capture_pack(const std::filesystem::path& output_path,
                        const CaptureManifest& manifest,
                        const std::vector<std::uint8_t>& iq,
                        const SignalCard& card,
                        const SpectrumSummary& spectrum) {
  if (output_path.extension() != ".nvcap") {
    throw std::invalid_argument("capture output must use the .nvcap extension");
  }
  if (!nv_validate_confidence(manifest.confidence)) {
    throw std::invalid_argument("capture confidence is outside policy bounds");
  }

  std::filesystem::create_directories(output_path);

  std::ofstream manifest_file(output_path / "manifest.nv1");
  if (!manifest_file) {
    throw std::runtime_error("unable to write manifest.nv1");
  }
  manifest_file << "nv1.version=" << manifest.version << '\n';
  manifest_file << "name=" << manifest.name << '\n';
  manifest_file << "center_frequency_hz=" << manifest.center_frequency_hz << '\n';
  manifest_file << "sample_rate_hz=" << manifest.sample_rate_hz << '\n';
  manifest_file << "duration_ms=" << manifest.duration_ms << '\n';
  manifest_file << "confidence=" << manifest.confidence << '\n';
  manifest_file << "category=" << nv_signal_category_name(manifest.category) << '\n';
  manifest_file << "modulation_guess=" << modulation_class_id(manifest.modulation) << '\n';
  manifest_file << "iq_file=" << manifest.iq_file << '\n';
  manifest_file << "spectrum_file=" << manifest.spectrum_file << '\n';
  manifest_file << "signal_card_file=" << manifest.signal_card_file << '\n';
  manifest_file << "notes_file=" << manifest.notes_file << '\n';

  std::ofstream iq_file(output_path / manifest.iq_file, std::ios::binary);
  if (!iq_file) {
    throw std::runtime_error("unable to write iq.u8");
  }
  iq_file.write(reinterpret_cast<const char*>(iq.data()), static_cast<std::streamsize>(iq.size()));

  write_text_file(output_path / manifest.signal_card_file, format_signal_card_markdown(card));

  std::ostringstream spectrum_csv;
  spectrum_csv << "metric,value\n";
  spectrum_csv << "average_power_db," << spectrum.average_power_db << '\n';
  spectrum_csv << "noise_floor_db," << spectrum.noise_floor_db << '\n';
  spectrum_csv << "peak_power_db," << spectrum.peak_power_db << '\n';
  spectrum_csv << "peak_frequency_hz," << spectrum.peak_frequency_hz << '\n';
  spectrum_csv << "bandwidth_estimate_hz," << spectrum.bandwidth_estimate_hz << '\n';
  spectrum_csv << "burst_like," << (spectrum.burst_like ? 1 : 0) << '\n';
  write_text_file(output_path / manifest.spectrum_file, spectrum_csv.str());

  write_text_file(output_path / manifest.notes_file,
                  "Mock capture pack. Receive-only signal-shape analysis; no content decoding attempted.\n");
}

CapturePack create_mock_capture(const std::filesystem::path& output_path,
                                std::uint32_t duration_seconds,
                                std::uint64_t center_frequency_hz) {
  if (duration_seconds == 0U || duration_seconds > 3600U) {
    throw std::invalid_argument("mock survey duration must be between 1 and 3600 seconds");
  }

  CaptureSettings settings;
  settings.center_frequency_hz = center_frequency_hz;
  settings.duration_ms = duration_seconds * 1000U;

  MockSdrSource source;
  auto iq = source.read_iq(settings);
  auto spectrum = analyze_iq_power(iq, settings);

  SignalObservation observation;
  observation.center_frequency_hz = settings.center_frequency_hz;
  observation.sample_rate_hz = settings.sample_rate_hz;
  observation.duration_ms = settings.duration_ms;
  observation.spectrum = spectrum;

  auto card = classify_signal(observation);
  auto manifest = make_manifest(settings, card, "mock-survey");
  write_capture_pack(output_path, manifest, iq, card, spectrum);
  return CapturePack{output_path, manifest, std::move(iq), std::move(card)};
}

CaptureManifest read_manifest(const std::filesystem::path& capture_path) {
  std::ifstream input(capture_path / "manifest.nv1");
  if (!input) {
    throw std::runtime_error("unable to open manifest.nv1 in " + capture_path.string());
  }

  std::unordered_map<std::string, std::string> fields;
  std::string line;
  while (std::getline(input, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }
    const auto separator = line.find('=');
    if (separator == std::string::npos) {
      throw std::runtime_error("manifest line is not key=value: " + line);
    }
    fields.emplace(line.substr(0, separator), line.substr(separator + 1U));
  }

  CaptureManifest manifest;
  manifest.version = field_or_default(fields, "nv1.version", "");
  if (manifest.version != "1") {
    throw std::runtime_error("unsupported capture manifest version");
  }
  manifest.name = field_or_default(fields, "name", "unnamed");
  manifest.center_frequency_hz = parse_u64(fields, "center_frequency_hz");
  manifest.sample_rate_hz = parse_u32(fields, "sample_rate_hz");
  manifest.duration_ms = parse_u32(fields, "duration_ms");
  manifest.confidence = parse_int(fields, "confidence");
  manifest.category = nv_signal_category_from_name(field_or_default(fields, "category", "Unknown").c_str());
  manifest.modulation = modulation_from_id(field_or_default(fields, "modulation_guess", "unknown"));
  manifest.iq_file = field_or_default(fields, "iq_file", "iq.u8");
  manifest.spectrum_file = field_or_default(fields, "spectrum_file", "spectrum.csv");
  manifest.signal_card_file = field_or_default(fields, "signal_card_file", "signal-card.md");
  manifest.notes_file = field_or_default(fields, "notes_file", "notes.txt");

  if (!nv_validate_frequency_hz(manifest.center_frequency_hz) ||
      !nv_validate_sample_rate_hz(manifest.sample_rate_hz) ||
      !nv_validate_duration_ms(manifest.duration_ms) ||
      !nv_validate_confidence(manifest.confidence)) {
    throw std::runtime_error("capture manifest values are outside policy bounds");
  }

  return manifest;
}

CapturePack load_capture_pack(const std::filesystem::path& capture_path) {
  auto manifest = read_manifest(capture_path);
  std::ifstream input(capture_path / manifest.iq_file, std::ios::binary);
  if (!input) {
    throw std::runtime_error("unable to open IQ file in capture pack");
  }

  std::vector<std::uint8_t> iq((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  CaptureSettings settings;
  settings.center_frequency_hz = manifest.center_frequency_hz;
  settings.sample_rate_hz = manifest.sample_rate_hz;
  settings.duration_ms = manifest.duration_ms;

  SignalObservation observation;
  observation.center_frequency_hz = manifest.center_frequency_hz;
  observation.sample_rate_hz = manifest.sample_rate_hz;
  observation.duration_ms = manifest.duration_ms;
  observation.spectrum = analyze_iq_power(iq, settings);

  auto card = classify_signal(observation);
  return CapturePack{capture_path, manifest, std::move(iq), std::move(card)};
}

std::string explain_capture_pack(const std::filesystem::path& capture_path) {
  const auto manifest = read_manifest(capture_path);
  const auto card_path = capture_path / manifest.signal_card_file;
  if (std::filesystem::exists(card_path)) {
    return read_text_file(card_path);
  }

  CaptureSettings settings;
  settings.center_frequency_hz = manifest.center_frequency_hz;
  settings.sample_rate_hz = manifest.sample_rate_hz;
  settings.duration_ms = manifest.duration_ms;
  return format_signal_card(identify_frequency(settings.center_frequency_hz, settings.duration_ms));
}

std::string replay_capture_pack(const std::filesystem::path& capture_path) {
  auto pack = load_capture_pack(capture_path);
  std::ostringstream output;
  output << "REPLAY\n\n";
  output << "Capture: " << pack.root.string() << '\n';
  output << "IQ bytes: " << pack.iq.size() << "\n\n";
  output << format_signal_card(pack.card);
  return output.str();
}

}  // namespace nevins
