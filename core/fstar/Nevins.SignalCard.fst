module Nevins.SignalCard

open Nevins.Types
open Nevins.Policy

type guidance_class =
  | NormalGuidance
  | ConservativeGuidance

type signal_card = {
  frequency_hz: frequency_hz;
  bandwidth_estimate_hz: bandwidth_hz;
  duration_ms: duration_ms;
  modulation_guess: string;
  confidence: confidence;
  category: signal_category;
  explanation_present: bool;
  safe_next_step_present: bool;
  guidance: guidance_class
}

let restricted_guidance_is_conservative (c:signal_category) (g:guidance_class) : Tot bool =
  match c, g with
  | RestrictedPrivate, ConservativeGuidance -> true
  | RestrictedPrivate, NormalGuidance -> false
  | Unknown, ConservativeGuidance -> true
  | Unknown, NormalGuidance -> false
  | _, _ -> true

let valid_signal_card (card:signal_card) : Tot bool =
  card.explanation_present &&
  card.safe_next_step_present &&
  valid_confidence card.confidence &&
  restricted_guidance_is_conservative card.category card.guidance

let restricted_cards_are_conservative (card:signal_card { card.category == RestrictedPrivate }) :
  Lemma (valid_signal_card card ==> card.guidance == ConservativeGuidance) =
  ()
