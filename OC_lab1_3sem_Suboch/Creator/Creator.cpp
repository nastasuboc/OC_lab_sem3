#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include "employee.h"

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    if (argc != 3)
    {
        std::cerr << "Использование: Creator <имя бинарного файла> <количество записей>\n";
        return 1;
    }

    const char* fileName = argv[1];
    int count = std::atoi(argv[2]);

    if (count <= 0)
    {
        std::cerr << "Количество записей должно быть положительным числом\n";
        return 1;
    }

    std::ofstream out(fileName, std::ios::binary);
    if (!out)
    {
        std::cerr << "Не удалось создать файл " << fileName << "\n";
        return 1;
    }

    for (int i = 0; i < count; ++i)
    {
        employee e{};
        std::string name;

        std::cout << "Сотрудник #" << (i + 1) << "\n";
        std::cout << "  Номер: ";
        std::cin >> e.num;

        std::cout << "  Имя: ";
        std::cin >> name;
        std::copy(name.begin(), name.end(), e.name);

        std::cout << "  Часы: ";
        std::cin >> e.hours;

        out.write(reinterpret_cast<char*>(&e), sizeof(e));
    }

    std::cout << "Бинарный файл успешно создан.\n";
    return 0;
}
