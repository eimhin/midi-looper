
#include <math.h>
#include <algorithm>
#include <new>
#include <distingnt/api.h>

#define M_PI		3.14159265358979323846
#define M_PI_2		1.57079632679489661923
#define M_PI_4		0.78539816339744830962

const float pi = M_PI;

#define noErr (0)

typedef int16_t SInt16;
typedef int32_t SInt32;
typedef uint32_t UInt32;
typedef float Float32;
typedef double Float64;
typedef bool Boolean;

inline uint32_t GetSampleRate(void)
{
	return NT_globals.sampleRate;
}

struct _airwindowsAlgorithm : public _NT_algorithm
{
	_airwindowsAlgorithm() {}
	~_airwindowsAlgorithm() {}
	
	void render( const Float32* inputL, const Float32* inputR, Float32* outputL, Float32* outputR, UInt32 inFramesToProcess );
	int reset(void);
	
	float	scaled[ kNumberOfParameters ];
#ifdef AIRWINDOWS_KERNELS
	uint32_t	numChannels;
#endif

	inline float GetParameter( int index )
	{
		return scaled[ index ];
	}
