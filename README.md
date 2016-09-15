Reduced Complexity ManyCore (RC/MC)
===================================

The RC/MC architecture consists of a large number of small RISC-V processor
cores that are connected by the real-time capable PaterNoster Network-on-Chip.
In contrast to conventional multicores there is no shared memory or cache
hierarchy. Instead, each core has a small private memory and communicates with
the other cores via explicit fine grained messages.

Small cores provide a high performance/Watt ratio and a good timing
predictability. Therefore we investigate, if the RC/MC architecture can provide
the same performance as conventional shared memory multicores with lower energy
consumption and better real-time capabilities.



