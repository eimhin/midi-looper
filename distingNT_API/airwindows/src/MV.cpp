#include <math.h>
#include <new>
#include <distingnt/api.h>
#define AIRWINDOWS_NAME "MV "
#define AIRWINDOWS_DESCRIPTION "A dual-mono reverb based on BitShiftGain and the old Midiverbs."
#define AIRWINDOWS_GUID NT_MULTICHAR( 'A','M','V',' ' )
#define AIRWINDOWS_KERNELS
enum {

	kParam_One =0,
	kParam_Two =1,
	kParam_Three =2,
	kParam_Four =3,
	kParam_Five =4,
	//Add your parameters here...
	kNumberOfParameters=5
};
enum { kParamInput1, kParamOutput1, kParamOutput1mode,
kParamPrePostGain,
kParam0, kParam1, kParam2, kParam3, kParam4, };
static const uint8_t page2[] = { kParamInput1, kParamOutput1, kParamOutput1mode };
static const uint8_t page3[] = { kParamPrePostGain };
static const _NT_parameter	parameters[] = {
NT_PARAMETER_AUDIO_INPUT( "Input 1", 1, 1 )
NT_PARAMETER_AUDIO_OUTPUT_WITH_MODE( "Output 1", 1, 13 )
{ .name = "Pre/post gain", .min = -36, .max = 0, .def = -20, .unit = kNT_unitDb, .scaling = kNT_scalingNone, .enumStrings = NULL },
{ .name = "Depth", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Bright", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Regen", .min = 0, .max = 1000, .def = 500, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Output", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
{ .name = "Dry/Wet", .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling1000, .enumStrings = NULL },
};
static const uint8_t page1[] = {
kParam0, kParam1, kParam2, kParam3, kParam4, };
enum { kNumTemplateParameters = 4 };
#include "../include/template1.h"
struct _kernel {
	void render( const Float32* inSourceP, Float32* inDestP, UInt32 inFramesToProcess );
	void reset(void);
	float GetParameter( int index ) { return owner->GetParameter( index ); }
	_airwindowsAlgorithm* owner;
		
				
		int alpA, delayA;
		int alpB, delayB;
		int alpC, delayC;
		int alpD, delayD;
		int alpE, delayE;
		int alpF, delayF;
		int alpG, delayG;
		int alpH, delayH;
		int alpI, delayI;
		int alpJ, delayJ;
		int alpK, delayK;
		int alpL, delayL;
		int alpM, delayM;
		int alpN, delayN;
		int alpO, delayO;
		int alpP, delayP;
		int alpQ, delayQ;
		int alpR, delayR;
		int alpS, delayS;
		int alpT, delayT;
		int alpU, delayU;
		int alpV, delayV;
		int alpW, delayW;
		int alpX, delayX;
		int alpY, delayY;
		int alpZ, delayZ;
		
		Float32 avgA;
		Float32 avgB;
		Float32 avgC;
		Float32 avgD;
		Float32 avgE;
		Float32 avgF;
		Float32 avgG;
		Float32 avgH;
		Float32 avgI;
		Float32 avgJ;
		Float32 avgK;
		Float32 avgL;
		Float32 avgM;
		Float32 avgN;
		Float32 avgO;
		Float32 avgP;
		Float32 avgQ;
		Float32 avgR;
		Float32 avgS;
		Float32 avgT;
		Float32 avgU;
		Float32 avgV;
		Float32 avgW;
		Float32 avgX;
		Float32 avgY;
		Float32 avgZ;
		
		Float32 feedback;
		
		uint32_t fpd;
	
	struct _dram {
			Float32 aA[15150];
		Float32 aB[14618];
		Float32 aC[14358];
		Float32 aD[13818];		
		Float32 aE[13562];
		Float32 aF[13046];
		Float32 aG[11966];
		Float32 aH[11130];
		Float32 aI[10598];
		Float32 aJ[9810];
		Float32 aK[9522];
		Float32 aL[8982];
		Float32 aM[8786];
		Float32 aN[8462];
		Float32 aO[8310];
		Float32 aP[7982];
		Float32 aQ[7322];
		Float32 aR[6818];
		Float32 aS[6506];
		Float32 aT[6002];
		Float32 aU[5838];
		Float32 aV[5502];
		Float32 aW[5010];
		Float32 aX[4850];
		Float32 aY[4296];
		Float32 aZ[4180];
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
	int allpasstemp;
	Float32 avgtemp;
	int stage = GetParameter( kParam_One ) * 27.0f;
	int damp = (1.0f-GetParameter( kParam_Two )) * stage;
	Float32 feedbacklevel = GetParameter( kParam_Three );
	if (feedbacklevel <= 0.0625f) feedbacklevel = 0.0f;
	if (feedbacklevel > 0.0625f && feedbacklevel <= 0.125f) feedbacklevel = 0.0625f; //-24db
	if (feedbacklevel > 0.125f && feedbacklevel <= 0.25f) feedbacklevel = 0.125f; //-18db
	if (feedbacklevel > 0.25f && feedbacklevel <= 0.5f) feedbacklevel = 0.25f; //-12db
	if (feedbacklevel > 0.5f && feedbacklevel <= 0.99f) feedbacklevel = 0.5f; //-6db
	if (feedbacklevel > 0.99f) feedbacklevel = 1.0f;
	//we're forcing even the feedback level to be Midiverb-ized
	Float32 gain = GetParameter( kParam_Four );
	Float32 wet = GetParameter( kParam_Five );
	
	while (nSampleFrames-- > 0) {
		float inputSample = *sourceP;

		if (fabs(inputSample)<1.18e-23f) inputSample = fpd * 1.18e-17f;
		float drySample = inputSample;
		
		inputSample += feedback;
		
		inputSample = sin(inputSample);

		
		switch (stage){
			case 27:
			case 26:
				allpasstemp = alpA - 1;
				if (allpasstemp < 0 || allpasstemp > delayA) {allpasstemp = delayA;}
				inputSample -= dram->aA[allpasstemp]*0.5f;
				dram->aA[alpA] = inputSample;
				inputSample *= 0.5f;
				alpA--; if (alpA < 0 || alpA > delayA) {alpA = delayA;}
				inputSample += (dram->aA[alpA]);
				if (damp > 26) {
				avgtemp = inputSample;
				inputSample += avgA;
				inputSample *= 0.5f;
				avgA = avgtemp;
				}
				//allpass filter A		
			case 25:
				allpasstemp = alpB - 1;
				if (allpasstemp < 0 || allpasstemp > delayB) {allpasstemp = delayB;}
				inputSample -= dram->aB[allpasstemp]*0.5f;
				dram->aB[alpB] = inputSample;
				inputSample *= 0.5f;
				alpB--; if (alpB < 0 || alpB > delayB) {alpB = delayB;}
				inputSample += (dram->aB[alpB]);
				if (damp > 25) {
				avgtemp = inputSample;
				inputSample += avgB;
				inputSample *= 0.5f;
				avgB = avgtemp;
				}
				//allpass filter B
			case 24:
				allpasstemp = alpC - 1;
				if (allpasstemp < 0 || allpasstemp > delayC) {allpasstemp = delayC;}
				inputSample -= dram->aC[allpasstemp]*0.5f;
				dram->aC[alpC] = inputSample;
				inputSample *= 0.5f;
				alpC--; if (alpC < 0 || alpC > delayC) {alpC = delayC;}
				inputSample += (dram->aC[alpC]);
				if (damp > 24) {
				avgtemp = inputSample;
				inputSample += avgC;
				inputSample *= 0.5f;
				avgC = avgtemp;
				}
				//allpass filter C
			case 23:
				allpasstemp = alpD - 1;
				if (allpasstemp < 0 || allpasstemp > delayD) {allpasstemp = delayD;}
				inputSample -= dram->aD[allpasstemp]*0.5f;
				dram->aD[alpD] = inputSample;
				inputSample *= 0.5f;
				alpD--; if (alpD < 0 || alpD > delayD) {alpD = delayD;}
				inputSample += (dram->aD[alpD]);
				if (damp > 23) {
				avgtemp = inputSample;
				inputSample += avgD;
				inputSample *= 0.5f;
				avgD = avgtemp;
				}
				//allpass filter D
			case 22:
				allpasstemp = alpE - 1;
				if (allpasstemp < 0 || allpasstemp > delayE) {allpasstemp = delayE;}
				inputSample -= dram->aE[allpasstemp]*0.5f;
				dram->aE[alpE] = inputSample;
				inputSample *= 0.5f;
				alpE--; if (alpE < 0 || alpE > delayE) {alpE = delayE;}
				inputSample += (dram->aE[alpE]);
				if (damp > 22) {
				avgtemp = inputSample;
				inputSample += avgE;
				inputSample *= 0.5f;
				avgE = avgtemp;
				}
				//allpass filter E
			case 21:
				allpasstemp = alpF - 1;
				if (allpasstemp < 0 || allpasstemp > delayF) {allpasstemp = delayF;}
				inputSample -= dram->aF[allpasstemp]*0.5f;
				dram->aF[alpF] = inputSample;
				inputSample *= 0.5f;
				alpF--; if (alpF < 0 || alpF > delayF) {alpF = delayF;}
				inputSample += (dram->aF[alpF]);
				if (damp > 21) {
				avgtemp = inputSample;
				inputSample += avgF;
				inputSample *= 0.5f;
				avgF = avgtemp;
				}
				//allpass filter F
			case 20:
				allpasstemp = alpG - 1;
				if (allpasstemp < 0 || allpasstemp > delayG) {allpasstemp = delayG;}
				inputSample -= dram->aG[allpasstemp]*0.5f;
				dram->aG[alpG] = inputSample;
				inputSample *= 0.5f;
				alpG--; if (alpG < 0 || alpG > delayG) {alpG = delayG;}
				inputSample += (dram->aG[alpG]);
				if (damp > 20) {
				avgtemp = inputSample;
				inputSample += avgG;
				inputSample *= 0.5f;
				avgG = avgtemp;
				}
				//allpass filter G
			case 19:
				allpasstemp = alpH - 1;
				if (allpasstemp < 0 || allpasstemp > delayH) {allpasstemp = delayH;}
				inputSample -= dram->aH[allpasstemp]*0.5f;
				dram->aH[alpH] = inputSample;
				inputSample *= 0.5f;
				alpH--; if (alpH < 0 || alpH > delayH) {alpH = delayH;}
				inputSample += (dram->aH[alpH]);
				if (damp > 19) {
				avgtemp = inputSample;
				inputSample += avgH;
				inputSample *= 0.5f;
				avgH = avgtemp;
				}
				//allpass filter H
			case 18:
				allpasstemp = alpI - 1;
				if (allpasstemp < 0 || allpasstemp > delayI) {allpasstemp = delayI;}
				inputSample -= dram->aI[allpasstemp]*0.5f;
				dram->aI[alpI] = inputSample;
				inputSample *= 0.5f;
				alpI--; if (alpI < 0 || alpI > delayI) {alpI = delayI;}
				inputSample += (dram->aI[alpI]);
				if (damp > 18) {
				avgtemp = inputSample;
				inputSample += avgI;
				inputSample *= 0.5f;
				avgI = avgtemp;
				}
				//allpass filter I
			case 17:
				allpasstemp = alpJ - 1;
				if (allpasstemp < 0 || allpasstemp > delayJ) {allpasstemp = delayJ;}
				inputSample -= dram->aJ[allpasstemp]*0.5f;
				dram->aJ[alpJ] = inputSample;
				inputSample *= 0.5f;
				alpJ--; if (alpJ < 0 || alpJ > delayJ) {alpJ = delayJ;}
				inputSample += (dram->aJ[alpJ]);
				if (damp > 17) {
				avgtemp = inputSample;
				inputSample += avgJ;
				inputSample *= 0.5f;
				avgJ = avgtemp;
				}
				//allpass filter J
			case 16:
				allpasstemp = alpK - 1;
				if (allpasstemp < 0 || allpasstemp > delayK) {allpasstemp = delayK;}
				inputSample -= dram->aK[allpasstemp]*0.5f;
				dram->aK[alpK] = inputSample;
				inputSample *= 0.5f;
				alpK--; if (alpK < 0 || alpK > delayK) {alpK = delayK;}
				inputSample += (dram->aK[alpK]);
				if (damp > 16) {
				avgtemp = inputSample;
				inputSample += avgK;
				inputSample *= 0.5f;
				avgK = avgtemp;
				}
				//allpass filter K
			case 15:
				allpasstemp = alpL - 1;
				if (allpasstemp < 0 || allpasstemp > delayL) {allpasstemp = delayL;}
				inputSample -= dram->aL[allpasstemp]*0.5f;
				dram->aL[alpL] = inputSample;
				inputSample *= 0.5f;
				alpL--; if (alpL < 0 || alpL > delayL) {alpL = delayL;}
				inputSample += (dram->aL[alpL]);
				if (damp > 15) {
				avgtemp = inputSample;
				inputSample += avgL;
				inputSample *= 0.5f;
				avgL = avgtemp;
				}
				//allpass filter L
			case 14:
				allpasstemp = alpM - 1;
				if (allpasstemp < 0 || allpasstemp > delayM) {allpasstemp = delayM;}
				inputSample -= dram->aM[allpasstemp]*0.5f;
				dram->aM[alpM] = inputSample;
				inputSample *= 0.5f;
				alpM--; if (alpM < 0 || alpM > delayM) {alpM = delayM;}
				inputSample += (dram->aM[alpM]);
				if (damp > 14) {
				avgtemp = inputSample;
				inputSample += avgM;
				inputSample *= 0.5f;
				avgM = avgtemp;
				}
				//allpass filter M
			case 13:
				allpasstemp = alpN - 1;
				if (allpasstemp < 0 || allpasstemp > delayN) {allpasstemp = delayN;}
				inputSample -= dram->aN[allpasstemp]*0.5f;
				dram->aN[alpN] = inputSample;
				inputSample *= 0.5f;
				alpN--; if (alpN < 0 || alpN > delayN) {alpN = delayN;}
				inputSample += (dram->aN[alpN]);
				if (damp > 13) {
				avgtemp = inputSample;
				inputSample += avgN;
				inputSample *= 0.5f;
				avgN = avgtemp;
				}
				//allpass filter N
			case 12:
				allpasstemp = alpO - 1;
				if (allpasstemp < 0 || allpasstemp > delayO) {allpasstemp = delayO;}
				inputSample -= dram->aO[allpasstemp]*0.5f;
				dram->aO[alpO] = inputSample;
				inputSample *= 0.5f;
				alpO--; if (alpO < 0 || alpO > delayO) {alpO = delayO;}
				inputSample += (dram->aO[alpO]);
				if (damp > 12) {
				avgtemp = inputSample;
				inputSample += avgO;
				inputSample *= 0.5f;
				avgO = avgtemp;
				}
				//allpass filter O
			case 11:
				allpasstemp = alpP - 1;
				if (allpasstemp < 0 || allpasstemp > delayP) {allpasstemp = delayP;}
				inputSample -= dram->aP[allpasstemp]*0.5f;
				dram->aP[alpP] = inputSample;
				inputSample *= 0.5f;
				alpP--; if (alpP < 0 || alpP > delayP) {alpP = delayP;}
				inputSample += (dram->aP[alpP]);
				if (damp > 11) {
				avgtemp = inputSample;
				inputSample += avgP;
				inputSample *= 0.5f;
				avgP = avgtemp;
				}
				//allpass filter P
			case 10:
				allpasstemp = alpQ - 1;
				if (allpasstemp < 0 || allpasstemp > delayQ) {allpasstemp = delayQ;}
				inputSample -= dram->aQ[allpasstemp]*0.5f;
				dram->aQ[alpQ] = inputSample;
				inputSample *= 0.5f;
				alpQ--; if (alpQ < 0 || alpQ > delayQ) {alpQ = delayQ;}
				inputSample += (dram->aQ[alpQ]);
				if (damp > 10) {
				avgtemp = inputSample;
				inputSample += avgQ;
				inputSample *= 0.5f;
				avgQ = avgtemp;
				}
				//allpass filter Q
			case 9:
				allpasstemp = alpR - 1;
				if (allpasstemp < 0 || allpasstemp > delayR) {allpasstemp = delayR;}
				inputSample -= dram->aR[allpasstemp]*0.5f;
				dram->aR[alpR] = inputSample;
				inputSample *= 0.5f;
				alpR--; if (alpR < 0 || alpR > delayR) {alpR = delayR;}
				inputSample += (dram->aR[alpR]);
				if (damp > 9) {
				avgtemp = inputSample;
				inputSample += avgR;
				inputSample *= 0.5f;
				avgR = avgtemp;
				}
				//allpass filter R
			case 8:
				allpasstemp = alpS - 1;
				if (allpasstemp < 0 || allpasstemp > delayS) {allpasstemp = delayS;}
				inputSample -= dram->aS[allpasstemp]*0.5f;
				dram->aS[alpS] = inputSample;
				inputSample *= 0.5f;
				alpS--; if (alpS < 0 || alpS > delayS) {alpS = delayS;}
				inputSample += (dram->aS[alpS]);
				if (damp > 8) {
				avgtemp = inputSample;
				inputSample += avgS;
				inputSample *= 0.5f;
				avgS = avgtemp;
				}
				//allpass filter S
			case 7:
				allpasstemp = alpT - 1;
				if (allpasstemp < 0 || allpasstemp > delayT) {allpasstemp = delayT;}
				inputSample -= dram->aT[allpasstemp]*0.5f;
				dram->aT[alpT] = inputSample;
				inputSample *= 0.5f;
				alpT--; if (alpT < 0 || alpT > delayT) {alpT = delayT;}
				inputSample += (dram->aT[alpT]);
				if (damp > 7) {
				avgtemp = inputSample;
				inputSample += avgT;
				inputSample *= 0.5f;
				avgT = avgtemp;
				}
				//allpass filter T
			case 6:
				allpasstemp = alpU - 1;
				if (allpasstemp < 0 || allpasstemp > delayU) {allpasstemp = delayU;}
				inputSample -= dram->aU[allpasstemp]*0.5f;
				dram->aU[alpU] = inputSample;
				inputSample *= 0.5f;
				alpU--; if (alpU < 0 || alpU > delayU) {alpU = delayU;}
				inputSample += (dram->aU[alpU]);
				if (damp > 6) {
				avgtemp = inputSample;
				inputSample += avgU;
				inputSample *= 0.5f;
				avgU = avgtemp;
				}
				//allpass filter U
			case 5:
				allpasstemp = alpV - 1;
				if (allpasstemp < 0 || allpasstemp > delayV) {allpasstemp = delayV;}
				inputSample -= dram->aV[allpasstemp]*0.5f;
				dram->aV[alpV] = inputSample;
				inputSample *= 0.5f;
				alpV--; if (alpV < 0 || alpV > delayV) {alpV = delayV;}
				inputSample += (dram->aV[alpV]);
				if (damp > 5) {
				avgtemp = inputSample;
				inputSample += avgV;
				inputSample *= 0.5f;
				avgV = avgtemp;
				}
				//allpass filter V
			case 4:
				allpasstemp = alpW - 1;
				if (allpasstemp < 0 || allpasstemp > delayW) {allpasstemp = delayW;}
				inputSample -= dram->aW[allpasstemp]*0.5f;
				dram->aW[alpW] = inputSample;
				inputSample *= 0.5f;
				alpW--; if (alpW < 0 || alpW > delayW) {alpW = delayW;}
				inputSample += (dram->aW[alpW]);
				if (damp > 4) {
				avgtemp = inputSample;
				inputSample += avgW;
				inputSample *= 0.5f;
				avgW = avgtemp;
				}
				//allpass filter W
			case 3:
				allpasstemp = alpX - 1;
				if (allpasstemp < 0 || allpasstemp > delayX) {allpasstemp = delayX;}
				inputSample -= dram->aX[allpasstemp]*0.5f;
				dram->aX[alpX] = inputSample;
				inputSample *= 0.5f;
				alpX--; if (alpX < 0 || alpX > delayX) {alpX = delayX;}
				inputSample += (dram->aX[alpX]);
				if (damp > 3) {
				avgtemp = inputSample;
				inputSample += avgX;
				inputSample *= 0.5f;
				avgX = avgtemp;
				}
				//allpass filter X
			case 2:
				allpasstemp = alpY - 1;
				if (allpasstemp < 0 || allpasstemp > delayY) {allpasstemp = delayY;}
				inputSample -= dram->aY[allpasstemp]*0.5f;
				dram->aY[alpY] = inputSample;
				inputSample *= 0.5f;
				alpY--; if (alpY < 0 || alpY > delayY) {alpY = delayY;}
				inputSample += (dram->aY[alpY]);
				if (damp > 2) {
				avgtemp = inputSample;
				inputSample += avgY;
				inputSample *= 0.5f;
				avgY = avgtemp;
				}
				//allpass filter Y
			case 1:
				allpasstemp = alpZ - 1;
				if (allpasstemp < 0 || allpasstemp > delayZ) {allpasstemp = delayZ;}
				inputSample -= dram->aZ[allpasstemp]*0.5f;
				dram->aZ[alpZ] = inputSample;
				inputSample *= 0.5f;
				alpZ--; if (alpZ < 0 || alpZ > delayZ) {alpZ = delayZ;}
				inputSample += (dram->aZ[alpZ]);
				if (damp > 1) {
				avgtemp = inputSample;
				inputSample += avgZ;
				inputSample *= 0.5f;
				avgZ = avgtemp;
				}
				//allpass filter Z
		}
		
		feedback = inputSample * feedbacklevel;
				
		if (gain != 1.0f) {
			inputSample *= gain;
		}
		//we can pad with the gain to tame distortyness from the PurestConsole code
		
		if (inputSample > 1.0f) inputSample = 1.0f;
		if (inputSample < -1.0f) inputSample = -1.0f;
		//without this, you can get a NaN condition where it spits out DC offset at full blast!
		
		inputSample = asin(inputSample);
		
		
		if (wet !=1.0f) {
			inputSample = (inputSample * wet) + (drySample * (1.0f-wet));
		}
		//Dry/Wet control, defaults to the last slider

		
		
		*destP = inputSample;
		
		sourceP += inNumChannels; destP += inNumChannels;
	}
}
}
void _airwindowsAlgorithm::_kernel::reset(void) {
{
	int count;
	for(count = 0; count < 15149; count++) {dram->aA[count] = 0.0;}
	for(count = 0; count < 14617; count++) {dram->aB[count] = 0.0;}
	for(count = 0; count < 14357; count++) {dram->aC[count] = 0.0;}
	for(count = 0; count < 13817; count++) {dram->aD[count] = 0.0;}
	for(count = 0; count < 13561; count++) {dram->aE[count] = 0.0;}
	for(count = 0; count < 13045; count++) {dram->aF[count] = 0.0;}
	for(count = 0; count < 11965; count++) {dram->aG[count] = 0.0;}
	for(count = 0; count < 11129; count++) {dram->aH[count] = 0.0;}
	for(count = 0; count < 10597; count++) {dram->aI[count] = 0.0;}
	for(count = 0; count < 9809; count++) {dram->aJ[count] = 0.0;}
	for(count = 0; count < 9521; count++) {dram->aK[count] = 0.0;}
	for(count = 0; count < 8981; count++) {dram->aL[count] = 0.0;}
	for(count = 0; count < 8785; count++) {dram->aM[count] = 0.0;}
	for(count = 0; count < 8461; count++) {dram->aN[count] = 0.0;}
	for(count = 0; count < 8309; count++) {dram->aO[count] = 0.0;}
	for(count = 0; count < 7981; count++) {dram->aP[count] = 0.0;}
	for(count = 0; count < 7321; count++) {dram->aQ[count] = 0.0;}
	for(count = 0; count < 6817; count++) {dram->aR[count] = 0.0;}
	for(count = 0; count < 6505; count++) {dram->aS[count] = 0.0;}
	for(count = 0; count < 6001; count++) {dram->aT[count] = 0.0;}
	for(count = 0; count < 5837; count++) {dram->aU[count] = 0.0;}
	for(count = 0; count < 5501; count++) {dram->aV[count] = 0.0;}
	for(count = 0; count < 5009; count++) {dram->aW[count] = 0.0;}
	for(count = 0; count < 4849; count++) {dram->aX[count] = 0.0;}
	for(count = 0; count < 4295; count++) {dram->aY[count] = 0.0;}
	for(count = 0; count < 4179; count++) {dram->aZ[count] = 0.0;}	
	
	alpA = 1; delayA = 7573; avgA = 0.0;
	alpB = 1; delayB = 7307; avgB = 0.0;
	alpC = 1; delayC = 7177; avgC = 0.0;
	alpD = 1; delayD = 6907; avgD = 0.0;
	alpE = 1; delayE = 6779; avgE = 0.0;
	alpF = 1; delayF = 6521; avgF = 0.0;
	alpG = 1; delayG = 5981; avgG = 0.0;
	alpH = 1; delayH = 5563; avgH = 0.0;
	alpI = 1; delayI = 5297; avgI = 0.0;
	alpJ = 1; delayJ = 4903; avgJ = 0.0;
	alpK = 1; delayK = 4759; avgK = 0.0;
	alpL = 1; delayL = 4489; avgL = 0.0;
	alpM = 1; delayM = 4391; avgM = 0.0;
	alpN = 1; delayN = 4229; avgN = 0.0;
	alpO = 1; delayO = 4153; avgO = 0.0;
	alpP = 1; delayP = 3989; avgP = 0.0;
	alpQ = 1; delayQ = 3659; avgQ = 0.0;
	alpR = 1; delayR = 3407; avgR = 0.0;
	alpS = 1; delayS = 3251; avgS = 0.0;
	alpT = 1; delayT = 2999; avgT = 0.0;
	alpU = 1; delayU = 2917; avgU = 0.0;
	alpV = 1; delayV = 2749; avgV = 0.0;
	alpW = 1; delayW = 2503; avgW = 0.0;
	alpX = 1; delayX = 2423; avgX = 0.0;
	alpY = 1; delayY = 2146; avgY = 0.0;
	alpZ = 1; delayZ = 2088; avgZ = 0.0;
	
	feedback = 0.0;
	
	fpd = 1.0; while (fpd < 16386) fpd = rand()*UINT32_MAX;
}
};
