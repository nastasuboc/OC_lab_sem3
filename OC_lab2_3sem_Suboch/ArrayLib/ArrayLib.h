#pragma once

#ifdef ARRAYLIB_EXPORTS
#define ARRAYLIB_API __declspec(dllexport)
#else
#define ARRAYLIB_API __declspec(dllimport)
#endif

extern "C" ARRAYLIB_API void ReplaceMinMaxWithAvg(int* arr, int size, int min_val, int max_val, double avg_val);