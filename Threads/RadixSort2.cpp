#include "stdafx.h"
#include "RadixSort2.h"

// Radix sort revisited again
// -> Pierre Terdiman 2018

// For little-endian machines
	#define H0_OFFSET	0
	#define H1_OFFSET	256
	#define H2_OFFSET	512
	#define H3_OFFSET	768
	#define BYTES_INC	j

static void createHist(udword* histogram1024, const udword* input, udword nb)
{
	ZeroMemory(histogram1024, 256*4*sizeof(udword));

	const ubyte* p = reinterpret_cast<const ubyte*>(input);
	const ubyte* pe = &p[nb*4];
	udword* h0 = &histogram1024[H0_OFFSET];
	udword* h1 = &histogram1024[H1_OFFSET];
	udword* h2 = &histogram1024[H2_OFFSET];
	udword* h3 = &histogram1024[H3_OFFSET];

	while(p!=pe)
	{
		h0[*p++]++;
		h1[*p++]++;
		h2[*p++]++;
		h3[*p++]++;
	}
}

static inline_ const udword* CheckPassValidity(udword pass, const udword* mHistogram1024, udword nb, const void* input)
{
	const udword* CurCount = &mHistogram1024[pass<<8];

	const ubyte UniqueVal = *((reinterpret_cast<const ubyte*>(input))+pass);

	if(CurCount[UniqueVal]==nb)
		return null;

	return CurCount;
}

RadixSort2::RadixSort2() : mCurrentSize(0)
{
	mSortedCombo = mSortedCombo2 = null;
}

RadixSort2::~RadixSort2()
{
	ICE_FREE(mSortedCombo2);
	ICE_FREE(mSortedCombo);
}

bool RadixSort2::Resize(udword nb)
{
	ICE_FREE(mSortedCombo2);
	ICE_FREE(mSortedCombo);
	mSortedCombo	= reinterpret_cast<Combo*>(ICE_ALLOC(sizeof(Combo)*nb));
	mSortedCombo2	= reinterpret_cast<Combo*>(ICE_ALLOC(sizeof(Combo)*nb));
	return true;
}

template<udword j>
static void sortLoop(const Combo* Indices, const Combo* IndicesEnd, const ubyte* InputBytes2, Combo* sortedCombo2, udword* offsets)
{
	Combo* links[256];
	for(udword i=0;i<256;i++)
		links[i] = sortedCombo2 + offsets[i];

	while(Indices!=IndicesEnd)
	{
		const udword sortedValue = *reinterpret_cast<const udword*>(InputBytes2);
		const ubyte id = *(InputBytes2 + BYTES_INC);
		Combo* dest = links[id]++;
		const udword index = Indices->mRank;
		Indices++;
		InputBytes2 += sizeof(Combo);
		dest->mRank = index;
		dest->mValue = sortedValue;
	}
}

// This special version doesn't output the final sorted value since we don't need it
template<udword j>
static void sortLoop2(const Combo* Indices, const Combo* IndicesEnd, const ubyte* InputBytes2, Combo* sortedCombo2, udword* offsets)
{
	udword* links[256];
	udword* base = reinterpret_cast<udword*>(sortedCombo2);
	for(udword i=0;i<256;i++)
		links[i] = base + offsets[i];

	while(Indices!=IndicesEnd)
	{
		const ubyte id = *(InputBytes2 + BYTES_INC);
		udword* dest = links[id]++;
		const udword index = Indices->mRank;
		Indices++;
		InputBytes2 += sizeof(Combo);
		*dest = index;
	}
}

// This version is only for positive integers and doesn't support "temporal coherence".
// Main improvements are:
// - output put both rank and sorted value for each pass. Then the next pass reads the new values sequentially.
// - output both rank/value together as a combo structure instead of writing to separate arrays
// - skip the value output in the last pass
udword* RadixSort2::Sort(const udword* input, udword nb)
{
	if(!input || !nb)
		return null;

	{
		udword CurSize = mCurrentSize;
		if(nb!=CurSize)
		{
			if(nb>CurSize)
				Resize(nb);
			mCurrentSize = nb;
		}
	}

	udword histogram[1024];
	createHist(histogram, input, nb);

	// We need to know ahead of time which one will be the final pass
	const udword* PassValidity[4];
	udword LastPass = 0xffffffff;
	for(udword j=0;j<4;j++)
	{
		const udword* ValidPass = CheckPassValidity(j, histogram, nb, input);
		PassValidity[j] = ValidPass;
		if(ValidPass)
			LastPass = j;
	}

	bool invalidRanks = true;
	for(udword j=0;j<4;j++)
	{
		const udword* CurCount = PassValidity[j];
		if(!CurCount)
			continue;

		const ubyte* InputBytes = reinterpret_cast<const ubyte*>(input);
        InputBytes += BYTES_INC;

		if(invalidRanks)
		{
			invalidRanks = false;

			// Create links
			Combo* links[256];
			{
				links[0] = mSortedCombo2;
				for(udword i=1;i<256;i++)
					links[i] = links[i-1] + CurCount[i-1];
			}

			// Radix Sort
			if(j!=LastPass)
			{
				for(udword i=0;i<nb;i++)
				{
					const ubyte id = InputBytes[i<<2];
					Combo* dest = links[id]++;
					ASSERT(dest<mSortedCombo2+nb);
					const udword sortedValue = input[i];
					dest->mRank = i;
					dest->mValue = sortedValue;
				}
			}
			else
			{
				ASSERT(0);
			}
		}
		else
		{
			// Create offsets
			udword offsets[256];
			{
				offsets[0] = 0;
				for(udword i=1;i<256;i++)
					offsets[i] = offsets[i-1] + CurCount[i-1];
			}

			// Radix Sort
			const Combo* Indices	= mSortedCombo;
			const Combo* IndicesEnd	= mSortedCombo + nb;

			const ubyte* InputBytes2 = reinterpret_cast<const ubyte*>(mSortedCombo);
			InputBytes2 += 4;
			if(j!=LastPass)
			{
				if(j==0)
					sortLoop<0>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
				else if(j==1)
					sortLoop<1>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
				else if(j==2)
					sortLoop<2>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
				else if(j==3)
					sortLoop<3>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
			}
			else
			{
				if(j==0)
					sortLoop2<0>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
				else if(j==1)
					sortLoop2<1>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
				else if(j==2)
					sortLoop2<2>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
				else if(j==3)
					sortLoop2<3>(Indices, IndicesEnd, InputBytes2, mSortedCombo2, offsets);
			}
		}

		Combo* Tmp	= mSortedCombo;
		mSortedCombo = mSortedCombo2;
		mSortedCombo2 = Tmp;
	}
	return reinterpret_cast<udword*>(mSortedCombo);
}

