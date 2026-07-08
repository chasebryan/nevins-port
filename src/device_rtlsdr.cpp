#include "nevins/device.h"

#include <stdexcept>

#if NEVINS_ENABLE_RTLSDR
#include <rtl-sdr.h>
#endif

namespace nevins {

DeviceStatus RtlSdrSource::status() const {
#if NEVINS_ENABLE_RTLSDR
  const auto count = rtlsdr_get_device_count();
  return DeviceStatus{
      "RTL-SDR receiver",
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

std::vector<std::uint8_t> RtlSdrSource::read_iq(const CaptureSettings&) {
  throw std::runtime_error("Real RTL-SDR capture is scaffolded but not enabled for this build path.");
}

}  // namespace nevins
