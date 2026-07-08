# Signal Card Format

Every detected signal should produce a plain-English signal card.

Required fields:

- Frequency
- Bandwidth estimate
- SNR or power estimate if available
- Duration or analysis window
- Modulation-class guess
- Confidence score
- Why the system thinks that
- What is safe to try next
- What not to assume
- Category: public, amateur, weather, satellite, ISM, aviation-public, unknown, or restricted/private
- Routing status

Example:

```text
SIGNAL CARD

Frequency: 433.920 MHz
Type guess: Bursty OOK/ASK-like ISM signal
Confidence: 68%

Why:

* Short repeated burst pattern
* Narrow occupied bandwidth
* Common ISM-band frequency range
* Shape, power, timing, and frequency range are the only inputs

Safe next step:

* Save IQ capture
* Replay capture locally
* Compare timing profile against known public examples

What not to assume:

* Do not infer identity or message from signal shape alone

Category: ISM
Routing: shape-only analysis route
```

Signal cards must avoid overclaiming. They describe observable signal properties and conservative classes, not private identities or message content.
