#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Air"
#define AIRWINDOWS_DESCRIPTION "A different bright EQ from any other. Requires 44.1K."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','A','i','r' )
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
{ .name = "22K tap", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "15K tap", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "11K tap", .min = -1000, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "filters Q", .min = 0, .max = 1000, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Output Level", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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
 
		Float32 airPrevA;
		Float32 airEvenA;
		Float32 airOddA;
		Float32 airFactorA;
		bool flipA;
		Float32 airPrevB;
		Float32 airEvenB;
		Float32 airOddB;
		Float32 airFactorB;
		bool flipB;
		Float32 airPrevC;
		Float32 airEvenC;
		Float32 airOddC;
		Float32 airFactorC;
		bool flop;
		Float32 tripletPrev;
		Float32 tripletMid;
		Float32 tripletA;
		Float32 tripletB;
		Float32 tripletC;
		Float32 tripletFactor;
		int count;
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

	//This code will pass-thru the audio data.
	//This is where you want to process data to produce an effect.

	
	UInt32 nSampleFrames = inFramesToProcess;
	const Float32 *sourceP = inSourceP;
	Float32 *destP = inDestP;
	Float32 hiIntensity = -powf(GetParameter( kParam_One ),3)*2;
	Float32 tripletintensity = -powf(GetParameter( kParam_Two ),3);
	Float32 airIntensity = -powf(GetParameter( kParam_Three ),3)/2;
	Float32 filterQ = 2.1f-GetParameter( kParam_Four );
	Float32 output = GetParameter( kParam_Five );
	Float32 wet = GetParameter( kParam_Six );
	Float32 dry = 1.0f-wet;
	
	float inputSample;
	Float32 drySample;
	Float32 correction;
	//all types of air band are running in parallel, not series
	
	while (nSampleFrames-- > 0) {
		inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		drySample = inputSample;
		
		sourceP += inNumChannels;
		correction = 0.0f;
		
		
		if (count < 1 || count > 3) count = 1;
		switch (count)
		{
			case 1:
				tripletFactor = tripletPrev - inputSample;
				tripletA += tripletFactor;
				tripletC -= tripletFactor;
				tripletFactor = tripletA * tripletintensity;
				tripletPrev = tripletMid;
				tripletMid = inputSample;
				correction = correction + tripletFactor;
			break;
			case 2:
				tripletFactor = tripletPrev - inputSample;
				tripletB += tripletFactor;
				tripletA -= tripletFactor;
				tripletFactor = tripletB * tripletintensity;
				tripletPrev = tripletMid;
				tripletMid = inputSample;
				correction = correction + tripletFactor;
			break;
			case 3:
				tripletFactor = tripletPrev - inputSample;
				tripletC += tripletFactor;
				tripletB -= tripletFactor;
				tripletFactor = tripletC * tripletintensity;
				tripletPrev = tripletMid;
				tripletMid = inputSample;
				correction = correction + tripletFactor;
			break;
		}
		tripletA /= filterQ;
		tripletB /= filterQ;
		tripletC /= filterQ;
		count++;
		//finished Triplet section- 15K
		
		
		
		
		if (flop)
			{
			airFactorA = airPrevA - inputSample;
			if (flipA)
				{
				airEvenA += airFactorA;
				airOddA -= airFactorA;
				airFactorA = airEvenA * airIntensity;
				}
			else
				{
				airOddA += airFactorA;
				airEvenA -= airFactorA;
				airFactorA = airOddA * airIntensity;
				}
			airOddA = (airOddA - ((airOddA - airEvenA)/256.0f)) / filterQ;
			airEvenA = (airEvenA - ((airEvenA - airOddA)/256.0f)) / filterQ;
			airPrevA = inputSample;
			flipA = !flipA;
			correction = correction + airFactorA;
			}
		else
			{
			airFactorB = airPrevB - inputSample;
			if (flipB)
				{
				airEvenB += airFactorB;
				airOddB -= airFactorB;
				airFactorB = airEvenB * airIntensity;
				}
			else
				{
				airOddB += airFactorB;
				airEvenB -= airFactorB;
				airFactorB = airOddB * airIntensity;
				}
			airOddB = (airOddB - ((airOddB - airEvenB)/256.0f)) / filterQ;
			airEvenB = (airEvenB - ((airEvenB - airOddB)/256.0f)) / filterQ;
			airPrevB = inputSample;
			flipB = !flipB;
			correction = correction + airFactorB;
			}
		//11K one
		
		
		
		airFactorC = airPrevC - inputSample;
		if (flop)
			{
			airEvenC += airFactorC;
			airOddC -= airFactorC;
			airFactorC = airEvenC * hiIntensity;
			}
		else
			{
			airOddC += airFactorC;
			airEvenC -= airFactorC;
			airFactorC = airOddC * hiIntensity;
			}
		airOddC = (airOddC - ((airOddC - airEvenC)/256.0f)) / filterQ;
		airEvenC = (airEvenC - ((airEvenC - airOddC)/256.0f)) / filterQ;
		airPrevC = inputSample;
		correction = correction + airFactorC;
		flop = !flop;
		
		
		
		
		inputSample += correction;
		
		if (output < 1.0f) inputSample *= output;
		if (wet < 1.0f) inputSample = (drySample * dry)+(inputSample*wet);
		//nice little output stage template: if we have another scale of floating point
		//number, we really don't want to meaninglessly multiply that by 1.0f.		
		
		
		
		*destP = inputSample;
		destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	airPrevA = 0.0;
	airEvenA = 0.0;
	airOddA = 0.0;
	airFactorA = 0.0;
	flipA = false;
	airPrevB = 0.0;
	airEvenB = 0.0;
	airOddB = 0.0;
	airFactorB = 0.0;
	flipB = false;
	airPrevC = 0.0;
	airEvenC = 0.0;
	airOddC = 0.0;
	airFactorC = 0.0;
	flop = false;
	tripletPrev = 0.0;
	tripletMid = 0.0;
	tripletA = 0.0;
	tripletB = 0.0;
	tripletC = 0.0;
	tripletFactor = 0.0;
	count = 1;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};
