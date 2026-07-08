#include "nevins/core_api.h"

#include <cassert>

int main() {
  const auto limits = nv_default_device_limits();

  const NvScanPlan valid_plan{
      433000000U,
      434000000U,
      25000U,
      2048000U,
      1000U,
  };
  assert(nv_validate_scan_plan(&valid_plan, &limits));

  const NvScanPlan invalid_order{
      434000000U,
      433000000U,
      25000U,
      2048000U,
      1000U,
  };
  assert(!nv_validate_scan_plan(&invalid_order, &limits));

  const NvScanPlan invalid_step{
      433000000U,
      434000000U,
      0U,
      2048000U,
      1000U,
  };
  assert(!nv_validate_scan_plan(&invalid_step, &limits));

  const NvScanPlan invalid_rate{
      433000000U,
      434000000U,
      25000U,
      1000U,
      1000U,
  };
  assert(!nv_validate_scan_plan(&invalid_rate, &limits));

  assert(nv_validate_confidence(0));
  assert(nv_validate_confidence(100));
  assert(!nv_validate_confidence(-1));
  assert(!nv_validate_confidence(101));

  assert(!nv_policy_allows_action(NV_SIGNAL_RESTRICTED_PRIVATE, NV_ACTION_DECODE_CONTENT));
  assert(!nv_policy_allows_action(NV_SIGNAL_UNKNOWN, NV_ACTION_DECODE_CONTENT));
  assert(nv_policy_allows_action(NV_SIGNAL_ISM, NV_ACTION_RECORD_IQ));
  assert(nv_policy_allows_action(NV_SIGNAL_ISM, NV_ACTION_REPLAY));

  return 0;
}
