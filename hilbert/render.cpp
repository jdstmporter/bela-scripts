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
#include <complex>
#include <libraries/ne10/NE10.h> // neon library
#include <libraries/Scope/Scope.h>


typedef ne10_float32_t nfloat;
typedef ne10_uint32_t  nuint;
typedef std::complex<ne10_float32_t> ncomplex;


// filter vars
ne10_fir_instance_f32_t hilbertFilter;
nfloat *hilbertTaps;
nfloat *hilbertState;
ne10_fir_instance_f32_t delayFilter;
nfloat *delayTaps;
nfloat *delayState;

nfloat *input;
nfloat *imag;
nfloat *delayed;

ncomplex *inter;
ncomplex *shifter;

#define NSTAGES 66
#define FILTER_TAP_NUM (1+2*NSTAGES)

template<typename T>
T * get(nuint n) {
	return (T *)NE10_MALLOC(n*sizeof(T));
}

#define FREQ 1000.0

unsigned blockSize;
unsigned nChannels;

nfloat phase;
//nuint offset=0;
nfloat invSampleRate;
nfloat shiftInvSampleRate;
nfloat shiftPhase=0;

nfloat shiftFreq;

Scope scope;

void toComplex() {
	ne10_fir_float_neon(&hilbertFilter, input, imag, blockSize);
	ne10_fir_float_neon(&delayFilter, input, delayed, blockSize);
	for(auto i=0;i<blockSize;i++) inter[i]=ncomplex(input[i],delayed[i]);
}


bool setup(BelaContext *context, void *userData)
{
	// Retrieve a parameter passed in from the initAudio() call
	
	// tell the scope how many channels and the sample rate
	scope.setup(2, context->audioSampleRate);

	blockSize = context->audioFrames;
	nChannels = context -> audioOutChannels;
	
	input=get<nfloat>(blockSize);
	imag=get<nfloat>(blockSize);
	delayed=get<nfloat>(blockSize);
	
	shifter=get<ncomplex>(blockSize);
	
	inter=new ncomplex[blockSize];
	
	hilbertTaps=get<nfloat>(FILTER_TAP_NUM);
	for(nuint i=0;i<NSTAGES;i++) {
		nfloat value = (0==(i&1)) ? 0.0 : 2.0/(M_PI*i);
		hilbertTaps[NSTAGES+i]=value;
		hilbertTaps[NSTAGES-i]=-value;
	}
	hilbertState=get<nfloat>(FILTER_TAP_NUM+blockSize-1);
	ne10_fir_init_float(&hilbertFilter, FILTER_TAP_NUM, hilbertTaps, hilbertState, blockSize);
	
	delayTaps=get<nfloat>(FILTER_TAP_NUM);
	for(nuint i=0;i<FILTER_TAP_NUM;i++) {
		delayTaps[i] = (i==NSTAGES) ? 1.0 : 0.0;
	}
	delayState=get<nfloat>(FILTER_TAP_NUM+blockSize-1);
	ne10_fir_init_float(&delayFilter, FILTER_TAP_NUM, delayTaps, delayState, blockSize);

	shiftFreq = 1000.0 * analogRead(context,0,0);
	invSampleRate = 2.0*(nfloat)M_PI*FREQ/(nfloat)context->audioSampleRate;
	shiftInvSampleRate = 2.0*(nfloat)M_PI/(nfloat)context->audioSampleRate;

	return true;
}

void render(BelaContext *context, void *userData)
{
	shiftFreq = (analogRead(context,0,0)-0.5) * 8000.0;
	auto nSamples = context->audioFrames;
	for(unsigned n = 0; n < nSamples; n++) {
		input[n] = audioRead(context,n,0);
		
		shifter[n]=std::polar<nfloat>(1.0,shiftPhase);
		shiftPhase += shiftInvSampleRate*shiftFreq;
		while (shiftPhase > M_PI) shiftPhase -= 2.0f * (nfloat)M_PI;
		while (shiftPhase < -M_PI) shiftPhase += 2.0f * (nfloat)M_PI;
		
	}
	ne10_fir_float_neon(&hilbertFilter, input, imag, blockSize);
	ne10_fir_float_neon(&delayFilter, input, delayed, blockSize);
	for(auto i=0;i<nSamples;i++) {
		inter[i]=ncomplex(imag[i],delayed[i]) * shifter[i];
	}

	for(unsigned n=0;n<blockSize;n++) {
		scope.log(input[n],inter[n].real());
		audioWrite(context, n, 0, input[n]);
		audioWrite(context, n, 1, inter[n].real());
	}
}




void cleanup(BelaContext *context, void *userData)
{
	NE10_FREE(hilbertTaps);
	NE10_FREE(hilbertState);
	NE10_FREE(delayTaps);
	NE10_FREE(delayState);
	NE10_FREE(input);
	NE10_FREE(imag);
	NE10_FREE(shifter);
	delete[] inter;
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
