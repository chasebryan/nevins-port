#pragma once

#include "nevins/device.h"
#include "nevins/signal_card.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace nevins {

struct CaptureManifest {
  std::string version = "1";
  std::string name = "mock-survey";
  std::uint64_t center_frequency_hz = 433920000;
  std::uint32_t sample_rate_hz = 2048000;
  std::uint32_t duration_ms = 5000;
  int confidence = 0;
  NvSignalCategory category = NV_SIGNAL_UNKNOWN;
  ModulationClass modulation = ModulationClass::Unknown;
  std::string iq_file = "iq.u8";
  std::string spectrum_file = "spectrum.csv";
  std::string signal_card_file = "signal-card.md";
  std::string notes_file = "notes.txt";
};

struct CapturePack {
  std::filesystem::path root;
  CaptureManifest manifest;
  std::vector<std::uint8_t> iq;
  SignalCard card;
};

CaptureManifest make_manifest(const CaptureSettings& settings, const SignalCard& card, std::string name);
void write_capture_pack(const std::filesystem::path& output_path,
                        const CaptureManifest& manifest,
                        const std::vector<std::uint8_t>& iq,
                        const SignalCard& card,
                        const SpectrumSummary& spectrum);

CapturePack create_mock_capture(const std::filesystem::path& output_path,
                                std::uint32_t duration_seconds,
                                std::uint64_t center_frequency_hz = 433920000);

CaptureManifest read_manifest(const std::filesystem::path& capture_path);
CapturePack load_capture_pack(const std::filesystem::path& capture_path);
std::string explain_capture_pack(const std::filesystem::path& capture_path);
std::string replay_capture_pack(const std::filesystem::path& capture_path);

}  // namespace nevins
