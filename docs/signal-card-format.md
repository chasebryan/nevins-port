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
- Policy status

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
* No content decoding attempted

Safe next step:

* Save IQ capture
* Replay capture locally
* Compare timing profile against known public examples
* Do not infer device owner or private content

Category: ISM
Policy: receive-only analysis; content decoding not attempted
```

Signal cards must avoid overclaiming. They describe observable signal properties and conservative classes, not private identities or message content.
