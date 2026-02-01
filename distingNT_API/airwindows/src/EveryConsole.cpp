#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "EveryConsole"
#define AIRWINDOWS_DESCRIPTION "Mix-and-match Airwindows Console."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','E','v','e' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	//Add your parameters here...
	kNumberOfParameters=3
};
static const int kRC = 1;
static const int kRB = 2;
static const int kSC = 3;
static const int kSB = 4;
static const int k6C = 5;
static const int k6B = 6;
static const int k7C = 7;
static const int k7B = 8;
static const int kBC = 9;
static const int kBB = 10;
static const int kZC = 11;
static const int kZB = 12;
static const int kDefaultValue_ParamOne = kSC;
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParamPrePostGain,
kParam0, kParam1, kParam2, };
static char const * const enumStrings0[] = { "", "Retro Channel", "Retro Buss", "Sin() Channel", "aSin() Buss", "C6 Channel", "C6 Buss", "C7 Channel", "C7 Buss", "BShifty Channel", "BShifty Buss", "CZero Channel", "CZero Buss", };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Console Type", .min = 1, .max = 12, .def = 3, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = enumStrings0 },
{ .name = "In Trim", .min = 0, .max = 2000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Out Trim", .min = 0, .max = 2000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, };
enum { kNumTemplateParameters = 4 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		uint32_t fpd;
	
	struct _dram {
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
	
	float half = 0.0f;
	float falf = 0.0f;
	int console = (int) GetParameter( kParam_One );
	Float32 inTrim = GetParameter( kParam_Two ); //0-2
	Float32 outTrim = GetParameter( kParam_Three );
	
	while (nSampleFrames-- > 0) {
		float inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		
		if (inTrim != 1.0f) {
			inputSample *= inTrim;
		}

		switch (console)
		{
			case kRC:
				half = inputSample * 0.83f;
				falf = fabs(half);
				half *= falf;
				half *= falf;
				inputSample -= half;
				break; //original ConsoleChannel, before sin/asin
				
			case kRB:
				half = inputSample * 0.885f;
				falf = fabs(half);
				half *= falf;
				half *= falf;
				inputSample += half;
				break; //original ConsoleBuss, before sin/asin
				
			case kSC:
				if (inputSample > M_PI_2) inputSample = M_PI_2;
				if (inputSample < -M_PI_2) inputSample = -M_PI_2;
				//clip to max sine without any wavefolding
				inputSample = sin(inputSample);
				break; //sin() function ConsoleChannel
				
			case kSB:
				if (inputSample > 1.0f) inputSample = 1.0f;
				if (inputSample < -1.0f) inputSample = -1.0f;
				//without this, you can get a NaN condition where it spits out DC offset at full blast!
				inputSample = asin(inputSample);
				break; //sin() function ConsoleBuss
				
			case k6C:
				//encode/decode courtesy of torridgristle under the MIT license
				if (inputSample > 1.0f) inputSample= 1.0f;
				else if (inputSample > 0.0f) inputSample = 1.0f - powf(1.0f-inputSample,2.0f);
				if (inputSample < -1.0f) inputSample = -1.0f;
				else if (inputSample < 0.0f) inputSample = -1.0f + powf(1.0f+inputSample,2.0f);
				//Inverse Square 1-(1-x)^2 and 1-(1-x)^0.5f for Console6Channel
				break; //crude sine. Note that because modern processors love math more than extra variables, this is optimized
				
			case k6B:
				//encode/decode courtesy of torridgristle under the MIT license
				if (inputSample > 1.0f) inputSample= 1.0f;
				else if (inputSample > 0.0f) inputSample = 1.0f - powf(1.0f-inputSample,0.5f);
				if (inputSample < -1.0f) inputSample = -1.0f;
				else if (inputSample < 0.0f) inputSample = -1.0f + powf(1.0f+inputSample,0.5f);
				//Inverse Square 1-(1-x)^2 and 1-(1-x)^0.5f for Console6Buss
				break; //crude arcsine. Note that because modern processors love math more than extra variables, this is optimized
				
			case k7C:
				if (inputSample > 1.097f) inputSample = 1.097f;
				if (inputSample < -1.097f) inputSample = -1.097f;
				inputSample = ((sin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0f) ?1:fabs(inputSample)))*0.8f)+(sin(inputSample)*0.2f);
				//this is a version of Spiral blended 80/20 with regular Density.
				//It's blending between two different harmonics in the overtones of the algorithm
				break; //Console7Channel
				
			case k7B:
				if (inputSample > 1.0f) inputSample = 1.0f;
				if (inputSample < -1.0f) inputSample = -1.0f;
				inputSample = ((asin(inputSample*fabs(inputSample))/((fabs(inputSample) == 0.0f) ?1:fabs(inputSample)))*0.618033988749894848204586f)+(asin(inputSample)*0.381966011250105f);
				//this is an asin version of Spiral blended with regular asin ConsoleBuss.
				//It's blending between two different harmonics in the overtones of the algorithm.
				break; //Console7Buss
				
			case kBC:
				inputSample += ((powf(inputSample,5)/128.0f) + (powf(inputSample,9)/262144.0f)) - ((powf(inputSample,3)/8.0f) + (powf(inputSample,7)/4096.0f));
				break; //crude sine. Note that because modern processors love math more than extra variables, this is optimized
				
			case kBB:
				inputSample += (powf(inputSample,3)/4.0f)+(powf(inputSample,5)/8.0f)+(powf(inputSample,7)/16.0f)+(powf(inputSample,9)/32.0f);
				break; //crude arcsine. Note that because modern processors love math more than extra variables, this is optimized
				
			case kZC:
				if (inputSample > 1.4137166941154f) inputSample = 1.4137166941154f;
				if (inputSample < -1.4137166941154f) inputSample = -1.4137166941154f;
				if (inputSample > 0.0f) inputSample = (inputSample/2.0f)*(2.8274333882308f-inputSample);
				else inputSample = -(inputSample/-2.0f)*(2.8274333882308f+inputSample);
				break; //ConsoleZeroChannel
				
			case kZB:
				if (inputSample > 2.8f) inputSample = 2.8f;
				if (inputSample < -2.8f) inputSample = -2.8f;
				if (inputSample > 0.0f) inputSample = (inputSample*2.0f)/(3.0f-inputSample);
				else inputSample = -(inputSample*-2.0f)/(3.0f+inputSample);
				break; //ConsoleZeroBuss
		}
		
		if (outTrim != 1.0f) {
			inputSample *= outTrim;
		}
		
		
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};
