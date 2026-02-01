#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "EQ "
#define AIRWINDOWS_DESCRIPTION "Just the EQ parts of CStrip."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','E','Q',' ' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	kParam_Five =4,
	kParam_Six =5,
	kParam_Seven =6,
	kParam_Eight =7,
	//Add your parameters here...
	kNumberOfParameters=8
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParamPrePostGain,
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Treble", .min = -12000, .max = 12000, .def = 0, .unit = kNT_unitDb, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Mid", .min = -12000, .max = 12000, .def = 0, .unit = kNT_unitDb, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Bass", .min = -12000, .max = 12000, .def = 0, .unit = kNT_unitDb, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Lowpass", .min = 1000, .max = 16000, .def = 16000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Treble Freq", .min = 1000, .max = 16000, .def = 6000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Bass Freq", .min = 300, .max = 16000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling10, .enumStrings = NULL },
{ .name = "Highpass", .min = 300, .max = 16000, .def = 300, .unit = kNT_unitNone, .scaling = kNT_scaling10, .enumStrings = NULL },
{ .name = "Output Gain", .min = -18000, .max = 18000, .def = 0, .unit = kNT_unitDb, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, kParam5, kParam6, kParam7, };
enum { kNumTemplateParameters = 4 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
 
		Float32 iirHighSampleA;
		Float32 iirHighSampleB;
		Float32 iirHighSampleC;
		Float32 iirHighSampleD;
		Float32 iirHighSampleE;
		Float32 iirLowSampleA;
		Float32 iirLowSampleB;
		Float32 iirLowSampleC;
		Float32 iirLowSampleD;
		Float32 iirLowSampleE;
		Float32 iirHighSample;
		Float32 iirLowSample;
		
		Float32 tripletA;
		Float32 tripletB;
		Float32 tripletC;
		Float32 tripletFactor;
		
		Float32 lowpassSampleAA;
		Float32 lowpassSampleAB;
		Float32 lowpassSampleBA;
		Float32 lowpassSampleBB;
		Float32 lowpassSampleCA;
		Float32 lowpassSampleCB;
		Float32 lowpassSampleDA;
		Float32 lowpassSampleDB;
		Float32 lowpassSampleE;
		Float32 lowpassSampleF;
		Float32 lowpassSampleG;
		
		Float32 highpassSampleAA;
		Float32 highpassSampleAB;
		Float32 highpassSampleBA;
		Float32 highpassSampleBB;
		Float32 highpassSampleCA;
		Float32 highpassSampleCB;
		Float32 highpassSampleDA;
		Float32 highpassSampleDB;
		Float32 highpassSampleE;
		Float32 highpassSampleF;
		
		bool flip;
		int flipthree;
		
		Float32 lastSample;
		Float32 last2Sample;

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
	Float32 overallscale = GetSampleRate();
	Float32 inputSample;
	Float32 highSample = 0.0f;
	Float32 midSample = 0.0f;
	Float32 bassSample = 0.0f;
	Float32 densityA = GetParameter( kParam_One )/2.0f;
	Float32 densityB = GetParameter( kParam_Two )/2.0f;
	Float32 densityC = GetParameter( kParam_Three )/2.0f;

	densityA = powf(10.0f,densityA/20.0f)-1.0f;
	densityB = powf(10.0f,densityB/20.0f)-1.0f;
	densityC = powf(10.0f,densityC/20.0f)-1.0f;
	//convert to 0 to X multiplier with 1.0f being O db
	//minus one gives nearly -1 to ? (should top out at 1)
	//calibrate so that X db roughly equals X db with maximum topping out at 1 internally
	
	Float32 tripletIntensity = -densityA;
	
	Float32 iirAmountC = (GetParameter( kParam_Four )*0.0188f) + 0.7f;
	if (iirAmountC > 1.0f) iirAmountC = 1.0f;
	bool engageLowpass = false;
	if (GetParameter( kParam_Four ) < 15.99f) engageLowpass = true;
	
	Float32 iirAmountA = (GetParameter( kParam_Five )*1000)/overallscale;
	Float32 iirAmountB = (GetParameter( kParam_Six )*10)/overallscale;
	Float32 iirAmountD = (GetParameter( kParam_Seven )*1.0f)/overallscale;
	bool engageHighpass = false;
	if (GetParameter( kParam_Seven ) > 30.01f) engageHighpass = true;
	//bypass the highpass and lowpass if set to extremes
	Float32 bridgerectifier;
	Float32 outA = fabs(densityA);
	Float32 outB = fabs(densityB);
	Float32 outC = fabs(densityC);
	Float32 outputgain = powf(10.0f,GetParameter( kParam_Eight )/20.0f);
	
	
	
