#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace nevins {

struct CaptureSettings {
  std::uint64_t center_frequency_hz = 433920000;
  std::uint32_t sample_rate_hz = 2048000;
  std::uint32_t duration_ms = 5000;
  std::int32_t gain_db_tenths = 0;
};

struct DeviceStatus {
  std::string name;
  std::string mode;
  bool is_mock = true;
  bool hardware_present = false;
  bool receive_only = true;
  std::uint64_t min_frequency_hz = 24000000;
  std::uint64_t max_frequency_hz = 1766000000;
  std::uint32_t min_sample_rate_hz = 250000;
  std::uint32_t max_sample_rate_hz = 3200000;
};

class ISdrSource {
 public:
  virtual ~ISdrSource() = default;

  virtual DeviceStatus status() const = 0;
  virtual std::vector<std::uint8_t> read_iq(const CaptureSettings& settings) = 0;
};

class MockSdrSource final : public ISdrSource {
 public:
  DeviceStatus status() const override;
  std::vector<std::uint8_t> read_iq(const CaptureSettings& settings) override;
};

class RtlSdrSource final : public ISdrSource {
 public:
  DeviceStatus status() const override;
  std::vector<std::uint8_t> read_iq(const CaptureSettings& settings) override;
};

std::vector<std::uint8_t> generate_mock_iq(const CaptureSettings& settings);

}  // namespace nevins
