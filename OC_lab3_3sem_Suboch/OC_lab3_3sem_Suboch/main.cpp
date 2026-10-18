#include "marker.h"
#include <iostream>
#include <vector>
#include <clocale>

int main() {
	setlocale(LC_ALL, "rus");
	InitializeCriticalSection(&cs);
	std::cout << "Введите размер массива: ";
	std::cin >> n;
	arr = new int[n];
	for (int i = 0; i < n; ++i) {
		arr[i] = 0;
	}
	int k{};
	std::cout << "Введите количество потоков marker: ";
	std::cin >> k;
	startEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	std::vector<HANDLE> threads(k);
	std::vector<bool> active(k, true); 
	for (int i = 0; i < k; ++i) {
		blockedEvent.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));
		continueEvent.push_back(CreateEvent(NULL, FALSE, FALSE, NULL)); 
		exitEvent.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));
	}
	for (int i = 0; i < k; ++i) {
		threads[i] = CreateThread(NULL, 0, marker, (LPVOID)(INT_PTR)(i + 1), 0, NULL);
	}
	SetEvent(startEvent);
	int left = k;
	while (left > 0) {
		std::vector<HANDLE> h;
		for (int i = 0; i < k; ++i) {
			if (active[i]) h.push_back(blockedEvent[i]);
		}
		WaitForMultipleObjects((DWORD)h.size(), h.data(), TRUE, INFINITE);

		EnterCriticalSection(&cs);
		printArray();
		LeaveCriticalSection(&cs);

		int num;
		while (true) {
			std::cout << "Номер потока для завершения работы: ";
			std::cin >> num;
			if (num >= 1 && num <= k && active[num - 1]) break;
		}

		SetEvent(exitEvent[num - 1]);
		WaitForSingleObject(threads[num - 1], INFINITE);
		active[num - 1] = false;
		left--;

		EnterCriticalSection(&cs);
		printArray();
		LeaveCriticalSection(&cs);

		for (int i = 0; i < k; ++i) {
			if (active[i]) {
				ResetEvent(blockedEvent[i]);
				SetEvent(continueEvent[i]);
			}
		}
	}

	for (int i = 0; i < k; ++i) {
		CloseHandle(threads[i]);
		CloseHandle(blockedEvent[i]);
		CloseHandle(continueEvent[i]);
		CloseHandle(exitEvent[i]);
	}
	CloseHandle(startEvent);
	DeleteCriticalSection(&cs);
	delete[] arr;
	return 0;
}