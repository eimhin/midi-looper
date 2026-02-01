
};

static const _NT_parameterPage pages[] = {
	{ .name = "Settings", .numParams = ARRAY_SIZE(page1), .params = page1 },
	{ .name = "Gain", .numParams = ARRAY_SIZE(page3), .params = page3 },
	{ .name = "Routing", .numParams = ARRAY_SIZE(page2), .params = page2 },
};

static const _NT_parameterPages parameterPages = {
	.numPages = ARRAY_SIZE(pages),
	.pages = pages,
};

void	parameterChanged( _NT_algorithm* self, int p )
{
	_airwindowsAlgorithm* pThis = (_airwindowsAlgorithm*)self;
	if ( p >= kNumTemplateParameters )
	{
		float scaling = 1.0f;
		switch ( parameters[ p ].scaling )
		{
		default:
		case kNT_scalingNone:
			break;
		case kNT_scaling10:
			scaling = 0.1f;
			break;
		case kNT_scaling100:
			scaling = 0.01f;
			break;
		case kNT_scaling1000:
			scaling = 0.001f;
			break;
		}
		pThis->scaled[ p - kNumTemplateParameters ] = self->v[ p ] * scaling;
	}
}

void			calculateRequirements( _NT_algorithmRequirements& req, const int32_t* specifications );
_NT_algorithm*	construct( const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications );
void 			step( _NT_algorithm* self, float* busFrames, int numFramesBy4 );

#ifdef AIRWINDOWS_KERNELS
static const _NT_specification specifications[] = {
	{ .name = "Channels", .min = 1, .max = 8, .def = 1, .type = kNT_typeGeneric },
};
#endif

static const _NT_factory factory = 
{
	.guid = AIRWINDOWS_GUID,
	.name = AIRWINDOWS_NAME,
	.description = AIRWINDOWS_DESCRIPTION,
#ifdef AIRWINDOWS_KERNELS
	.numSpecifications = 1,
	.specifications = specifications,
#else
	.numSpecifications = 0,
#endif
	.calculateRequirements = calculateRequirements,
	.construct = construct,
	.parameterChanged = parameterChanged,
	.step = step,
	.draw = NULL,
#ifdef AIRWINDOWS_TAGS
	.tags = AIRWINDOWS_TAGS,
#endif
};

uintptr_t pluginEntry( _NT_selector selector, uint32_t data )
{
	switch ( selector )
	{
	case kNT_selector_version:
		return kNT_apiVersionCurrent;
	case kNT_selector_numFactories:
		return 1;
	case kNT_selector_factoryInfo:
		return (uintptr_t)( ( data == 0 ) ? &factory : NULL );
	}
	return 0;
}
