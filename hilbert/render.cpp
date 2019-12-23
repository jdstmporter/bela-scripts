/*
 ____  _____ _        _    
| __ )| ____| |      / \   
|  _ \|  _| | |     / _ \  
| |_) | |___| |___ / ___ \ 
|____/|_____|_____/_/   \_\

The platform for ultra-low latency audio and sensor processing

http://bela.io

A project of the Augmented Instruments Laboratory within the
Centre for Digital Music at Queen Mary University of London.
http://www.eecs.qmul.ac.uk/~andrewm

(c) 2016 Augmented Instruments Laboratory: Andrew McPherson,
  Astrid Bin, Liam Donovan, Christian Heinrichs, Robert Jack,
  Giulio Moro, Laurel Pardue, Victor Zappi. All rights reserved.

The Bela software is distributed under the GNU Lesser General Public License
(LGPL 3.0), available here: https://www.gnu.org/licenses/lgpl-3.0.txt
*/


#define ENABLE_NE10_FIR_FLOAT_NEON	// Define needed for Ne10 library

#include <Bela.h>
#include <cmath>
#include <libraries/ne10/NE10.h> // neon library
#include <libraries/Scope/Scope.h>


typedef ne10_float32_t nfloat;
typedef ne10_uint32_t  nuint;

// filter vars
ne10_fir_instance_f32_t filter;
nfloat normaliser;
nfloat *input;
nfloat *output;

nfloat *state;
nfloat *taps;

#define NSTAGES 64
#define FILTER_TAP_NUM (1+2*NSTAGES)

nfloat * get(nuint n) {
	return (nfloat *)NE10_MALLOC(n*sizeof(nfloat));
}

#define FREQ 1000.0

unsigned blockSize;
unsigned nChannels;

nfloat phase;
nuint offset=0;
nfloat invSampleRate;

Scope scope;




bool setup(BelaContext *context, void *userData)
{
	// Retrieve a parameter passed in from the initAudio() call
	
	// tell the scope how many channels and the sample rate
	scope.setup(2, context->audioSampleRate);

	blockSize = context->audioFrames;
	nChannels = context -> audioOutChannels;
	state=get(FILTER_TAP_NUM+blockSize-1);
	input=get(blockSize);
	output=get(blockSize);
	taps=get(FILTER_TAP_NUM);
	
	nfloat norm = 0.0;
	for(nuint i=0;i<NSTAGES;i++) {
		nfloat value = (0==(i&1)) ? 0.0 : 2.0/(M_PI*i);
		taps[NSTAGES+i]=value;
		taps[NSTAGES-i]=-value;
		norm+=value;
	}
	normaliser=1.0/norm;
	ne10_fir_init_float(&filter, FILTER_TAP_NUM, taps, state, blockSize);

	invSampleRate = 2.0*(nfloat)M_PI*FREQ/(nfloat)context->audioSampleRate;
	offset = 0;


	return true;
}

void render(BelaContext *context, void *userData)
{
	for(unsigned n = 0; n < context->audioFrames; n++) {
		input[n] = 0.8f * cos(phase);
		phase += invSampleRate;
		if(phase > M_PI) phase -= 2.0f * (nfloat)M_PI;
	}

	ne10_fir_float_neon(&filter, input, output, blockSize);
	
	for(unsigned n=0;n<blockSize;n++) {
		scope.log(input[n],output[n]);
		audioWrite(context, n, 0, input[n]);
		audioWrite(context, n, 1, output[n]);
	}
}




void cleanup(BelaContext *context, void *userData)
{
	NE10_FREE(taps);
	NE10_FREE(state);
	NE10_FREE(input);
	NE10_FREE(output);
}


/**
\example filter-FIR/render.cpp

Finite Impulse Response Filter
------------------------------

This scripts needs to be run in a terminal because it requires you to interact with Bela using your computer's keyboard.
Note that it cannot be run from within the IDE or the IDE's console.

See <a href="https://github.com/BelaPlatform/Bela/wiki/Interact-with-Bela-using-the-Bela-scripts" target="_blank">here</a> how to use Bela with a terminal.

In this project an audio recording processesd through an FIR filter.

To control the playback of the audio sample, use your computer keyboard, by pressing:

'a' \<enter\> to start playing the sample

's' \<enter\> to stop

'q' \<enter\> or ctrl-C to quit

*/
