#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "ADT"
#define AIRWINDOWS_DESCRIPTION "A double short delay tap with saturation."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','A','D','T' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	kParam_Five =4,
	kParam_Six =5,
	//Add your parameters here...
	kNumberOfParameters=6
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParamPrePostGain,
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Headroom", .min = 0, .max = 2000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "A Delay Time", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "A Level", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "B Delay Time", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "B Level", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Output Level", .min = 0, .max = 2000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, };
enum { kNumTemplateParameters = 4 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		int gcount;
		Float32 offsetA;
		Float32 offsetB;
		uint32_t fpd;
	
	struct _dram {
			Float32 p[10000];
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
	
	Float32 gain = GetParameter( kParam_One ) * 0.636f;
	Float32 targetA = powf(GetParameter( kParam_Two ),4) * 4790.0f;
	Float32 fractionA;
	Float32 minusA;
	Float32 intensityA = GetParameter( kParam_Three ) / 2.0f;
	//first delay
	Float32 targetB = (powf(GetParameter( kParam_Four ),4) * 4790.0f);
	Float32 fractionB;
	Float32 minusB;
	Float32 intensityB = GetParameter( kParam_Five ) / 2.0f;
	//second delay
	Float32 output = GetParameter( kParam_Six );
	Float32 total;
	int count;
	Float32 temp;
	while (nSampleFrames-- > 0) {
		float inputSample = *sourceP;
		
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		
		
		if (fabs(offsetA - targetA) > 1000) offsetA = targetA;
		offsetA = ((offsetA*999.0f)+targetA)/1000.0f;
		fractionA = offsetA - floor(offsetA);
		minusA = 1.0f - fractionA;
		
		if (fabs(offsetB - targetB) > 1000) offsetB = targetB;
		offsetB = ((offsetB*999.0f)+targetB)/1000.0f;
		fractionB = offsetB - floor(offsetB);
		minusB = 1.0f - fractionB;
		//chase delay taps for smooth action
		
		if (gain > 0) inputSample /= gain;
		
		if (inputSample > 1.2533141373155f) inputSample = 1.2533141373155f;
		if (inputSample < -1.2533141373155f) inputSample = -1.2533141373155f;
		inputSample = sin(inputSample * fabs(inputSample)) / ((fabs(inputSample) == 0.0f) ?1:fabs(inputSample));
		//Spiral: lean out the sound a little when decoded by ConsoleBuss
		
		if (gcount < 1 || gcount > 4800) {gcount = 4800;}
		count = gcount;
		total = 0.0f;
		dram->p[count+4800] = dram->p[count] = inputSample;
		//float buffer
		
		if (intensityA != 0.0f)
		{
			count = (int)(gcount+floor(offsetA));
			temp = (dram->p[count] * minusA); //less as value moves away from .0
			temp += dram->p[count+1]; //we can assume always using this in one way or another?
			temp += (dram->p[count+2] * fractionA); //greater as value moves away from .0
			temp -= (((dram->p[count]-dram->p[count+1])-(dram->p[count+1]-dram->p[count+2]))/50); //interpolation hacks 'r us
			total += (temp * intensityA);
		}
		
		if (intensityB != 0.0f)
		{
			count = (int)(gcount+floor(offsetB));
			temp = (dram->p[count] * minusB); //less as value moves away from .0
			temp += dram->p[count+1]; //we can assume always using this in one way or another?
			temp += (dram->p[count+2] * fractionB); //greater as value moves away from .0
			temp -= (((dram->p[count]-dram->p[count+1])-(dram->p[count+1]-dram->p[count+2]))/50); //interpolation hacks 'r us
			total += (temp * intensityB);
		}
		
		gcount--;
		//still scrolling through the samples, remember
		
		inputSample += total;
		
		if (inputSample > 1.0f) inputSample = 1.0f;
		if (inputSample < -1.0f) inputSample = -1.0f;
		//without this, you can get a NaN condition where it spits out DC offset at full blast!
		
		inputSample = asin(inputSample);
		//amplitude aspect
		
		inputSample *= gain;
		
		if (output < 1.0f) inputSample *= output;
		
		
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	for(int count = 0; count < 9999; count++) {dram->p[count] = 0;}
	offsetA = 9001;
	offsetB = 9001;  //  :D
	gcount = 0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};
