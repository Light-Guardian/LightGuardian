## File Description

* ```robustness.cpp``` simulates the transimission and reconstruction procedure when some end-hosts break down.  We use random selection as the sketchlet selection strategy.
* ```robustness_kchance.cpp``` simulates the transimission and reconstruction procedure when some end-hosts break down. We use k+chance selection as the sketchlets selection strategy.
* ```robustness_rate.cpp``` calculates *the probability that all sketches can be reconstructed* and *the percentage of sketches that can be fully reconstructed* as the number of the broken end-hosts changes.

## Usage

We describe the inputs to the three programs as follows.

* ```robustness.cpp```
  * \# sketchlets.
  * \# broken end-hosts.
  * A random seed.
* ```robustness_kchance.cpp```
  * \# sketchlets.
  * A parameter *k*，which refers to the program using *k+chance* selection algorithm.
  * \# the broken end-hosts.
  * A random seed.
* ```robustness_rate.cpp```
  * \# sketchlets.
  * \# broken end-hosts.

