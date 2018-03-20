///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	A 3-passes radix-sort using a 11-bit wide radix.
 *	\file		IceRadix3Passes.cpp
 *	\author		Pierre Terdiman
 *	\date		December, 20, 2006
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	This is a radix-sort similar to my previous "revisited radix sort" code, except this one uses only 3 passes
 *	in the worst case. This is achieved using a 11-bit wide radix instead of a standard 8-bit one. This idea was
 *	originally proposed by Michael Herf, after he read my old radix article on the web. It took me years to find
 *	the time and motivation to try it, but here it is!
 *
 *	The interface is the same as for the previous sorter, so this one should be "plug and play". Just replace
 *	your "RadixSort" classes with "RadixSort3" everywhere and it should compile and run.
 *
 *	RadixSort3 is usually faster when there is a large number of values to sort (say > 10000). For smaller sets,
 *	it is *slower* than the previous version. This is kindof obvious: using 11 bits means we loop 2048 times
 *	instead of 256 in various places, so if you just have a few hundreds values to sort, creating the histograms
 *	might take just as long as running the actual sort. Anyway, it is usually a good idea to optimize the worst
 *	case, so the 3-passes version is definitely useful. In particular, it should be a good choice when used in
 *	my old "box pruning" code.
 *
 *	History:
 *	- 12.20.06: first version
 *	- 06.08.08:	removed optimizations from Kyle Hubert, since they made the code crash when negative zeros are involved.
 *				Big thanks to Ignacio Castano for reporting this bug!
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Precompiled Header
#include "StdAfx.h"

using namespace IceCore;

#define INVALIDATE_RANKS	mCurrentSize|=0x80000000
#define VALIDATE_RANKS		mCurrentSize&=0x7fffffff
#define CURRENT_SIZE		(mCurrentSize&0x7fffffff)
#define INVALID_RANKS		(mCurrentSize&0x80000000)

#define CHECK_RESIZE(n)																		\
	if(n!=mPreviousSize)																	\
	{																						\
				if(n>mCurrentSize)	Resize(n);												\
		else						ResetRanks();											\
		mPreviousSize = n;																	\
	}

#define	RADIX_NB_BITS	11
#define	RADIX_SIZE		2048
#define	MAX_NB_PASSES	3
#define H0_OFFSET		0
#define H1_OFFSET		RADIX_SIZE
#define H2_OFFSET		RADIX_SIZE*2

#define CREATE_HISTOGRAMS(type, buffer)														\
	/* Clear counters/histograms */															\
	ZeroMemory(Histogram, RADIX_SIZE*MAX_NB_PASSES*sizeof(udword));							\
																							\
	/* Prepare to count */																	\
	const udword* p = (const udword*)input;													\
	const udword* pe = (const udword*)(input+nb);											\
	udword* h0 = &Histogram[H0_OFFSET];	/* Histogram for first pass (LSB)	*/				\
	udword* h1 = &Histogram[H1_OFFSET];	/* Histogram for second pass		*/				\
	udword* h2 = &Histogram[H2_OFFSET];	/* Histogram for third pass (MSB)	*/				\
																							\
	bool AlreadySorted = true;	/* Optimism... */											\
																							\
	if(INVALID_RANKS)																		\
	{																						\
		/* Prepare for temporal coherence */												\
		const type* Running = (type*)buffer;												\
		type PrevVal = *Running;															\
																							\
		while(p!=pe)																		\
		{																					\
			/* Read input buffer in previous sorted order */								\
			const type Val = *Running++;													\
			/* Check whether already sorted or not */										\
			if(Val<PrevVal)	{ AlreadySorted = false; break; } /* Early out */				\
			/* Update for next iteration */													\
			PrevVal = Val;																	\
																							\
			/* Create histograms */															\
			udword Data = *p++;																\
			udword b0 = Data & 2047;														\
			udword b1 = (Data>>RADIX_NB_BITS) & 2047;										\
			udword b2 = (Data>>(RADIX_NB_BITS*2)) & 2047;									\
			h0[b0]++;																		\
			h1[b1]++;																		\
			h2[b2]++;																		\
		}																					\
																							\
		/* If all input values are already sorted, we just have to return and leave the */	\
		/* previous list unchanged. That way the routine may take advantage of temporal */	\
		/* coherence, for example when used to sort transparent faces.					*/	\
		if(AlreadySorted)																	\
		{																					\
			mNbHits++;																		\
			for(udword i=0;i<nb;i++)	mRanks[i] = i;										\
			return *this;																	\
		}																					\
	}																						\
	else																					\
	{																						\
		/* Prepare for temporal coherence */												\
		const udword* Indices = mRanks;														\
		type PrevVal = (type)buffer[*Indices];												\
																							\
		while(p!=pe)																		\
		{																					\
			/* Read input buffer in previous sorted order */								\
			const type Val = (type)buffer[*Indices++];										\
			/* Check whether already sorted or not */										\
			if(Val<PrevVal)	{ AlreadySorted = false; break; } /* Early out */				\
			/* Update for next iteration */													\
			PrevVal = Val;																	\
																							\
			/* Create histograms */															\
			udword Data = *p++;																\
			udword b0 = Data & 2047;														\
			udword b1 = (Data>>RADIX_NB_BITS) & 2047;										\
			udword b2 = (Data>>(RADIX_NB_BITS*2)) & 2047;									\
			h0[b0]++;																		\
			h1[b1]++;																		\
			h2[b2]++;																		\
		}																					\
																							\
		/* If all input values are already sorted, we just have to return and leave the */	\
		/* previous list unchanged. That way the routine may take advantage of temporal */	\
		/* coherence, for example when used to sort transparent faces.					*/	\
		if(AlreadySorted)	{ mNbHits++; return *this;	}									\
	}																						\
																							\
	/* Else there has been an early out and we must finish computing the histograms */		\
	while(p!=pe)																			\
	{																						\
		/* Create histograms without the previous overhead */								\
		const udword Data = *p++;															\
		const udword b0 = Data & 2047;														\
		const udword b1 = (Data>>RADIX_NB_BITS) & 2047;										\
		const udword b2 = (Data>>(RADIX_NB_BITS*2)) & 2047;									\
		h0[b0]++;																			\
		h1[b1]++;																			\
		h2[b2]++;																			\
	}

