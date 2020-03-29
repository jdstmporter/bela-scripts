# bela-scripts
## Sample scripts for sound processing on the [Bela][B] platform

### hilbert

This demonstrates the [discrete Hilbert transform][H] by using it to implement a true frequency shifter, i.e. one that does not rely on grains, or other destructive algorithms, and hence preserves the original waveform and its spectrum.

*NB*: currently mono only.

**Signals:**

  - Audio in L :  audio input
  - Audio out L : echoes the input
  - Audio out R : the shifted output
  - Analogue in 0 : CV for the frequency to shift by 
    (currently full range is 0 - 4000 Hz)

### Notes
Works well for frequency shift up; need to deal with aliasing issues for shift down.  Current version gets around this by only implementing shift *up*.

[B]: http://bela.io
[H]: https://flylib.com/books/en/2.729.1/impulse_response_of_a_hilbert_transformer.html

