#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "Elliptical"
#define AIRWINDOWS_DESCRIPTION "Highpasses the side channel."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','E','l','l' )
enum {

	kParam_A =0,
	kParam_B =1,
	//Add your parameters here...
	kNumberOfParameters=2
};
enum { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode,
kParamPrePostGain,
kParam0, kParam1, };
static const uint8_t page2[] = { kParamInputL, kParamInputR, kParamOutputL, kParamOutputLmode, kParamOutputR, kParamOutputRmode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input L", 1, 1 )
NT_PARAMETER_AUDIO_INPUT( "Input R", 1, 2 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output L", 1, 13 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output R", 1, 14 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Cutoff", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Slope", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, };
enum { kNumTemplateParameters = 7 };
#include "../include/template1.h"
 
	float iirA;
	float iirB; //first stage is the flipping one, for lowest slope. It is always engaged, and is the highest one
	float iirC; //we introduce the second pole at the same frequency, to become a pseudo-Capacitor behavior
	float iirD;
	float iirE;
	float iirF; //our slope control will have a powf() on it so that the high orders are way to the right side
	float iirG;
	float iirH; //seven poles max, and the final pole is always at 20hz directly.
	bool fpFlip;
	
	uint32_t fpdL;
	uint32_t fpdR;

	struct _dram {
		};
	_dram* dram;
#include "../include/template2.h"
#include "../include/templateStereo.h"
void _airwindowsAlgorithm::render( const Float32* inputL, const Float32* inputR, Float32* outputL, Float32* outputR, UInt32 inFramesToProcess ) {

	UInt32 nSampleFrames = inFramesToProcess;
	float overallscale = 1.0f;
	overallscale /= 44100.0f;
	overallscale *= GetSampleRate();
	
	float rangescale = 0.1f / overallscale;
	float cutoff = powf(GetParameter( kParam_A ),3);
	float slope = powf(GetParameter( kParam_B ),3) * 6.0f;
	float newA = cutoff * rangescale;
	float newB = newA; //other part of interleaved IIR is the same
	float fpOld = 0.618033988749894848204586f; //golden ratio!
	float newC = cutoff * rangescale; //first extra pole is the same
	cutoff = (cutoff * fpOld) + (0.00001f * (1.0f-fpOld));
	float newD = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * (1.0f-fpOld));
	float newE = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * (1.0f-fpOld));
	float newF = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * (1.0f-fpOld));
	float newG = cutoff * rangescale;
	cutoff = (cutoff * fpOld) + (0.00001f * (1.0f-fpOld));
	float newH = cutoff * rangescale;
	//converge toward the unvarying fixed cutoff value
	float polesC = slope; if (slope > 1.0f) polesC = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	float polesD = slope; if (slope > 1.0f) polesD = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	float polesE = slope; if (slope > 1.0f) polesE = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	float polesF = slope; if (slope > 1.0f) polesF = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	float polesG = slope; if (slope > 1.0f) polesG = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	float polesH = slope; if (slope > 1.0f) polesH = 1.0f; slope -= 1.0f; if (slope < 0.0f) slope = 0.0f;
	//each one will either be 0.0f, the fractional slope value, or 1
		
	while (nSampleFrames-- > 0) {
		float inputSampleL = *inputL;
		float inputSampleR = *inputR;
		if (fabs(inputSampleL)<1.18e-23f) inputSampleL = fpdL * 1.18e-17f;
		if (fabs(inputSampleR)<1.18e-23f) inputSampleR = fpdR * 1.18e-17f;
		
		float mid = inputSampleL + inputSampleR;
		float side = inputSampleL - inputSampleR;
		//assign mid and side.Between these sections, you can do mid/side processing
		float temp = side;
		float correction;
		
		if (fpFlip) {
			iirA = (iirA * (1.0f - newA)) + (temp * newA); temp -= iirA; correction = iirA;
		} else {
			iirB = (iirB * (1.0f - newB)) + (temp * newB); temp -= iirB; correction = iirB;
		}
		iirC = (iirC * (1.0f - newC)) + (temp * newC); temp -= iirC;
		iirD = (iirD * (1.0f - newD)) + (temp * newD); temp -= iirD;
		iirE = (iirE * (1.0f - newE)) + (temp * newE); temp -= iirE;
		iirF = (iirF * (1.0f - newF)) + (temp * newF); temp -= iirF;
		iirG = (iirG * (1.0f - newG)) + (temp * newG); temp -= iirG;
		iirH = (iirH * (1.0f - newH)) + (temp * newH); temp -= iirH;
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
		
		side -= correction;
		fpFlip = !fpFlip;
		
		inputSampleL = (mid+side)/2.0f;
		inputSampleR = (mid-side)/2.0f;
		//unassign mid and side
		
		
		
		*outputL = inputSampleL;
		*outputR = inputSampleR;
		//direct stereo out
		
		inputL += 1;
		inputR += 1;
		outputL += 1;
		outputR += 1;
	}
	};
int _airwindowsAlgorithm::reset(void) {

{
	iirA = 0.0;
	iirB = 0.0;
	iirC = 0.0;
	iirD = 0.0;
	iirE = 0.0;
	iirF = 0.0;
	iirG = 0.0;
	iirH = 0.0;
	fpFlip = true;
	
	fpdL = 1.0; while (fpdL < 16386) fpdL = rand()*UINT32_MAX;
	fpdR = 1.0; while (fpdR < 16386) fpdR = rand()*UINT32_MAX;
	return noErr;
}

};
