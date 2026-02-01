
#ifndef _AUEFFECTBASE_H
#define _AUEFFECTBASE_H

#include <math.h>
#include <sstream>
#include <string>
#include <vector>

const float pi = M_PI;

typedef const char* CFStringRef;
typedef std::vector<std::string> CFArrayRef;
#define CFSTR(x) x

CFArrayRef CFArrayCreate(void* allocator, const void **values, int numValues, const void *callBacks)
{
	const CFStringRef* strings = (const CFStringRef*)values;
	CFArrayRef arr;
	for ( int i=0; i<numValues; ++i )
		arr.push_back( strings[i] );
	return arr;
}

typedef int16_t SInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef uint32_t AudioUnitScope;
typedef uint32_t AudioUnitElement;
typedef uint32_t AudioUnitParameterID;
typedef uint32_t AudioUnitPropertyID;
typedef int32_t ComponentResult;
typedef int32_t OSStatus;
typedef float Float32;
typedef double Float64;
typedef bool Boolean;

#define COMPONENT_ENTRY(x)

#define noErr (0)
#define kAudioUnitErr_InvalidProperty (1)
#define kAudioUnitErr_InvalidParameter (2)

enum AudioUnitParameterOptions
{
	kAudioUnitParameterFlag_CFNameRelease		= (1UL << 4),

	kAudioUnitParameterFlag_OmitFromPresets		= (1UL << 13),
	kAudioUnitParameterFlag_PlotHistory			= (1UL << 14),
	kAudioUnitParameterFlag_MeterReadOnly		= (1UL << 15),
	
	// bit positions 18,17,16 are set aside for display scales. bit 19 is reserved.
	kAudioUnitParameterFlag_DisplayMask			= (7UL << 16) | (1UL << 22),
	kAudioUnitParameterFlag_DisplaySquareRoot	= (1UL << 16),
	kAudioUnitParameterFlag_DisplaySquared		= (2UL << 16),
	kAudioUnitParameterFlag_DisplayCubed		= (3UL << 16),
	kAudioUnitParameterFlag_DisplayCubeRoot		= (4UL << 16),
	kAudioUnitParameterFlag_DisplayExponential	= (5UL << 16),

	kAudioUnitParameterFlag_HasClump	 		= (1UL << 20),
	kAudioUnitParameterFlag_ValuesHaveStrings	= (1UL << 21),
	
	kAudioUnitParameterFlag_DisplayLogarithmic 	= (1UL << 22),
	
	kAudioUnitParameterFlag_IsHighResolution 	= (1UL << 23),
	kAudioUnitParameterFlag_NonRealTime 		= (1UL << 24),
	kAudioUnitParameterFlag_CanRamp 			= (1UL << 25),
	kAudioUnitParameterFlag_ExpertMode 			= (1UL << 26),
	kAudioUnitParameterFlag_HasCFNameString 	= (1UL << 27),
	kAudioUnitParameterFlag_IsGlobalMeta 		= (1UL << 28),
	kAudioUnitParameterFlag_IsElementMeta		= (1UL << 29),
	kAudioUnitParameterFlag_IsReadable			= (1UL << 30),
	kAudioUnitParameterFlag_IsWritable			= (1UL << 31)
};

#define kAudioUnitScope_Global (0)

typedef void* AudioUnit;

enum AudioUnitParameterUnit
{
	kAudioUnitParameterUnit_Generic				= 0,
	kAudioUnitParameterUnit_Indexed				= 1,
	kAudioUnitParameterUnit_Boolean				= 2,
	kAudioUnitParameterUnit_Percent				= 3,
	kAudioUnitParameterUnit_Seconds				= 4,
	kAudioUnitParameterUnit_SampleFrames		= 5,
	kAudioUnitParameterUnit_Phase				= 6,
	kAudioUnitParameterUnit_Rate				= 7,
	kAudioUnitParameterUnit_Hertz				= 8,
	kAudioUnitParameterUnit_Cents				= 9,
	kAudioUnitParameterUnit_RelativeSemiTones	= 10,
	kAudioUnitParameterUnit_MIDINoteNumber		= 11,
	kAudioUnitParameterUnit_MIDIController		= 12,
	kAudioUnitParameterUnit_Decibels			= 13,
	kAudioUnitParameterUnit_LinearGain			= 14,
	kAudioUnitParameterUnit_Degrees				= 15,
	kAudioUnitParameterUnit_EqualPowerCrossfade = 16,
	kAudioUnitParameterUnit_MixerFaderCurve1	= 17,
	kAudioUnitParameterUnit_Pan					= 18,
	kAudioUnitParameterUnit_Meters				= 19,
	kAudioUnitParameterUnit_AbsoluteCents		= 20,
	kAudioUnitParameterUnit_Octaves				= 21,
	kAudioUnitParameterUnit_BPM					= 22,
    kAudioUnitParameterUnit_Beats               = 23,
	kAudioUnitParameterUnit_Milliseconds		= 24,
	kAudioUnitParameterUnit_Ratio				= 25,
	kAudioUnitParameterUnit_CustomUnit			= 26,
	kAudioUnitParameterUnit_MIDI2Controller	 	= 27
};

