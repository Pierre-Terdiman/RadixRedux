// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>

	#define ICE_DONT_CHECK_COMPILER_OPTIONS

	// From Windows...
	typedef int                 BOOL;
	#ifndef FALSE
	#define FALSE               0
	#endif

	#ifndef TRUE
	#define TRUE                1
	#endif

	#include <stdio.h>
	#include <stdlib.h>
	#include <assert.h>
	#include <string.h>
	#include <float.h>
	#include <Math.h>

	#ifndef ASSERT
		#define	ASSERT(exp)	{}
	#endif
	#define ICE_COMPILE_TIME_ASSERT(exp)	extern char ICE_Dummy[ (exp) ? 1 : -1 ]

	#define	Log				{}
//	#define	SetIceError		false
	#define	SetIceError(x,y)	false
	#define	EC_OUTOFMEMORY	"Out of memory"

	#include ".\Ice\IcePreprocessor.h"

	#undef ICECORE_API
	#define ICECORE_API//	OPCODE_API

	#include ".\Ice\IceTypes.h"
	#include ".\Ice\IceFPU.h"
	#include ".\Ice\IceMemoryMacros.h"

	namespace IceCore
	{
		#include ".\Ice\IceUtils.h"
		#include ".\Ice\IceAllocator.h"
		#include ".\Ice\IceRevisitedRadix.h"
		#include ".\Ice\IceRadix3Passes.h"
		#include ".\Ice\IceRandom.h"
	}
	using namespace IceCore;

	inline_ void	StartProfile(udword& val)
	{
		__asm{
			cpuid
			rdtsc
			mov		ebx, val
			mov		[ebx], eax
		}
	}

	inline_ void	EndProfile(udword& val)
	{
		__asm{
			cpuid
			rdtsc
			mov		ebx, val
			sub		eax, [ebx]
			mov		[ebx], eax
		}
	}
