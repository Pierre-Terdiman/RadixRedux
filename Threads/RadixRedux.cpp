#include "stdafx.h"

void TestRadix();
void TestRadix2();
void TestIntroSort();
void TestStdSort();
void InitSortValues();
void ReleaseSortValues();

int _tmain(int argc, _TCHAR* argv[])
{
	InitSortValues();
	TestRadix();
	TestRadix2();
	TestIntroSort();
	TestStdSort();
	ReleaseSortValues();
	return 0;
}