enum AudioUnitRenderActionFlags
{
	kAudioUnitRenderAction_PreRender			= (1UL << 2),
	kAudioUnitRenderAction_PostRender			= (1UL << 3),
	kAudioUnitRenderAction_OutputIsSilence		= (1UL << 4),
	kAudioOfflineUnitRenderAction_Preflight		= (1UL << 5),
	kAudioOfflineUnitRenderAction_Render		= (1UL << 6),
	kAudioOfflineUnitRenderAction_Complete		= (1UL << 7),
	kAudioUnitRenderAction_PostRenderError		= (1UL << 8),
	kAudioUnitRenderAction_DoNotCheckRenderArgs	= (1UL << 9)
};

struct AudioUnitParameterInfo
{
	const char* name;
	const char* unitName;
	int		unit;
	float	minValue;
	float	maxValue;
	float	defaultValue;
	int		flags;
};

struct AUChannelInfo {
	SInt16		inChannels;
	SInt16		outChannels;
};
typedef struct AUChannelInfo AUChannelInfo;

struct AudioBuffer
{
    UInt32              mNumberChannels;
    UInt32              mDataByteSize;
    void* 			    mData;
};
typedef struct AudioBuffer  AudioBuffer;

struct AudioBufferList
{
    UInt32      mNumberBuffers;
    AudioBuffer mBuffers[1]; // this is a variable length array of mNumberBuffers elements
};
typedef struct AudioBufferList  AudioBufferList;

namespace AUBase
{
	void	FillInParameterName (AudioUnitParameterInfo& ioInfo, CFStringRef inName, bool inShouldRelease)
	{
		ioInfo.name = inName;
	}
};

struct AUEffectBase
{
	AUEffectBase( AudioUnit c )
	: numParameters( 0 )
	{
	}

	void			UseIndexedParameters( int n )
	{
		numParameters = n;
	}

	virtual UInt32 	SupportedNumChannels(const AUChannelInfo ** outInfo) { return 0; }

	void			CreateElements(void) {}
	
	AUEffectBase*	Globals()		{ return this; }
	
	void			SetParameter( int, float ) {}
	float			GetParameter( int ) { return 0; }

	ComponentResult	Initialize(void) { return noErr; }
	void			Reset( int, int ) {}

	ComponentResult GetPropertyInfo( AudioUnitPropertyID             inID,
											   AudioUnitScope                  inScope,
											   AudioUnitElement                inElement,
											   UInt32&                         outDataSize,
											   Boolean&                        outWritable )
	{
		return noErr;
	}
	ComponentResult			GetProperty(			AudioUnitPropertyID 			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement			 	inElement,
													void *							outData)
	{
		return noErr;
	}
	
	int				GetSampleRate(void) { return 1; }
	
	virtual ComponentResult		GetParameterInfo(AudioUnitScope		inScope,
                                                        AudioUnitParameterID	inParameterID,
                                                        AudioUnitParameterInfo	&outParameterInfo ) = 0;
	virtual ComponentResult		GetParameterValueStrings(AudioUnitScope		inScope,
														AudioUnitParameterID	inParameterID,
														CFArrayRef *		outStrings)
	{
		return kAudioUnitErr_InvalidProperty;
	}

