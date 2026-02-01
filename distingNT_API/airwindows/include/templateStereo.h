
void	calculateRequirements( _NT_algorithmRequirements& req, const int32_t* specifications )
{
	req.numParameters = ARRAY_SIZE(parameters);
	req.sram = sizeof(_airwindowsAlgorithm);
	req.dram = sizeof(_airwindowsAlgorithm::_dram);
	req.dtc = 0;
	req.itc = 0;
}

_NT_algorithm*	construct( const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications )
{
	_airwindowsAlgorithm* alg = new (ptrs.sram) _airwindowsAlgorithm();
	alg->dram = (_airwindowsAlgorithm::_dram*)ptrs.dram;
	alg->reset();
	alg->parameters = parameters;
	alg->parameterPages = &parameterPages;
	return alg;
}

void 	step( _NT_algorithm* self, float* busFrames, int numFramesBy4 )
{
	_airwindowsAlgorithm* pThis = (_airwindowsAlgorithm*)self;
	int numFrames = numFramesBy4 * 4;
	const float* in0 = busFrames + ( pThis->v[kParamInputL] - 1 ) * numFrames;
	const float* in1 = busFrames + ( pThis->v[kParamInputR] - 1 ) * numFrames;
	float* out0 = busFrames + ( pThis->v[kParamOutputL] - 1 ) * numFrames;
	float* out1 = busFrames + ( pThis->v[kParamOutputR] - 1 ) * numFrames;
	bool replace0 = pThis->v[kParamOutputLmode];
	bool replace1 = pThis->v[kParamOutputRmode];
	
	float* temp0 = NT_globals.workBuffer;
	float* temp1 = temp0 + numFrames;
	
	float preGain = 1.0f;
	int pre = pThis->v[kParamPrePostGain];
	if ( pre != 0 )
	{
		preGain = powf( 10.0f, pre/20.0f );
		for ( int i=0; i<numFrames; ++i )
		{
			float v0 = in0[i] * preGain;
			float v1 = in1[i] * preGain;
			temp0[i] = v0;
			temp1[i] = v1;
		}
		in0 = temp0;
		in1 = temp1;
		temp0 = NT_globals.workBuffer + 2 * numFrames;
		temp1 = temp0 + numFrames;
	}
	
	pThis->render( in0, in1, replace0 ? out0 : temp0, replace1 ? out1 : temp1, numFrames );

	if ( pre != 0 )
	{
	    float postGain = 1.0f/preGain;
		if ( !replace0 )
		{
			for ( int i=0; i<numFrames; ++i )
				out0[i] += temp0[i] * postGain;
		}
		else
		{
			for ( int i=0; i<numFrames; ++i )
				out0[i] *= postGain;
		}
		if ( !replace1 )
		{
			for ( int i=0; i<numFrames; ++i )
				out1[i] += temp1[i] * postGain;
		}
		else
		{
			for ( int i=0; i<numFrames; ++i )
				out1[i] *= postGain;
		}
	}
	else
	{	
		if ( !replace0 )
		{
			for ( int i=0; i<numFrames; ++i )
				out0[i] += temp0[i];
		}
		if ( !replace1 )
		{
			for ( int i=0; i<numFrames; ++i )
				out1[i] += temp1[i];
		}
	}
}
