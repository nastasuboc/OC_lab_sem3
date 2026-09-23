#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "employee.h"

bool RunAndWait(const std::string& cmd)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    std::vector<char> buf(cmd.begin(), cmd.end());
    buf.push_back('\0');

    if (!CreateProcessA(
        NULL,
        buf.data(),
        NULL, NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL, NULL,
        &si, &pi))
    {
        std::cerr << "Ошибка CreateProcess (" << GetLastError() << ")\n";
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return true;
}

int main()
{
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
    std::string binFile, reportFile;
    int count;
    double rate;

    std::cout << "Введите имя бинарного файла: ";
    std::cin >> binFile;

    std::cout << "Введите количество записей: ";
    std::cin >> count;

    std::string creatorCmd = "Creator.exe " + binFile + " " + std::to_string(count);
    if (!RunAndWait(creatorCmd))
        return 1;

    std::ifstream in(binFile, std::ios::binary);
    if (!in)
    {
        std::cerr << "Не удалось открыть файл " << binFile << "\n";
        return 1;
    }

    std::cout << "\nСодержимое бинарного файла:\n";
    employee e;

    while (in.read(reinterpret_cast<char*>(&e), sizeof(e)))
        std::cout << e.num << " " << e.name << " " << e.hours << "\n";

    in.close();

    std::cout << "\nВведите имя файла отчёта: ";
    std::cin >> reportFile;

    std::cout << "Введите оплату за час: ";
    std::cin >> rate;

    std::string reporterCmd =
        "Reporter.exe " + binFile + " " + reportFile + " " + std::to_string(rate);

    if (!RunAndWait(reporterCmd))
        return 1;

    std::ifstream rep(reportFile);
    if (!rep)
    {
        std::cerr << "Не удалось открыть отчёт " << reportFile << "\n";
        return 1;
    }

    std::cout << "\nОтчёт:\n";
    std::string line;

    while (std::getline(rep, line))
        std::cout << line << "\n";

    rep.close();

    return 0;
}
