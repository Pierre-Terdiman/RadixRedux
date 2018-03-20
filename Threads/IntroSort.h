#ifndef TERNARY_INTRO_SORT_H
#define TERNARY_INTRO_SORT_H


//======================================================================//
// Class: IntroSort														//
//																		//
// Template based implementation of Introspective sorting algorithm		//
// using a ternary quicksort.											//
//																		//
// Author: M.A. Maniscalco												//
// Date: January 20, 2005												//
// email: michael@michael-maniscalco.com								//
//																		//
//======================================================================//



// *** COMPILER WARNING DISABLED ***
// Disable a warning which appears in MSVC
// "conversion from '__w64 int' to 'unsigned int'"
// Just plain annoying ...  Restored at end of this file.
#pragma warning (disable : 4244)


#define MIN_LENGTH_FOR_QUICKSORT	64
#define MAX_DEPTH_BEFORE_HEAPSORT	256




//=====================================================================
// IntroSort class declaration 
// Notes: Any object used with this class must implement the following
// the operators:   <=, >=, ==
//=====================================================================

template <class T>
class IntroSort
{
public:
	IntroSort();

	virtual ~IntroSort();

	void Sort(T * array, unsigned int count);

private:
	void Partition(T * left, unsigned int count, unsigned int depth = 1);
	
	T SelectPivot(T value1, T value2, T value3);

	void Swap(T * valueA, T * valueB);

	// insertion sort methods
	void InsertionSort(T * array, unsigned int count);

	// heap sort methods
	void HeapSort(T * array, int length);

	void HeapSort(T * array, int k, int N);

};



//=====================================================================
// Begin IntroSort methods:
//=====================================================================

template <class T>
IntroSort<T>::IntroSort()
{
	// constructor
}



template <class T>
IntroSort<T>::~IntroSort()
{
	// destructor
}

	


template <class T>
inline void IntroSort<T>::Sort(T * array, unsigned int count)
{
	// Public method used to invoke the sort.

	
	// Call quick sort partition method if there are enough
	// elements to warrant it or insertion sort otherwise.
	if (count >= MIN_LENGTH_FOR_QUICKSORT)
		Partition(array, count);
	else
		InsertionSort(array, count);
}




template <class T> 
inline void IntroSort<T>::Swap(T * valueA, T * valueB)
{
	// do the ol' "switch-a-me-do" on two values.
	T temp = *valueA; 
	*valueA = *valueB; 
	*valueB = temp;
}





template <class T> 
inline T IntroSort<T>::SelectPivot(T value1, T value2, T value3)
{
	// middle of three method.
	if (value1 < value2)
		return ((value2 < value3) ? value2 : (value1 < value3) ? value3 : value1);
	return ((value1 < value3) ? value1 : (value2 < value3) ? value3 : value2); 
}