#define CHECK_PASS_VALIDITY(pass)															\
	/* Shortcut to current counters */														\
	const udword* CurCount = &Histogram[pass<<RADIX_NB_BITS];								\
																							\
	/* Reset flag. The sorting pass is supposed to be performed. (default) */				\
	bool PerformPass = true;																\
																							\
	/* Check pass validity */																\
																							\
	/* If all values are the same, sorting is useless. */									\
	/* It may happen when sorting bytes or words instead of dwords. */						\
	/* This routine actually sorts words faster than dwords, and bytes */					\
	/* faster than words. Standard running time (O(4*n))is reduced to O(2*n) */				\
	/* for words and O(n) for bytes. Running time for floats depends on actual values... */	\
																							\
	/* Get first value */																	\
	udword UniqueVal = *input;																\
	UniqueVal >>= pass*RADIX_NB_BITS;														\
	UniqueVal &= 2047;																		\
																							\
	/* Check that value's counter */														\
	if(CurCount[UniqueVal]==nb)	PerformPass=false;



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Constructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort3::RadixSort3() : mRanks(null), mRanks2(null), mCurrentSize(0), mTotalCalls(0), mNbHits(0), mDeleteRanks(true)
{
	// Initialize indices
	INVALIDATE_RANKS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Destructor.
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort3::~RadixSort3()
{
	// Release everything
	if(mDeleteRanks)
	{
		ICE_FREE(mRanks2);
		ICE_FREE(mRanks);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Resizes the inner lists.
 *	\param		nb	[in] new size (number of dwords)
 *	\return		true if success
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RadixSort3::Resize(udword nb)
{
	if(mDeleteRanks)
	{
		// Free previously used ram
		ICE_FREE(mRanks2);
		ICE_FREE(mRanks);

		// Get some fresh one
		mRanks	= (udword*)ICE_ALLOC(sizeof(udword)*nb);	CHECKALLOC(mRanks);
		mRanks2	= (udword*)ICE_ALLOC(sizeof(udword)*nb);	CHECKALLOC(mRanks2);
	}
	return true;
}

inline_ void RadixSort3::CheckResize(udword nb)
{
	udword CurSize = CURRENT_SIZE;
	if(nb!=CurSize)
	{
		if(nb>CurSize)	Resize(nb);
		mCurrentSize = nb;
		INVALIDATE_RANKS;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 *	Main sort routine.
 *	This one is for integer values. After the call, mRanks contains a list of indices in sorted order, i.e. in the order you may process your data.
 *	\param		input	[in] a list of integer values to sort
 *	\param		nb		[in] number of values to sort, must be < 2^31
 *	\param		hint	[in] RADIX_SIGNED to handle negative values, RADIX_UNSIGNED if you know your input buffer only contains positive values
 *	\return		Self-Reference
 */
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RadixSort3& RadixSort3::Sort(const udword* input, udword nb, RadixHint hint)
{
	// Checkings
	if(!input || !nb || nb&0x80000000)	return *this;

	// Stats
	mTotalCalls++;

	// Resize lists if needed
	CheckResize(nb);

	// Allocate histograms & offsets on the stack
	udword Histogram[RADIX_SIZE*MAX_NB_PASSES];
	udword* Link[RADIX_SIZE];

	// Create histograms (counters). Counters for all passes are created in one run.
	// Pros:	read input buffer once instead of once per pass
	// Cons:	histogram buffer is N times bigger with N = max number of passes
	// We must take care of signed/unsigned values for temporal coherence.... I just
	// have 2 code paths even if just a single opcode changes. Self-modifying code, someone?
	if(hint==RADIX_UNSIGNED)	{ CREATE_HISTOGRAMS(udword, input);	}
	else						{ CREATE_HISTOGRAMS(sdword, input);	}

	// Radix sort, j is the pass number (0 = LSB, MAX_NB_PASSES-1 = MSB)
	for(udword j=0;j<MAX_NB_PASSES;j++)
	{
		CHECK_PASS_VALIDITY(j);

		// Sometimes the fourth (negative) pass is skipped because all numbers are negative and the MSB is 0xFF (for example). This is
		// not a problem, numbers are correctly sorted anyway.
		if(PerformPass)
		{
			// Should we care about negative values?
			if(j!=MAX_NB_PASSES-1 || hint==RADIX_UNSIGNED)
			{
				// Here we deal with positive values only

				// Create offsets
				Link[0] = mRanks2;
				for(udword i=1;i<RADIX_SIZE;i++)
					Link[i] = Link[i-1] + CurCount[i-1];
			}
			else
			{
				// This is a special case to correctly handle negative integers. They're sorted in the right order but at the wrong place.

#ifdef KYLE_HUBERT_VERSION
				Link[1024/2] = mRanks2;
				for(udword i=1+1024/2;i<1024;i++)
					Link[i] = Link[i-1] + CurCount[i-1];

				Link[0] = Link[1024-1] + CurCount[1024-1];
				for(udword i=1;i<1024/2;i++)
					Link[i] = Link[i-1] + CurCount[i-1];
#else
				// Compute #negative values involved if needed
				udword NbNegativeValues = 0;
				if(hint==RADIX_SIGNED)
				{
					const udword* h2 = &Histogram[H2_OFFSET];
					for(udword i=1024/2;i<1024;i++)	NbNegativeValues += h2[i];
				}

				Link[0] = &mRanks2[NbNegativeValues];
				for(udword i=1;i<1024/2;i++)
					Link[i] = Link[i-1] + CurCount[i-1];

				Link[1024/2] = mRanks2;
				for(udword i=1024/2+1;i<1024;i++)
					Link[i] = Link[i-1] + CurCount[i-1];
#endif
			}

			// Perform Radix Sort
			const udword Shift = j*RADIX_NB_BITS;
			if(INVALID_RANKS)
			{
				for(udword i=0;i<nb;i++)
				{
					udword data = (input[i]>>Shift)&2047;
					*Link[data]++ = i;
				}
				VALIDATE_RANKS;
			}
			else
			{
				const udword* Indices		= mRanks;
				const udword* IndicesEnd	= mRanks + nb;
				while(Indices!=IndicesEnd)
				{
					udword id = *Indices++;
					udword data = (input[id]>>Shift)&2047;
					*Link[data]++ = id;
				}
			}

			// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
			udword* Tmp = mRanks;
			mRanks = mRanks2;
			mRanks2 = Tmp;
		}
	}
	return *this;
}

RadixSort3& RadixSort3::Sort(const float* input2, udword nb)
{
	// Checkings
	if(!input2 || !nb || nb&0x80000000)	return *this;

	// Stats
	mTotalCalls++;

	const udword* input = (const udword*)input2;

	// Resize lists if needed
	CheckResize(nb);

	// Allocate histograms & offsets on the stack
	udword Histogram[RADIX_SIZE*MAX_NB_PASSES];
	udword* Link[RADIX_SIZE];

	// Create histograms (counters). Counters for all passes are created in one run.
	// Pros:	read input buffer once instead of once per pass
	// Cons:	histogram buffer is N times bigger with N = max number of passes
	// Floating-point values are always supposed to be signed values, so there's only one code path there.
	// Please note the floating point comparison needed for temporal coherence! Although the resulting asm code
	// is dreadful, this is surprisingly not such a performance hit - well, I suppose that's a big one on first
	// generation Pentiums....We can't make comparison on integer representations because, as Chris said, it just
	// wouldn't work with mixed positive/negative values....
	{ CREATE_HISTOGRAMS(float, input2); }

	// Radix sort, j is the pass number (0 = LSB, MAX_NB_PASSES-1 = MSB)
	for(udword j=0;j<MAX_NB_PASSES;j++)
	{
		CHECK_PASS_VALIDITY(j);

		// Should we care about negative values?
		if(j!=MAX_NB_PASSES-1)
		{
			// Here we deal with positive values only
			if(PerformPass)
			{
				// Create offsets
				Link[0] = mRanks2;
				for(udword i=1;i<RADIX_SIZE;i++)
					Link[i] = Link[i-1] + CurCount[i-1];

				// Perform Radix Sort
				const udword Shift = j*RADIX_NB_BITS;
				if(INVALID_RANKS)
				{
					for(udword i=0;i<nb;i++)
					{
						udword data = (input[i]>>Shift)&2047;
						*Link[data]++ = i;
					}
					VALIDATE_RANKS;
				}
				else
				{
					const udword* Indices		= mRanks;
					const udword* IndicesEnd	= mRanks + nb;
					while(Indices!=IndicesEnd)
					{
						udword id = *Indices++;
						udword data = (input[id]>>Shift)&2047;
						*Link[data]++ = id;
					}
				}

				// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
				udword* Tmp = mRanks;
				mRanks = mRanks2;
				mRanks2 = Tmp;
			}
		}
		else
		{
			// This is a special case to correctly handle negative values
			if(PerformPass)
			{
				// For the last pass we only deal with 10 bits, not 11.....
#ifdef KYLE_HUBERT_VERSION
				Link[1024-1] = mRanks2 + CurCount[1024-1];
				for(udword i=1024-2;i>(1024/2)-1;i--)	Link[i] = Link[i+1] + CurCount[i];

				Link[0] = Link[1024/2] + CurCount[1024/2];
				for(udword i=1;i<1024/2;i++)	Link[i] = Link[i-1] + CurCount[i-1];
#else
				// Compute #negative values involved if needed
				udword NbNegativeValues = 0;
				const udword* h2 = &Histogram[H2_OFFSET];
				for(udword i=1024/2;i<1024;i++)	NbNegativeValues += h2[i];

				Link[0] = &mRanks2[NbNegativeValues];
				for(udword i=1;i<1024/2;i++)		Link[i] = Link[i-1] + CurCount[i-1];

				Link[1023] = mRanks2;
				for(udword i=0;i<1024/2-1;i++)	Link[1022-i] = Link[1023-i] + CurCount[1023-i];
				for(udword i=1024/2;i<1024;i++)	Link[i] += CurCount[i];
#endif
				// Perform Radix Sort
				if(INVALID_RANKS)
				{
					for(udword i=0;i<nb;i++)
					{
						const udword Radix = input[i]>>22;				// Radix byte, same as above. AND is useless here (udword).
						// ### cmp to be killed. Not good. Later.
						if(Radix<1024/2)	*Link[Radix]++ = i;			// Number is positive, same as above
						else				*(--Link[Radix]) = i;		// Number is negative, flip the sorting order
					}
					VALIDATE_RANKS;
				}
				else
				{
					for(udword i=0;i<nb;i++)
					{
						const udword Radix = input[mRanks[i]]>>22;			// Radix byte, same as above. AND is useless here (udword).
						// ### cmp to be killed. Not good. Later.
						if(Radix<1024/2)	*Link[Radix]++ = mRanks[i];		// Number is positive, same as above
						else				*(--Link[Radix]) = mRanks[i];	// Number is negative, flip the sorting order
					}
				}
				// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
				udword* Tmp = mRanks;
				mRanks = mRanks2;
				mRanks2 = Tmp;
			}
			else
			{
				// The pass is useless, yet we still have to reverse the order of current list if all values are negative.
				if(UniqueVal>=1024/2)
//				if(UniqueVal>=RADIX_SIZE/2)
				{
					if(INVALID_RANKS)
					{
						// ###Possible?
						for(udword i=0;i<nb;i++)	mRanks2[i] = nb-i-1;
						VALIDATE_RANKS;
					}
					else
					{
						for(udword i=0;i<nb;i++)	mRanks2[i] = mRanks[nb-i-1];
					}

					// Swap pointers for next pass. Valid indices - the most recent ones - are in mRanks after the swap.
					udword* Tmp = mRanks;
					mRanks = mRanks2;
					mRanks2 = Tmp;
				}
			}
		}
	}
	return *this;
}

bool RadixSort3::SetRankBuffers(udword* ranks0, udword* ranks1)
{
	if(!ranks0 || !ranks1)	return false;

	mRanks			= ranks0;
	mRanks2			= ranks1;
	mDeleteRanks	= false;
	return true;
}
