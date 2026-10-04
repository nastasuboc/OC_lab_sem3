#include "pch.h"
#include "ArrayLib.h"


extern "C" ARRAYLIB_API void ReplaceMinMaxWithAvg(int* arr, int size, int min_val, int max_val, double avg_val) {
    if (!arr || size <= 0) return;

    for (int i = 0; i < size; ++i) {
        if (arr[i] == min_val || arr[i] == max_val) {
            arr[i] = static_cast<int>(avg_val);
        }
    }
}