template <class T> 
inline void IntroSort<T>::Partition(T * left, unsigned int count, unsigned int depth)
{
	if (depth > MAX_DEPTH_BEFORE_HEAPSORT)
	{
		// If enough recursion has happened then we bail to heap sort since it looks
		// as if we are experiencing a 'worst case' for quick sort.  This should not
		// happen very often at all.
		HeapSort(left, count);
		return;
	}

	T * right = left + count - 1;
	T * startingLeft = left;
	T * startingRight = right;
	T * equalLeft = left;
	T * equalRight = right;

	// select the pivot value.
	T pivot = SelectPivot(left[0], right[0], left[((right - left) >> 1)]);

	// do three way partitioning.
	do
	{
		while ((left < right) && (*left <= pivot))
			if (*(left++) == pivot)
			{
				// equal to pivot value.  move to far left.
				Swap(equalLeft++, left - 1);
			}
		
		while ((left < right) && (*right >= pivot))
			if (*(right--) == pivot)
			{
				// equal to pivot value.  move to far right.
				Swap(equalRight--, right + 1);
			}
	
		if (left >= right)
		{
			if (left == right)
			{
				if (*left >= pivot)
					left--;
				if (*right <= pivot)
					right++;
			}
			else
			{
				left--;
				right++;
			}
			break;	// done partitioning
		}

		// left and right are ready for swaping
		Swap(left++, right--);
	} while (true);
	

	// move values that were equal to pivot from the far left into the middle.
	// these values are now placed in their final sorted position.
	if (equalLeft > startingLeft)
	{
		do
		{
			Swap(--equalLeft, left--);
		} while (equalLeft > startingLeft);
		if (left < startingLeft)
			left = startingLeft;
	}

	// move values that were equal to pivot from the far right into the middle.
	// these values are now placed in their final sorted position.
	if (equalRight < startingRight)
	{
		do
		{
			Swap(++equalRight, right++);	
		} while (equalRight < startingRight);	
		if (right > startingRight)
			right = startingRight;
	}

	// Calculate new partition sizes ...
	unsigned int leftSize = left - startingLeft + 1;
	unsigned int rightSize = startingRight - right + 1;

	// Partition left (less than pivot) if there are enough values to warrant it
	// otherwise do insertion sort on the values.
	if (leftSize >= MIN_LENGTH_FOR_QUICKSORT)
		Partition(startingLeft, leftSize, depth + 1);
	else
		InsertionSort(startingLeft, leftSize);

	// Partition right (greater than pivot) if there are enough values to warrant it
	// otherwise do insertion sort on the values.
	if (rightSize >= MIN_LENGTH_FOR_QUICKSORT)
		Partition(right, rightSize, depth + 1);
	else
		InsertionSort(right, rightSize);
}







template <class T> 
inline void IntroSort<T>::InsertionSort(T * array, unsigned int count)
{
	// A basic insertion sort.
	if (count < 3)
	{
		if (count == 2)
			if (array[0] > array[1])
			{
				T temp = array[0];
				array[0] = array[1];
				array[1] = temp;
			}
		return;
	}

	T * ptr2, * ptr3 = array + 1, * ptr4 = array + count;

	if (array[0] > array[1])
	{
		T temp = array[0];
		array[0] = array[1];
		array[1] = temp;
	}


	while (true)
	{
		while ((++ptr3 < ptr4) && (*ptr3 >= ptr3[-1]));
		if (ptr3 >= ptr4)
			break;

		if (ptr3[-2] <= *ptr3)
		{ 
			if (ptr3[-1] > *ptr3)
			{
				T temp = *ptr3;
				*ptr3 = ptr3[-1];
				ptr3[-1] = temp;
			}
			continue;
		}

		ptr2 = ptr3 - 1;
		T v = *ptr3;
		while ((ptr2 >= array) && (*ptr2 > v))
		{
			ptr2[1] = ptr2[0];
			ptr2--;
		}
		ptr2[1] = v;
	}
}





template <class T> 
inline void IntroSort<T>::HeapSort(T * array, int length)
{
	// A basic heapsort.
	for (int k = length >> 1; k > 0; k--)
	    HeapSort(array, k, length);

	do
	{
		T temp = array[0];
		array[0] = array[--length];
		array[length] = temp;
        HeapSort(array, 1, length);
	} while (length > 1);
}





template <class T> 
inline void IntroSort<T>::HeapSort(T * array, int k, int N)
{
	// A basic heapsort.
	T temp = array[k - 1];
	int n = N >> 1;

	while (k <= n)
	{
		int j = (k << 1);
        if ((j < N) && (array[j - 1] < array[j]))
	        j++;
	    if (temp >= array[j - 1])
			break;
	    else 
		{
			array[k - 1] = array[j - 1];
			k = j;
        }
	}

    array[k - 1] = temp;
}




// Restore the default warning which appears in MSVC for
// warning #4244 which was disabled at top of this file.
#pragma warning (default : 4244)

#endif
