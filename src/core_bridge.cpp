#include "nevins/core_api.h"
#include "nevins/version.h"

#include <cstring>
#include <limits>

namespace {

constexpr std::uint64_t kMinFrequencyHz = 24000000;
constexpr std::uint64_t kMaxFrequencyHz = 1766000000;
constexpr std::uint32_t kMinSampleRateHz = 250000;
constexpr std::uint32_t kMaxSampleRateHz = 3200000;
constexpr std::uint32_t kMaxDurationMs = 3600000;
constexpr std::uint32_t kMaxScanSteps = 100000;

bool valid_category(NvSignalCategory category) {
  switch (category) {
    case NV_SIGNAL_PUBLIC_BROADCAST:
    case NV_SIGNAL_AMATEUR:
    case NV_SIGNAL_WEATHER:
    case NV_SIGNAL_SATELLITE:
    case NV_SIGNAL_ISM:
    case NV_SIGNAL_AVIATION_PUBLIC:
    case NV_SIGNAL_UNKNOWN:
    case NV_SIGNAL_RESTRICTED_PRIVATE:
      return true;
  }
  return false;
}

bool valid_action(NvActionCategory action) {
  switch (action) {
    case NV_ACTION_EXPLAIN_ONLY:
    case NV_ACTION_RECORD_IQ:
    case NV_ACTION_REPLAY:
    case NV_ACTION_ROUTE_TO_EXTERNAL_TOOL:
    case NV_ACTION_DECODE_CONTENT:
      return true;
  }
  return false;
}

}  // namespace

extern "C" {

const char* nv_core_version(void) {
  return nevins::kVersion;
}

NvDeviceLimits nv_default_device_limits(void) {
  return NvDeviceLimits{
      kMinFrequencyHz,
      kMaxFrequencyHz,
      kMinSampleRateHz,
      kMaxSampleRateHz,
      kMaxDurationMs,
      kMaxScanSteps,
  };
}

int nv_validate_frequency_hz(std::uint64_t frequency_hz) {
  return frequency_hz >= kMinFrequencyHz && frequency_hz <= kMaxFrequencyHz;
}

int nv_validate_sample_rate_hz(std::uint32_t sample_rate_hz) {
  return sample_rate_hz >= kMinSampleRateHz && sample_rate_hz <= kMaxSampleRateHz;
}

int nv_validate_duration_ms(std::uint32_t duration_ms) {
  return duration_ms > 0 && duration_ms <= kMaxDurationMs;
}

int nv_validate_confidence(int confidence) {
  return confidence >= 0 && confidence <= 100;
}

int nv_validate_gain_db_tenths(int gain_db_tenths) {
  return gain_db_tenths >= -100 && gain_db_tenths <= 600;
}

int nv_validate_scan_plan(const NvScanPlan* plan, const NvDeviceLimits* limits) {
  if (plan == nullptr || limits == nullptr) {
    return 0;
  }
  if (plan->start_frequency_hz >= plan->end_frequency_hz) {
    return 0;
  }
  if (plan->step_hz == 0 || plan->dwell_ms == 0) {
    return 0;
  }
  if (limits->max_steps == 0 || limits->max_duration_ms == 0) {
    return 0;
  }
  if (limits->min_frequency_hz > limits->max_frequency_hz ||
      limits->min_sample_rate_hz > limits->max_sample_rate_hz) {
    return 0;
  }
  if (plan->start_frequency_hz < limits->min_frequency_hz ||
      plan->end_frequency_hz > limits->max_frequency_hz) {
    return 0;
  }
  if (plan->sample_rate_hz < limits->min_sample_rate_hz ||
      plan->sample_rate_hz > limits->max_sample_rate_hz) {
    return 0;
  }
  if (plan->dwell_ms > limits->max_duration_ms) {
    return 0;
  }

  const std::uint64_t span = plan->end_frequency_hz - plan->start_frequency_hz;
  const std::uint64_t steps = 1 + (span / plan->step_hz);
  if (steps > std::numeric_limits<std::uint32_t>::max()) {
    return 0;
  }
  return steps <= limits->max_steps;
}

int nv_policy_allows_action(NvSignalCategory category, NvActionCategory action) {
  if (!valid_category(category) || !valid_action(action)) {
    return 0;
  }
  if (category == NV_SIGNAL_RESTRICTED_PRIVATE) {
    return action != NV_ACTION_DECODE_CONTENT && action != NV_ACTION_ROUTE_TO_EXTERNAL_TOOL;
  }
  if (category == NV_SIGNAL_UNKNOWN) {
    return action != NV_ACTION_DECODE_CONTENT && action != NV_ACTION_ROUTE_TO_EXTERNAL_TOOL;
  }
  return 1;
}

const char* nv_signal_category_name(NvSignalCategory category) {
  switch (category) {
    case NV_SIGNAL_PUBLIC_BROADCAST:
      return "PublicBroadcast";
    case NV_SIGNAL_AMATEUR:
      return "Amateur";
    case NV_SIGNAL_WEATHER:
      return "Weather";
    case NV_SIGNAL_SATELLITE:
      return "Satellite";
    case NV_SIGNAL_ISM:
      return "ISM";
    case NV_SIGNAL_AVIATION_PUBLIC:
      return "AviationPublic";
    case NV_SIGNAL_UNKNOWN:
      return "Unknown";
    case NV_SIGNAL_RESTRICTED_PRIVATE:
      return "RestrictedPrivate";
  }
  return "Unknown";
}

const char* nv_action_category_name(NvActionCategory action) {
  switch (action) {
    case NV_ACTION_EXPLAIN_ONLY:
      return "ExplainOnly";
    case NV_ACTION_RECORD_IQ:
      return "RecordIQ";
    case NV_ACTION_REPLAY:
      return "Replay";
    case NV_ACTION_ROUTE_TO_EXTERNAL_TOOL:
      return "RouteToExternalTool";
    case NV_ACTION_DECODE_CONTENT:
      return "DecodeContent";
  }
  return "ExplainOnly";
}

NvSignalCategory nv_signal_category_from_name(const char* name) {
  if (name == nullptr) {
    return NV_SIGNAL_UNKNOWN;
  }
  if (std::strcmp(name, "PublicBroadcast") == 0) {
    return NV_SIGNAL_PUBLIC_BROADCAST;
  }
  if (std::strcmp(name, "Amateur") == 0) {
    return NV_SIGNAL_AMATEUR;
  }
  if (std::strcmp(name, "Weather") == 0) {
    return NV_SIGNAL_WEATHER;
  }
  if (std::strcmp(name, "Satellite") == 0) {
    return NV_SIGNAL_SATELLITE;
  }
  if (std::strcmp(name, "ISM") == 0) {
    return NV_SIGNAL_ISM;
  }
  if (std::strcmp(name, "AviationPublic") == 0) {
    return NV_SIGNAL_AVIATION_PUBLIC;
  }
  if (std::strcmp(name, "RestrictedPrivate") == 0) {
    return NV_SIGNAL_RESTRICTED_PRIVATE;
  }
  return NV_SIGNAL_UNKNOWN;
}

}
