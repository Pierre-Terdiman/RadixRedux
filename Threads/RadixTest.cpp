#include "stdafx.h"
#include "RadixSort2.h"
#include <windows.h>

// Companion code for "Radix Redux" article.

/*
			100		1000	10000	100000	1000000	5242880			10000000	20000000
-----------------------------------------------------------			------------------------
Radix		11		36		360		4702	54318	2589746			2663803		3872497
RadixRedux	8		46		370		4948	52275	290504			537077		1116551
IntroSort	7		103		1376	17557	211480	1260403			2382323		1043539
std::sort	11		137		1775	22461	272223	1573536			3054785		2536993
*/

/*

			1000000	5242880	10000000	20000000
-----------------------------------------------------------------------------------
Radix		16		755		1960		4736
RadixRedux	15		81		154			315
IntroSort	61		348		692			1448
std::sort	78		451		885			1851

*/


//#define NB_TO_SORT	100
//#define NB_TO_SORT	1000
//#define NB_TO_SORT	10000
//#define NB_TO_SORT	100000
//#define NB_TO_SORT	1000000
#define NB_TO_SORT	5242880
//#define NB_TO_SORT	10000000
//#define NB_TO_SORT	20000000

#define RADIX_SORTER	RadixSort
//#define RADIX_SORTER	RadixSort3


static udword* gValues = null;

void InitSortValues()
{
	timeBeginPeriod(1);
	SRand(0);
	gValues = new udword[NB_TO_SORT];
	for(udword i=0;i<NB_TO_SORT;i++)
		gValues[i] = Rand()|(Rand()<<16);
}

void ReleaseSortValues()
{
	DELETEARRAY(gValues);
}

//#define START_PROFILE	udword Time;	StartProfile(Time);
//#define END_PROFILE(x)	EndProfile(Time);	printf(x, Time/1024);

#define START_PROFILE	udword Time = timeGetTime();
#define END_PROFILE(x)	printf(x, timeGetTime() - Time);


void TestRadix2()
{
	START_PROFILE
		RadixSort2 RS2;
		const udword* Sorted = RS2.Sort(gValues, NB_TO_SORT);
	END_PROFILE("%d (RadixRedux)\n")

	for(udword i=0;i<NB_TO_SORT-1;i++)
		if(gValues[Sorted[i]]>gValues[Sorted[i+1]])
			printf("ERROR!\n");
}

void TestRadix()
{
	START_PROFILE
		RADIX_SORTER RS;
		const udword* Sorted = RS.Sort(gValues, NB_TO_SORT, RADIX_UNSIGNED).GetRanks();
	END_PROFILE("%d (Radix)\n")

	for(udword i=0;i<NB_TO_SORT-1;i++)
		if(gValues[Sorted[i]]>gValues[Sorted[i+1]])
			printf("ERROR!\n");
}

	struct Key
	{
		udword	mValue;
		int		mID;

		inline_	bool	operator==(const Key& p)	const	{ return mValue == p.mValue;	}
		inline_	bool	operator<=(const Key& p)	const	{ return mValue <= p.mValue;	}
		inline_	bool	operator>=(const Key& p)	const	{ return mValue >= p.mValue;	}
		inline_	bool	operator<(const Key& p)		const	{ return mValue < p.mValue;		}
		inline_	bool	operator>(const Key& p)		const	{ return mValue > p.mValue;		}
	};

#include <algorithm>
void TestStdSort()
{
	Key* Values = new Key[NB_TO_SORT];
	for(udword i=0;i<NB_TO_SORT;i++)
	{
		Values[i].mValue = gValues[i];
		Values[i].mID = i;
	}

	START_PROFILE
		std::sort(Values, Values+NB_TO_SORT);
	END_PROFILE("%d (std::sort)\n")

	for(udword i=0;i<NB_TO_SORT-1;i++)
		if(Values[i]>Values[i+1])
			printf("ERROR!\n");

	DELETEARRAY(Values);
}

#include "IntroSort.h"
void TestIntroSort()
{
	Key* Values = new Key[NB_TO_SORT];
	for(udword i=0;i<NB_TO_SORT;i++)
	{
		Values[i].mValue = gValues[i];
		Values[i].mID = i;
	}

	START_PROFILE
		IntroSort<Key> introSorter;
		introSorter.Sort(Values, NB_TO_SORT);
	END_PROFILE("%d (IntroSort)\n")

	for(udword i=0;i<NB_TO_SORT-1;i++)
		if(Values[i]>Values[i+1])
			printf("ERROR!\n");

	DELETEARRAY(Values);
}
