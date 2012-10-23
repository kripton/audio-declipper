audio-declipper
===============

Proof-of-concept (or more) to declip sample-based audio files

Abstract
--------

This tool was created by the need to declip recorded wave-files that 
were recorded with a little too high gain. Currently it uses linear 
interpolation which is not the most ideal thing we could do but 
sine-based interpolation is planned.

The idea behind
---------------

In the analog world, things are quite simple. We use voltage to transfer signals 
from one system to another using a defined reference level as 0dB. Voltages 
will be smaller or maybe larger than the reference level. Whether the receiving 
system can safely handle or cope with the input voltage is another story but 
theoretically it could be pretty high.

In the digital world, we need to set a fixed upper level since computers 
cannot calculate with arbitrary high numbers (I'm talking about integers here 
as we get it from our audio interface. When using floats as internal 
representation, we could go higher but saving this to a file is again 
critical). Everything above 0dB is "clipped", meaning it just won't go higher 
and stay on the highest possible value. If you have many or large parts your 
audio-material will be distorted. 

So what if we could go higher? That is what I want to try with this tool. 
To get more headroom ("space to our upper limit 0dB") we divide every sample 
from the source by 2. _This will reduce your audio's resolution by 1 bit!_ 
Afterwards, the samples that were clipped before are re-calculated. Currently, 
linear interpolation is used which makes the waveform a straight whereby the 
slope is defined by the sample right before and right after the clip. For the 
future, a since-based interpolation is planned which might deliver better 
(closer to the lost, original information).

If there are longer durations of clipping, the linear interpolation might still 
deliver clipped audio. In this case you can run the declipper again (and loose 
another bit of your audio's resolution :P).

Magic?
------

A magic tool to de-clip audio files? Surely not! What got lost once (during 
recording) cannot be restored. Furthermore, you loose one bit of 
resolution (granularity) of your input on every run.

So how do we do it?
-------------------

We open the audio file using the libsndfile (means we can read anything supported 
by that library on your system and as long as the file is _mono (one channel only)_. 

### Linear interpolation:
Then we scan through the samples, checking if it has been clipped. If so, we 
calculate the first slope from the last sample before the clip and the first 
clipped sample and the second slope from the last clipped sample to the first 
non-clipped sample. Then we divide the length of the clipped samples into 
two parts. If the length is odd, the sample in the middle of the clipped region 
is calculated from the second half. Now the tool calculates a new value for 
every clipped sample using its position in the clipped region and the slope 
used for the half the sample is in.

All samples are then multiplied by 0.5 (divided by 2) and written to an output 
file _which is always a float WAVE file, no matter what the input format was_.

How are the results?
--------------------

I made some screenshots showing audacity and three short snippets from an audio 
file that was normalized first and then ampified by 12dB to create clipping. 
The input is shown above and the output is shown underneath it.
_No real-world tests have been done yet as this is a proof-of-concept for now_

Further Ideas
-------------

- Sine-based interpolation
- use buffered instead of single-sample procession through the file
- command line parameters for configuration
- (more output formats?)
- (multi-channel support? maybe: coupled changing of all channels or independent? configurable?)
- LADSPA/LV2-plugin? (achievable latency: 2x buffer size. Maybe not relly realtime).

Installation
------------

Dependencies:
- libsndfile (+ headers)
- Qt4 (QtCore is enough) (+ headers and qmake)

Checkout the repo, cd into it, run "qmake ." and "make".

If everything worked okay: "./audio-declipper testfile.wav"


Further notes
-------------

By now (thanks to Georg on the linux-audio-user mailing list) I've learned 
that there's xiph.org's postfish which includes a nice declipper. That one 
uses frequency-analysis via FFTW to restore the lost samples:
https://svn.xiph.org/trunk/postfish

Comparison to the linear declipper is in the "results"-folder, named banjo.png
