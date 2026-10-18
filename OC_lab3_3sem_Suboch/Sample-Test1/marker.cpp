#include "pch.h"
#include "marker.h"
#include <iostream>
#include <cstdlib>

int* arr;
int n{};

CRITICAL_SECTION cs;
HANDLE startEvent;
std::vector<HANDLE> blockedEvent;
std::vector<HANDLE> continueEvent;
std::vector<HANDLE> exitEvent;

void printArray() {
	for (int i = 0; i < n; ++i) {
		std::cout << arr[i] << ' ';
	}
	std::cout << std::endl;
}

DWORD WINAPI marker(LPVOID param) {
	int id = (int)(INT_PTR)param;
	int count = 0; 
	WaitForSingleObject(startEvent, INFINITE);
	srand(id);
	while (true) {
		int index = rand() % n;
		EnterCriticalSection(&cs);
		if (arr[index] == 0) {
			Sleep(5);
			arr[index] = id;
			count++;
			Sleep(5);
			LeaveCriticalSection(&cs);
		}
		else {
			std::cout << "Marker " << id << ": marked = " << count << ", cannot mark index " << index << std::endl;
			LeaveCriticalSection(&cs);
			SetEvent(blockedEvent[id - 1]); 
			HANDLE h[2] = { continueEvent[id - 1], exitEvent[id - 1] };
			DWORD r = WaitForMultipleObjects(2, h, FALSE, INFINITE);
			if (r == WAIT_OBJECT_0 + 1) {
				EnterCriticalSection(&cs);
				for (int i = 0; i < n; ++i) {
					if (arr[i] == id) arr[i] = 0;
				}
				LeaveCriticalSection(&cs);
				return 0;
			}
		}
	}
}