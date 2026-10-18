#pragma once
#include <vector>
#include <windows.h>

extern int* arr;
extern int n;
extern CRITICAL_SECTION cs;
extern HANDLE startEvent;
extern std::vector<HANDLE> blockedEvent;
extern std::vector<HANDLE> continueEvent;
extern std::vector<HANDLE> exitEvent;

void printArray();
DWORD WINAPI marker(LPVOID param);
