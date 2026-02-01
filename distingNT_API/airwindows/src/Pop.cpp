#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Pop"
#define AIRWINDOWS_DESCRIPTION "A crazy overcompressor with a very exaggerated sound."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','P','o','p' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	//Add your parameters here...
	kNumberOfParameters=3
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParamPrePostGain,
kParam0, kParam1, kParam2, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Intensity", .min = 0, .max = 1000, .def = 300, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Output", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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
 
		float muVary;
		float muAttack;
		float muNewSpeed;
		float muSpeedA;
		float muSpeedB;
		float muCoefficientA;
		float muCoefficientB;
		float thicken;
		float previous;
		float previous2;
		float previous3;
		float previous4;
		float previous5;
		int delay;
		bool flip;
		uint32_t fpd;
	
	struct _dram {
			Float32 d[10001];
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
	Float32 overallscale = 1.0f;
	overallscale /= 44100.0f;
	overallscale *= GetSampleRate();
	
	Float32 highGainOffset = powf(GetParameter( kParam_One ),2)*0.023f;
	Float32 threshold = 1.001f - (1.0f-powf(1.0f-GetParameter( kParam_One ),5));
	Float32 muMakeupGain = sqrt(1.0f / threshold);
	//gain settings around threshold
	Float32 release = (GetParameter( kParam_One )*100000.0f) + 300000.0f;
	int maxdelay = (int)(1450.0f * overallscale);
	if (maxdelay > 9999) maxdelay = 9999;
	release /= overallscale;
	Float32 fastest = sqrt(release);
	//speed settings around release
	Float32 output = GetParameter( kParam_Two );
	Float32 wet = GetParameter( kParam_Three );
	// µ µ µ µ µ µ µ µ µ µ µ µ is the kitten song o/~
	
	
	while (nSampleFrames-- > 0) {
		float inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		float drySample = inputSample;
		
		dram->d[delay] = inputSample;
		delay--;
		if (delay < 0 || delay > maxdelay) {delay = maxdelay;}
		//yes this is a second bounds check. it's cheap, check EVERY time
		inputSample = (inputSample * thicken) + (dram->d[delay] * (1.0f-thicken));
		
		float lowestSample = inputSample;
		if (fabs(inputSample) > fabs(previous)) lowestSample = previous;
		if (fabs(lowestSample) > fabs(previous2)) lowestSample = (lowestSample + previous2) / 1.99f;
		if (fabs(lowestSample) > fabs(previous3)) lowestSample = (lowestSample + previous3) / 1.98f;
		if (fabs(lowestSample) > fabs(previous4)) lowestSample = (lowestSample + previous4) / 1.97f;
		if (fabs(lowestSample) > fabs(previous5)) lowestSample = (lowestSample + previous5) / 1.96f;
		previous5 = previous4;
		previous4 = previous3;
		previous3 = previous2;
		previous2 = previous;
		previous = inputSample;
		inputSample *= muMakeupGain;
		Float32 punchiness = 0.95f-fabs(inputSample*0.08f);
		if (punchiness < 0.65f) punchiness = 0.65f;

		
		if (flip)
			{
				if (fabs(lowestSample) > threshold)
					{
						muVary = threshold / fabs(lowestSample);
						muAttack = sqrt(fabs(muSpeedA));
						muCoefficientA = muCoefficientA * (muAttack-1.0f);
						if (muVary < threshold)
							{
								muCoefficientA = muCoefficientA + threshold;
							}
						else
							{
								muCoefficientA = muCoefficientA + muVary;
							}
						muCoefficientA = muCoefficientA / muAttack;
					}
				else
					{
						muCoefficientA = muCoefficientA * ((muSpeedA * muSpeedA)-1.0f);
						muCoefficientA = muCoefficientA + 1.0f;
						muCoefficientA = muCoefficientA / (muSpeedA * muSpeedA);
					}
				muNewSpeed = muSpeedA * (muSpeedA-1);
				muNewSpeed = muNewSpeed + fabs(lowestSample*release)+fastest;
				muSpeedA = muNewSpeed / muSpeedA;
			}
		else
			{
				if (fabs(lowestSample) > threshold)
					{
						muVary = threshold / fabs(lowestSample);
						muAttack = sqrt(fabs(muSpeedB));
						muCoefficientB = muCoefficientB * (muAttack-1);
						if (muVary < threshold)
							{
								muCoefficientB = muCoefficientB + threshold;
							}
						else
							{
								muCoefficientB = muCoefficientB + muVary;
							}
						muCoefficientB = muCoefficientB / muAttack;
					}
				else
					{
						muCoefficientB = muCoefficientB * ((muSpeedB * muSpeedB)-1.0f);
						muCoefficientB = muCoefficientB + 1.0f;
						muCoefficientB = muCoefficientB / (muSpeedB * muSpeedB);
					}
				muNewSpeed = muSpeedB * (muSpeedB-1);
				muNewSpeed = muNewSpeed + fabs(lowestSample*release)+fastest;
				muSpeedB = muNewSpeed / muSpeedB;
			}
		//got coefficients, adjusted speeds
		float coefficient = highGainOffset;
		if (flip) coefficient += powf(muCoefficientA,2);
		else coefficient += powf(muCoefficientB,2);
		inputSample *= coefficient;
		thicken = (coefficient/5)+punchiness;//0.80f;
		thicken = (1.0f-wet)+(wet*thicken);
		//applied compression with vari-vari-µ-µ-µ-µ-µ-µ-is-the-kitten-song o/~
		//applied gain correction to control output level- tends to constrain sound rather than inflate it
		
		float bridgerectifier = fabs(inputSample);
		if (bridgerectifier > 1.2533141373155f) bridgerectifier = 1.2533141373155f;
		bridgerectifier = sin(bridgerectifier * fabs(bridgerectifier)) / ((fabs(bridgerectifier) == 0.0f) ?1:fabs(bridgerectifier));
		//using Spiral instead of Density algorithm
		if (inputSample > 0) inputSample = (inputSample*coefficient)+(bridgerectifier*(1-coefficient));
		else inputSample = (inputSample*coefficient)-(bridgerectifier*(1-coefficient));
		//second stage of overdrive to prevent overs and allow bloody loud extremeness
		flip = !flip;
		
		if (output < 1.0f) inputSample *= output;
		if (wet < 1.0f) inputSample = (drySample*(1.0f-wet))+(inputSample*wet);
		
		

		*destP = inputSample;
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	for(int count = 0; count < 10000; count++) {dram->d[count] = 0;}
	delay = 0;
	muSpeedA = 10000;
	muSpeedB = 10000;
	muCoefficientA = 1;
	muCoefficientB = 1;
	thicken = 1;
	muVary = 1;
	flip = false;
	previous = 0.0;
	previous2 = 0.0;
	previous3 = 0.0;
	previous4 = 0.0;
	previous5 = 0.0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};
