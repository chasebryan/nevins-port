module Nevins.Policy

type signal_category =
  | PublicBroadcast
  | Amateur
  | Weather
  | Satellite
  | ISM
  | AviationPublic
  | Unknown
  | RestrictedPrivate

type action_category =
  | ExplainOnly
  | RecordIQ
  | Replay
  | RouteToExternalTool
  | DecodeContent

let conservative_category (c:signal_category) : Tot bool =
  match c with
  | Unknown -> true
  | RestrictedPrivate -> true
  | _ -> false

let action_allowed (c:signal_category) (a:action_category) : Tot bool =
  match c, a with
  | RestrictedPrivate, DecodeContent -> false
  | RestrictedPrivate, RouteToExternalTool -> false
  | Unknown, DecodeContent -> false
  | Unknown, RouteToExternalTool -> false
  | _, _ -> true

let unknown_is_conservative () : Lemma (conservative_category Unknown == true) =
  ()

let restricted_cannot_decode () : Lemma (action_allowed RestrictedPrivate DecodeContent == false) =
  ()

let unknown_cannot_decode () : Lemma (action_allowed Unknown DecodeContent == false) =
  ()
