/*
Sample deomonstration code for Bela, showing capability to perform Steve Reich's 'Clapping Music' in 
the version for two performers (obviously, more tracks could be added)

Copyright 2020 Julian Porter

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this 
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation and/or other 
materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific 
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 

EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <Bela.h>
#include <libraries/Scope/Scope.h>
#include <vector>

struct Pattern {
	std::vector<bool> values;
	unsigned offset;
	
	Pattern(const unsigned n) : values(n,false), offset(0) {}
	Pattern(const std::vector<int> &v) : values(v.size(),false), offset(0) {
		for(auto n=0;n<v.size();n++) values[n]=v[n]!=0;
	} 
	Pattern(const Pattern &) = default;
	virtual ~Pattern() = default;
	
	void reset() { offset=0; }
	unsigned int size() const { return values.size(); }
	float operator *() const { return values[offset]  ? 1.0 : 0.0; }
	void next() { offset = (1+offset)%size(); }
};

Pattern reich1({1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0});
Pattern reich2({1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,0,
				1,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0});
Scope scope;

bool tick=false;

bool setup(BelaContext *context, void *userData)
{
	scope.setup(3, context->audioSampleRate);
	return true;
}

void render(BelaContext *context, void *userData)
{
	auto nSamples = context->analogFrames;
	for(auto n=0;n<nSamples;n++) {
		auto clkIn = analogRead(context,n,0);
		bool clk = clkIn>0.5;
		if(clk!=tick) {
			tick=clk;
			if(tick) { 
				reich1.next();
				reich2.next();
			}
		}
		float v1 = *reich1*0.6;
		float v2 = *reich2*0.6;
		analogWriteOnce(context,n,0,v1);
		analogWriteOnce(context,n,1,v2);
		scope.log(clkIn,v1,v2);
	}
}

void cleanup(BelaContext *context, void *userData)
{

}