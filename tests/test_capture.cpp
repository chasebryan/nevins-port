#include "nevins/capture.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <string>

int main() {
  const auto suffix = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  const auto path = std::filesystem::temp_directory_path() / ("nevins_test_capture_" + suffix + ".nvcap");
  std::filesystem::remove_all(path);

  const auto pack = nevins::create_mock_capture(path, 1U);
  assert(std::filesystem::exists(path / "manifest.nv1"));
  assert(std::filesystem::exists(path / "iq.u8"));
  assert(std::filesystem::exists(path / "signal-card.md"));
  assert(std::filesystem::exists(path / "spectrum.csv"));
  assert(std::filesystem::exists(path / "notes.txt"));
  assert(!pack.iq.empty());
  assert(pack.manifest.center_frequency_hz == 433920000U);
  assert(pack.manifest.category == NV_SIGNAL_ISM);

  const auto manifest = nevins::read_manifest(path);
  assert(manifest.version == "1");
  assert(manifest.sample_rate_hz == 2048000U);
  assert(manifest.duration_ms == 1000U);
  assert(manifest.confidence >= 0);
  assert(manifest.confidence <= 100);

  const auto explanation = nevins::explain_capture_pack(path);
  assert(explanation.find("SIGNAL CARD") != std::string::npos);
  assert(explanation.find("Routing: shape-only analysis route") != std::string::npos);

  const auto replay = nevins::replay_capture_pack(path);
  assert(replay.find("REPLAY") != std::string::npos);
  assert(replay.find("SIGNAL CARD") != std::string::npos);
  assert(replay.find("Category: ISM") != std::string::npos);

  std::filesystem::remove_all(path);
  return 0;
}
