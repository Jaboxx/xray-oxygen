#ifndef        COMMON_H
#define        COMMON_H

#include "shared\common.h"
uniform	float4		screen_res; //doesn't really belong here but fuck it
#include "common_defines.h"
#include "common_policies.h"
#include "common_iostructs.h"
#include "common_samplers.h"
#include "common_cbuffers.h"
#include "common_functions.h"

// #define USE_SUPER_SPECULAR


#ifdef        USE_R2_STATIC_SUN
#  define xmaterial float(1.0h/4.h)
#else
#  define xmaterial float(L_material.w)
#endif

#endif
