#ifndef RADIX_SORT2_H
#define RADIX_SORT2_H

	struct Combo
	{
		udword	mRank;
		udword	mValue;
	};

	class RadixSort2
	{
		public:
						RadixSort2();
						~RadixSort2();

				udword*	Sort(const udword* input, udword nb);

				udword	mCurrentSize;
				Combo*	mSortedCombo;
				Combo*	mSortedCombo2;
		private:
				void	CheckResize(udword nb);
				bool	Resize(udword nb);
	};

#endif // RADIX_SORT2_H
