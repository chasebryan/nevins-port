#include "nevins/signal_card.h"

#include <cassert>
#include <string>

int main() {
  const auto ism_card = nevins::identify_frequency(433920000U, 1000U);
  assert(ism_card.category == NV_SIGNAL_ISM);
  assert(ism_card.modulation == nevins::ModulationClass::OokAskBurstLike);
  assert(ism_card.confidence >= 0);
  assert(ism_card.confidence <= 100);
  assert(!ism_card.why.empty());
  assert(!ism_card.safe_next_steps.empty());
  assert(!ism_card.do_not_assume.empty());

  const auto rendered = nevins::format_signal_card(ism_card);
  assert(rendered.find("Frequency: 433.920 MHz") != std::string::npos);
  assert(rendered.find("Bursty OOK/ASK-like ISM signal") != std::string::npos);
  assert(rendered.find("Do not infer identity or message from signal shape alone") != std::string::npos);
  assert(rendered.find("Routing: shape-only analysis route") != std::string::npos);

  const auto restricted_card = nevins::identify_frequency(800000000U, 1000U);
  assert(restricted_card.category == NV_SIGNAL_RESTRICTED_PRIVATE);
  assert(!nv_policy_allows_action(restricted_card.category, NV_ACTION_DECODE_CONTENT));
  assert(restricted_card.policy.find("decode route unavailable") != std::string::npos);

  return 0;
}