	while (nSampleFrames-- > 0) {
		inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		
		last2Sample = lastSample;
		lastSample = inputSample;
		flip = !flip;
		flipthree++;
		if (flipthree < 1 || flipthree > 3) flipthree = 1;
		//counters
		
		switch (flipthree)
		{
			case 1:
				tripletFactor = last2Sample - inputSample;
				tripletA += tripletFactor;
				tripletC -= tripletFactor;
				tripletFactor = tripletA * tripletIntensity;
				iirHighSampleC = (iirHighSampleC * (1 - iirAmountA)) + (inputSample * iirAmountA);
				highSample = inputSample - iirHighSampleC;
				iirLowSampleC = (iirLowSampleC * (1 - iirAmountB)) + (inputSample * iirAmountB);
				bassSample = iirLowSampleC;
				break;
			case 2:
				tripletFactor = last2Sample - inputSample;
				tripletB += tripletFactor;
				tripletA -= tripletFactor;
				tripletFactor = tripletB * tripletIntensity;
				iirHighSampleD = (iirHighSampleD * (1 - iirAmountA)) + (inputSample * iirAmountA);
				highSample = inputSample - iirHighSampleD;
				iirLowSampleD = (iirLowSampleD * (1 - iirAmountB)) + (inputSample * iirAmountB);
				bassSample = iirLowSampleD;
				break;
			case 3:
				tripletFactor = last2Sample - inputSample;
				tripletC += tripletFactor;
				tripletB -= tripletFactor;
				tripletFactor = tripletC * tripletIntensity;
				iirHighSampleE = (iirHighSampleE * (1 - iirAmountA)) + (inputSample * iirAmountA);
				highSample = inputSample - iirHighSampleE;
				iirLowSampleE = (iirLowSampleE * (1 - iirAmountB)) + (inputSample * iirAmountB);
				bassSample = iirLowSampleE;
				break;
		}
		tripletA /= 2.0f;
		tripletB /= 2.0f;
		tripletC /= 2.0f;
		highSample = highSample + tripletFactor;
		
		if (flip)
		{
			iirHighSampleA = (iirHighSampleA * (1 - iirAmountA)) + (highSample * iirAmountA);
			highSample = highSample - iirHighSampleA;
			iirLowSampleA = (iirLowSampleA * (1 - iirAmountB)) + (bassSample * iirAmountB);
			bassSample = iirLowSampleA;
		}
		else
		{
			iirHighSampleB = (iirHighSampleB * (1 - iirAmountA)) + (highSample * iirAmountA);
			highSample = highSample - iirHighSampleB;
			iirLowSampleB = (iirLowSampleB * (1 - iirAmountB)) + (bassSample * iirAmountB);
			bassSample = iirLowSampleB;
		}
		iirHighSample = (iirHighSample * (1 - iirAmountA)) + (highSample * iirAmountA);
		highSample = highSample - iirHighSample;
		iirLowSample = (iirLowSample * (1 - iirAmountB)) + (bassSample * iirAmountB);
		bassSample = iirLowSample;
		
		midSample = (inputSample-bassSample)-highSample;

		//drive section
		highSample *= (densityA+1.0f);
		bridgerectifier = fabs(highSample)*1.57079633f;
		if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
		//max value for sine function
		if (densityA > 0) bridgerectifier = sin(bridgerectifier);
		else bridgerectifier = 1-cos(bridgerectifier);
		//produce either boosted or starved version
		if (highSample > 0) highSample = (highSample*(1-outA))+(bridgerectifier*outA);
		else highSample = (highSample*(1-outA))-(bridgerectifier*outA);
		//blend according to densityA control
		
		midSample *= (densityB+1.0f);
		bridgerectifier = fabs(midSample)*1.57079633f;
		if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
		//max value for sine function
		if (densityB > 0) bridgerectifier = sin(bridgerectifier);
		else bridgerectifier = 1-cos(bridgerectifier);
		//produce either boosted or starved version
		if (midSample > 0) midSample = (midSample*(1-outB))+(bridgerectifier*outB);
		else midSample = (midSample*(1-outB))-(bridgerectifier*outB);
		//blend according to densityB control
		
		bassSample *= (densityC+1.0f);
		bridgerectifier = fabs(bassSample)*1.57079633f;
		if (bridgerectifier > 1.57079633f) bridgerectifier = 1.57079633f;
		//max value for sine function
		if (densityC > 0) bridgerectifier = sin(bridgerectifier);
		else bridgerectifier = 1-cos(bridgerectifier);
		//produce either boosted or starved version
		if (bassSample > 0) bassSample = (bassSample*(1-outC))+(bridgerectifier*outC);
		else bassSample = (bassSample*(1-outC))-(bridgerectifier*outC);
		//blend according to densityC control

		inputSample = midSample;
		inputSample += highSample;
		inputSample += bassSample;
		
		if (engageHighpass)
		{
			if (flip)
			{
				highpassSampleAA = (highpassSampleAA * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleAA;
				highpassSampleBA = (highpassSampleBA * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleBA;
				highpassSampleCA = (highpassSampleCA * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleCA;
				highpassSampleDA = (highpassSampleDA * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleDA;
			}
			else
			{
				highpassSampleAB = (highpassSampleAB * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleAB;
				highpassSampleBB = (highpassSampleBB * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleBB;
				highpassSampleCB = (highpassSampleCB * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleCB;
				highpassSampleDB = (highpassSampleDB * (1 - iirAmountD)) + (inputSample * iirAmountD);
				inputSample = inputSample - highpassSampleDB;
			}
			highpassSampleE = (highpassSampleE * (1 - iirAmountD)) + (inputSample * iirAmountD);
			inputSample = inputSample - highpassSampleE;
			highpassSampleF = (highpassSampleF * (1 - iirAmountD)) + (inputSample * iirAmountD);
			inputSample = inputSample - highpassSampleF;			
			
		}
		
		if (engageLowpass)
		{
			if (flip)
			{
				lowpassSampleAA = (lowpassSampleAA * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleAA;
				lowpassSampleBA = (lowpassSampleBA * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleBA;
				lowpassSampleCA = (lowpassSampleCA * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleCA;
				lowpassSampleDA = (lowpassSampleDA * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleDA;
				lowpassSampleE = (lowpassSampleE * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleE;
			}
			else
			{
				lowpassSampleAB = (lowpassSampleAB * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleAB;
				lowpassSampleBB = (lowpassSampleBB * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleBB;
				lowpassSampleCB = (lowpassSampleCB * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleCB;
				lowpassSampleDB = (lowpassSampleDB * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleDB;
				lowpassSampleF = (lowpassSampleF * (1 - iirAmountC)) + (inputSample * iirAmountC);
				inputSample = lowpassSampleF;			
			}
			lowpassSampleG = (lowpassSampleG * (1 - iirAmountC)) + (inputSample * iirAmountC);
			inputSample = (lowpassSampleG * (1 - iirAmountC)) + (inputSample * iirAmountC);
		}
		
		//built in output trim and dry/wet if desired
		if (outputgain != 1.0f) inputSample *= outputgain;
		
		
	
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;

	iirHighSampleA = 0.0;
	iirHighSampleB = 0.0;
	iirHighSampleC = 0.0;
	iirHighSampleD = 0.0;
	iirHighSampleE = 0.0;
	iirLowSampleA = 0.0;
	iirLowSampleB = 0.0;
	iirLowSampleC = 0.0;
	iirLowSampleD = 0.0;
	iirLowSampleE = 0.0;
	iirHighSample = 0.0;
	iirLowSample = 0.0;
	tripletA = 0.0;
	tripletB = 0.0;
	tripletC = 0.0;
	tripletFactor = 0.0;
	flip = false;
	flipthree = 0;

	lowpassSampleAA = 0.0;
	lowpassSampleAB = 0.0;
	lowpassSampleBA = 0.0;
	lowpassSampleBB = 0.0;
	lowpassSampleCA = 0.0;
	lowpassSampleCB = 0.0;
	lowpassSampleDA = 0.0;
	lowpassSampleDB = 0.0;
	lowpassSampleE = 0.0;
	lowpassSampleF = 0.0;
	lowpassSampleG = 0.0;
	
	highpassSampleAA = 0.0;
	highpassSampleAB = 0.0;
	highpassSampleBA = 0.0;
	highpassSampleBB = 0.0;
	highpassSampleCA = 0.0;
	highpassSampleCB = 0.0;
	highpassSampleDA = 0.0;
	highpassSampleDB = 0.0;
	highpassSampleE = 0.0;
	highpassSampleF = 0.0;
	
	lastSample = 0.0;
	last2Sample = 0.0;
}
};
