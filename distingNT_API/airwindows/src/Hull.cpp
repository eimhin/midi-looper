#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Hull"
#define AIRWINDOWS_DESCRIPTION "An alternate form of highpass/lowpass filter."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','H','u','l' )
#define AIRWINDOWS_TAGS kNT_tagFilterEQ
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	//Add your parameters here...
	kNumberOfParameters=2
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParamPrePostGain,
kParam0, kParam1, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Freq", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Bright", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, };
enum { kNumTemplateParameters = 4 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		int bPointer;
		int cPointer;
		uint32_t fpd;
	
	struct _dram {
			float b[4005];	
		float c[105];
	};
	_dram* dram;
};
_kernel kernels[1];

#include "../include/template2.h"
#include "../include/templateKernels.h"
void _airwindowsAlgorithm::_kernel::render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess ) {
#define inNumChannels (1)
{
	UInt32 nSampleFrames = inFramesToProcess;
	const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;
	float overallscale = 1.0f;
	overallscale /= 96000.0f;
	overallscale *= GetSampleRate(); //this one's scaled to 96k for the deepest bass
	if (overallscale > 1.0f) overallscale = 1.0f; //and if you go for 192k, rather than crash
	//it just cuts out the maximum (2000) depth of averaging you can get
	Float32 hullSetting = powf(GetParameter( kParam_One ),3)*overallscale;
	int limitA = (hullSetting*2000.0f)+1.0f;
	float divisorA = 1.0f/limitA;
	int limitB = (hullSetting*1000.0f)+1.0f;
	float divisorB = 1.0f/limitB;
	int limitC = sqrt(hullSetting*2000.0f)+1.0f;
	float divisorC = 1.0f/limitC;
	Float32 wet = -GetParameter( kParam_Two ); //functions as dark/bright
		
	while (nSampleFrames-- > 0) {
		float inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		float drySample = inputSample;
		
		bPointer--; if (bPointer < 0) bPointer += 2000;
		dram->b[bPointer] = dram->b[bPointer+2000] = inputSample;

		int x = bPointer;
		float longAverage = 0.0f;
		float shortAverage = 0.0f;
		while (x < bPointer+limitB) {
			shortAverage += dram->b[x];
			longAverage += dram->b[x];
			x++;
		} //once shorter average is reached, continue to longer
		while (x < bPointer+limitA) {
			longAverage += dram->b[x];
			x++;
		}
		longAverage *= divisorA;
		shortAverage *= divisorB;
		
		
		cPointer--; if (cPointer < 0) cPointer += 50;
		dram->c[cPointer] = dram->c[cPointer+50] = shortAverage+(shortAverage-longAverage);
		
		x = cPointer;
		float shortestAverage = 0.0f;
		while (x < cPointer+limitC) {
			shortestAverage += dram->c[x];
			x++;
		}
		shortestAverage *= divisorC;
				
		if (wet > 0.0f) {
			inputSample = (shortestAverage * wet)+(drySample * (1.0f-wet)); //dark
		} else {
			inputSample = ((inputSample-shortestAverage) * fabs(wet))+(drySample * (1.0f+wet)); //bright
		}		
		
		
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	for(int count = 0; count < 4002; count++) dram->b[count] = 0.0;
	for(int count = 0; count < 102; count++) dram->c[count] = 0.0;
	bPointer = 1; cPointer = 1;
	
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};