	void			dumpParameters(void)
	{
		const AUChannelInfo* info;
		int num = SupportedNumChannels( &info );
		if ( num == 0 )
		{
			std::cout << "KERNELS" << std::endl;
		}
		else
		{
			if ( num == 1 && info->inChannels == 2 && info->outChannels == 2 )
				std::cout << "STEREO" << std::endl;
			else
				std::cout << "UNSUPPORTED" << std::endl;
		}
		
		int numEnums = 0;
		std::ostringstream s_params;
		std::ostringstream s_enums;
		
		for ( int i=0; i<numParameters; ++i )
		{
			AudioUnitParameterInfo info;
			GetParameterInfo( kAudioUnitScope_Global, i, info );
			const char* scaling = "kNT_scaling1000";
			int min = info.minValue * 1000.0f;
			int max = info.maxValue * 1000.0f;
			int def = info.defaultValue * 1000.0f;
			const char* unit = "kNT_unitNone";
			if ( min < -32768 || min > 32767 || max < -32768 || max > 32767 || def < -32768 || def > 32767 )
			{
				scaling = "kNT_scaling100";
				min = info.minValue * 100.0f;
				max = info.maxValue * 100.0f;
				def = info.defaultValue * 100.0f;
				if ( min < -32768 || min > 32767 || max < -32768 || max > 32767 || def < -32768 || def > 32767 )
				{
					scaling = "kNT_scaling10";
					min = info.minValue * 10.0f;
					max = info.maxValue * 10.0f;
					def = info.defaultValue * 10.0f;
					if ( min < -32768 || min > 32767 || max < -32768 || max > 32767 || def < -32768 || def > 32767 )
					{
						scaling = "kNT_scalingNone";
						min = info.minValue;
						max = info.maxValue;
						def = info.defaultValue;
					}
				}
			}
			std::string enums = "NULL";
			if ( info.unit == kAudioUnitParameterUnit_Indexed )
			{
				CFArrayRef outStrings;
				ComponentResult result = GetParameterValueStrings( kAudioUnitScope_Global, i, &outStrings );
				if ( result == noErr )
				{
					unit = "kNT_unitEnum";
					scaling = "kNT_scalingNone";
					min = info.minValue;
					max = info.maxValue;
					def = info.defaultValue;
					std::ostringstream s_enumName;
					s_enumName << "enumStrings";
					s_enumName << i;
					enums = s_enumName.str();
					s_enums << "static char const * const " << enums << "[] = { ";
					for ( int i=0; i<min; ++i )
						s_enums << "\"\", ";
					for ( int i=min; i<=max; ++i )
						s_enums << "\"" << outStrings[i-min] << "\", ";
					s_enums << "};" << std::endl;
					numEnums += 1;
				}
			}
			else switch( info.unit )
			{
			case kAudioUnitParameterUnit_Percent:
					unit = "kNT_unitPercent";
					break;
			case kAudioUnitParameterUnit_Seconds:
					unit = "kNT_unitSeconds";
					break;
			case kAudioUnitParameterUnit_SampleFrames:
					unit = "kNT_unitFrames";
					break;
			case kAudioUnitParameterUnit_Hertz:
					unit = "kNT_unitHz";
					break;
			case kAudioUnitParameterUnit_Cents:
			case kAudioUnitParameterUnit_AbsoluteCents:
					unit = "kNT_unitCents";
					break;
			case kAudioUnitParameterUnit_RelativeSemiTones:
					unit = "kNT_unitSemitones";
					break;
			case kAudioUnitParameterUnit_MIDINoteNumber:
					unit = "kNT_unitMIDINote";
					break;
			case kAudioUnitParameterUnit_Decibels:
					unit = "kNT_unitDb";
					break;
			case kAudioUnitParameterUnit_BPM:
					unit = "kNT_unitBPM";
					break;
			case kAudioUnitParameterUnit_Milliseconds:
					unit = "kNT_unitMs";
					break;
			}
			s_params << "{ .name = \"" << info.name << "\", .min = " << min << ", .max = " << max << ", .def = " << def << ", .unit = " << unit << ", .scaling = " << scaling << ", .enumStrings = " << enums << " }," << std::endl;
		}
		
		if ( numEnums )
		{
			std::cout << "ENUMS" << std::endl << numEnums << std::endl << s_enums.str();
		}

		std::cout << numParameters << std::endl << s_params.str();
	}
	
	int				numParameters;
};

struct AUKernelBase
{
	AUKernelBase( AudioUnit c ) {}
	
	float			GetParameter( int ) { return 0; }

	int				GetSampleRate(void) { return 1; }
};

#endif // _AUEFFECTBASE_H
