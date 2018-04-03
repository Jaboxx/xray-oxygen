#include "common.h"

struct v2p
{
	float2 tc0	: TEXCOORD0;
	float4 c	: COLOR0;

//	Igor: for additional depth dest
#ifdef	USE_SOFT_PARTICLES
	float4 tctexgen	: TEXCOORD1;
#endif	//	USE_SOFT_PARTICLES
	float  fog : FOG;		// fog
	float4 hpos	: SV_Position;
};

//	Must be less than view near
#define	DEPTH_EPSILON	0.1h
//////////////////////////////////////////////////////////////////////////////////////////
// Pixel
float4 main( v2p I ) : SV_Target
{
	float4 result = I.c * s_base.Sample(smp_base, I.tc0);

	//	Igor: additional depth test
#ifdef	USE_SOFT_PARTICLES
	float2 tcProj = I.tctexgen.xy / I.tctexgen.w;
#ifdef GBUFFER_OPTIMIZATION
	gbuffer_data gbd = gbuffer_load_data(tcProj, I.hpos);
#else
	gbuffer_data gbd = gbuffer_load_data(tcProj);
#endif
	float4 _P = float4(gbd.P, gbd.mtl);
	float spaceDepth = _P.z - I.tctexgen.z - DEPTH_EPSILON;
	if (spaceDepth < -2 * DEPTH_EPSILON) spaceDepth = 100000.0h; //  Skybox doesn't draw into position buffer
	result.a *= Contrast(saturate(spaceDepth*1.3h), 2);
	result.rgb *= Contrast(saturate(spaceDepth*1.3h), 2);
#endif	//	USE_SOFT_PARTICLES

	clip(result.a - (0.01f / 255.0f));
	result.a *= I.fog*I.fog; // ForserX: Port Skyloader fog fix
	return	result;
}
