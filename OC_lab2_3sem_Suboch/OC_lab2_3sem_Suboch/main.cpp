#include <iostream>
#include <windows.h>

typedef void (*ReplaceFunc)(int*, int, int, int, double);

struct ArrayData {
	int* arr;
	int size;
	int min_val;
	int max_val;
	double avg_val;
};

DWORD WINAPI thread_min_max(LPVOID lpParam) {
	ArrayData* data = (ArrayData*)lpParam;
	if (data->size <= 0) return 0;

	data->min_val = data->arr[0];
	data->max_val = data->arr[0];
	for (int i = 1; i < data->size; i++) {
		if (data->arr[i] > data->max_val) {
			data->max_val = data->arr[i];
		}
		Sleep(7);
		if (data->arr[i] < data->min_val) {
			data->min_val = data->arr[i];
		}
		Sleep(7);
	}
	std::cout << "(Поток thread_min_max) Минимальный элемент: " << data->min_val << std::endl;
	std::cout << "(Поток thread_min_max) Максимальный элемент: " << data->max_val << std::endl;
	return 0;
}

DWORD WINAPI thread_average(LPVOID lpParam) {
	ArrayData* data = (ArrayData*)lpParam;
	if (data->size <= 0) return 0;
	long long sum = 0;
	for (int i = 0; i < data->size; i++) {
		sum += data->arr[i];
		Sleep(12);
	}
	data->avg_val = (double)sum / data->size;
	std::cout << "(Поток thread_average) Среднее арифмитическое суммы всех элементов массива: " << data->avg_val << std::endl;
	return 0;
}

int main(){
	setlocale(LC_ALL, "rus");

	int size = 0;
	std::cout << "Введите размер массива: " << std::endl;
	std::cin >> size;
	if (size <= 0) {
		std::cout << "Ошибка: размер должен быть > 0!" << std::endl;
		return 1;
	}
	int* arr = new int[size];
	std::cout << "Введите " << size << " элементов массива: " << std::endl;
	for (int i = 0; i < size; i++) {
		std::cin >> arr[i];
	}

	ArrayData data;
	data.arr = arr;
	data.size = size;
	data.max_val = 0;
	data.min_val = 0;
	data.avg_val = 0.0;

	HANDLE hMinMax = CreateThread(NULL, 0, thread_min_max, &data, 0, NULL);
	HANDLE hAverage = CreateThread(NULL, 0, thread_average, &data, 0, NULL);
	if (hMinMax == NULL || hAverage == NULL) {
		std::cout << "Ошибка при создании потоков!" << std::endl;
		delete[] arr;
		return 1;
	}

	WaitForSingleObject(hMinMax, INFINITE);
	WaitForSingleObject(hAverage, INFINITE);
	CloseHandle(hMinMax);
	CloseHandle(hAverage);
	
	HMODULE hLib = LoadLibraryA("ArrayLib.dll");

	if (hLib != NULL) {
		ReplaceFunc replaceFunc = (ReplaceFunc)GetProcAddress(hLib, "ReplaceMinMaxWithAvg");

		if (replaceFunc != NULL) {
			replaceFunc(data.arr, data.size, data.min_val, data.max_val, data.avg_val);
			std::cout << "\n[DLL] Замена выполнена успешно через ArrayLib.dll!" << std::endl;
		}
		else {
			std::cout << "\nОшибка: Функция 'ReplaceMinMaxWithAvg' не найдена в DLL!" << std::endl;
		}

		FreeLibrary(hLib);
	}
	else {
		std::cout << "\nОшибка: Не удалось загрузить 'ArrayLib.dll'!" << std::endl;
	}
	std::cout << "(Поток main) Массив после изменений: " << std::endl;
	for (int i = 0; i < size; i++) {
		std::cout << arr[i] << ' ';
	}
	std::cout << std::endl;
	delete[] arr;
	return 0;
}   