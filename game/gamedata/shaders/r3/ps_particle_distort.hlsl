#include "common.h"

struct 	v2p
{
 	float2 	tc0	: 	TEXCOORD0;	// base
	float4	c	:	COLOR0;		// diffuse

#ifdef	USE_SOFT_PARTICLES
//	Igor: for additional depth dest
	float4 tctexgen	: TEXCOORD1;
#endif	//	USE_SOFT_PARTICLES
	float   fog : FOG;			// fog
	float4 hpos	: SV_Position;
};


//////////////////////////////////////////////////////////////////////////////////////////
// Pixel
Texture2D s_distort;
float4 main ( v2p I ) : SV_Target
{
	float4	distort = s_distort.Sample(smp_linear, I.tc0);
	float    factor = distort.a * dot(I.c.rgb,0.33h);
	float4	result = float4	(distort.rgb, factor);

	result.a *= I.fog*I.fog; // ForserX: Port Skyloader fog fix
	return result;
}
