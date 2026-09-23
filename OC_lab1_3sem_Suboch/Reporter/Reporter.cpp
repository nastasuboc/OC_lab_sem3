#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <cstdlib>
#include <windows.h>
#include "employee.h"

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    if (argc != 4)
    {
        std::cerr << "Использование: Reporter <бинарный файл> <файл отчёта> <ставка>\n";
        return 1;
    }

    const char* binName = argv[1];
    const char* reportName = argv[2];
    double rate = std::atof(argv[3]);

    std::ifstream in(binName, std::ios::binary);
    if (!in)
    {
        std::cerr << "Не удалось открыть бинарный файл " << binName << "\n";
        return 1;
    }

    std::vector<employee> list;
    employee e;

    while (in.read(reinterpret_cast<char*>(&e), sizeof(e)))
        list.push_back(e);

    in.close();

    std::sort(list.begin(), list.end(),
        [](const employee& a, const employee& b) { return a.num < b.num; });

    std::ofstream out(reportName);
    if (!out)
    {
        std::cerr << "Не удалось создать файл отчёта " << reportName << "\n";
        return 1;
    }

    out << "Отчёт по файлу \"" << binName << "\"\n";
    out << "Номер, Имя, Часы, Зарплата\n";
    out << std::fixed << std::setprecision(2);

    for (const employee& emp : list)
    {
        out << emp.num << ", " << emp.name << ", " << emp.hours
            << ", " << emp.hours * rate << "\n";
    }

    out.close();
    return 0;
}
