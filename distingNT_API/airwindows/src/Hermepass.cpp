#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Hermepass"
#define AIRWINDOWS_DESCRIPTION "A mastering highpass to set by ear only."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','H','e','s' )
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
{ .name = "Cutoff", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Slope", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
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

		
		Float32 iirA;
		Float32 iirB; //first stage is the flipping one, for lowest slope. It is always engaged, and is the highest one
		Float32 iirC; //we introduce the second pole at the same frequency, to become a pseudo-Capacitor behavior
		Float32 iirD;
		Float32 iirE;
		Float32 iirF; //our slope control will have a powf() on it so that the high orders are way to the right side
		Float32 iirG;
		Float32 iirH; //seven poles max, and the final pole is always at 20hz directly.
		
		
		uint32_t fpd;
		bool fpFlip;
	
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
	float overallscale = 1.0f;
	overallscale /= 44100.0f;
	overallscale *= GetSampleRate();
	float fpOld = 0.618033988749894848204586f; //golden ratio!
	float fpNew = 1.0f - fpOld;

	Float32 rangescale = 0.1f / overallscale;
	
	Float32 cutoff = powf(GetParameter( kParam_One ),3);
	Float32 slope = powf(GetParameter( kParam_Two ),3) * 6.0f;
	
	Float32 newA = cutoff * rangescale;
	Float32 newB = newA; //other part of interleaved IIR is the same
	
	Float32 newC = cutoff * rangescale; //first extra pole is the same
	cutoff = (cutoff * fpOld) + (0.00001f * fpNew);
	Float32 newD = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * fpNew);
	Float32 newE = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * fpNew);
	Float32 newF = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * fpNew);
	Float32 newG = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * fpNew);
	Float32 newH = cutoff * rangescale;
	//converge toward the unvarying fixed cutoff value
	
	Float32 oldA = 1.0f - newA;
	Float32 oldB = 1.0f - newB;
	Float32 oldC = 1.0f - newC;
	Float32 oldD = 1.0f - newD;
	Float32 oldE = 1.0f - newE;
	Float32 oldF = 1.0f - newF;
	Float32 oldG = 1.0f - newG;
	Float32 oldH = 1.0f - newH;
	
	Float32 polesC;
	Float32 polesD;
	Float32 polesE;
	Float32 polesF;
	Float32 polesG;
	Float32 polesH;
	
	polesC = slope; if (slope > 1.0f) polesC = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	polesD = slope; if (slope > 1.0f) polesD = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	polesE = slope; if (slope > 1.0f) polesE = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	polesF = slope; if (slope > 1.0f) polesF = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	polesG = slope; if (slope > 1.0f) polesG = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	polesH = slope; if (slope > 1.0f) polesH = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	//each one will either be 0.0f, the fractional slope value, or 1
	
	float inputSample;
	Float32 tempSample;
	Float32 correction;
	
	while (nSampleFrames-- > 0) {
		inputSample = *sourceP;
		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		tempSample = inputSample;

		if (fpFlip) {
			iirA = (iirA * oldA) + (tempSample * newA); tempSample -= iirA; correction = iirA;
		} else {
			iirB = (iirB * oldB) + (tempSample * newB); tempSample -= iirB; correction = iirB;
		}
		iirC = (iirC * oldC) + (tempSample * newC); tempSample -= iirC;
		iirD = (iirD * oldD) + (tempSample * newD); tempSample -= iirD;
		iirE = (iirE * oldE) + (tempSample * newE); tempSample -= iirE;
		iirF = (iirF * oldF) + (tempSample * newF); tempSample -= iirF;
		iirG = (iirG * oldG) + (tempSample * newG); tempSample -= iirG;
		iirH = (iirH * oldH) + (tempSample * newH); tempSample -= iirH;
		//set up all the iir filters in case they are used
		
		if (polesC == 1.0f) correction += iirC; if (polesC > 0.0f && polesC < 1.0f) correction += (iirC * polesC);
		if (polesD == 1.0f) correction += iirD; if (polesD > 0.0f && polesD < 1.0f) correction += (iirD * polesD);
		if (polesE == 1.0f) correction += iirE; if (polesE > 0.0f && polesE < 1.0f) correction += (iirE * polesE);
		if (polesF == 1.0f) correction += iirF; if (polesF > 0.0f && polesF < 1.0f) correction += (iirF * polesF);
		if (polesG == 1.0f) correction += iirG; if (polesG > 0.0f && polesG < 1.0f) correction += (iirG * polesG);
		if (polesH == 1.0f) correction += iirH; if (polesH > 0.0f && polesH < 1.0f) correction += (iirH * polesH);
		//each of these are added directly if they're fully engaged,
		//multiplied by 0-1 if they are the interpolated one, or skipped if they are beyond the interpolated one.
		//the purpose is to do all the math at the floating point exponent nearest to the tiny value in use.
		//also, it's formatted that way to easily substitute the next variable: this could be written as a loop
		//with everything an array value. However, this makes just as much sense for this few poles.
		
		inputSample -= correction;
		fpFlip = !fpFlip;
	
		
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	iirA = 0.0;
	iirB = 0.0;
	iirC = 0.0;
	iirD = 0.0;
	iirE = 0.0;
	iirF = 0.0;
	iirG = 0.0;
	iirH = 0.0;
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
	fpFlip = true;
}
};
