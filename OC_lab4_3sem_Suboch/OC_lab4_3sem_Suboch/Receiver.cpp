#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include "common.h"

using namespace std;

static const char* ENV_VAR_NAME = "Suboch";

static string chooseFileName()
{
    while (true) {
        cout << "Выберите, как получить имя файла:\n"
            << "  1 - ввести с консоли\n"
            << "  2 - получить из переменной среды '" << ENV_VAR_NAME << "'\n> ";
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        if (choice == 1) {
            string name;
            cout << "Введите имя бинарного файла: ";
            cin >> name;
            return name;
        }
        if (choice == 2) {
            char* value = nullptr;
            size_t size = 0;
            errno_t err = _dupenv_s(&value, &size, ENV_VAR_NAME);
            if (err != 0 || value == nullptr || size == 0 || value[0] == '\0') {
                cout << "Переменная среды '" << ENV_VAR_NAME
                    << "' не существует или пуста. Попробуйте снова.\n";
                free(value);   
                continue;
            }
            string name(value);
            free(value);       
            cout << "Имя файла из переменной среды: " << name << "\n";
            return name;
        }
        cout << "Неверный выбор.\n";
    }
}

int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    string fileName = chooseFileName();

    int capacity = 0;
    do {
        cout << "Введите количество записей в файле (> 0): ";
        if (!(cin >> capacity)) { cin.clear(); cin.ignore(10000, '\n'); capacity = 0; }
    } while (capacity <= 0);

    {
        ofstream fout(fileName, ios::binary | ios::trunc);
        if (!fout) {
            cerr << "Не удалось создать файл " << fileName << endl;
            return 1;
        }
        Header h{ capacity, 0, 0 };
        fout.write(reinterpret_cast<char*>(&h), sizeof(h));
        char empty[MSG_LEN] = { 0 };
        for (int i = 0; i < capacity; ++i)
            fout.write(empty, MSG_LEN);
    }

    int senderCount = 0;
    do {
        cout << "Введите количество процессов Sender (> 0): ";
        if (!(cin >> senderCount)) { cin.clear(); cin.ignore(10000, '\n'); senderCount = 0; }
    } while (senderCount <= 0);

    HANDLE hMutex = CreateMutexA(NULL, FALSE, MUTEX_NAME);
    HANDLE hEmpty = CreateSemaphoreA(NULL, capacity, capacity, EMPTY_SEM_NAME); 
    HANDLE hFull = CreateSemaphoreA(NULL, 0, capacity, FULL_SEM_NAME);        
    if (!hMutex || !hEmpty || !hFull) {
        cerr << "Не удалось создать объекты синхронизации\n";
        return 1;
    }

    vector<HANDLE> readyEvents(senderCount);
    vector<PROCESS_INFORMATION> procs(senderCount);

    for (int i = 0; i < senderCount; ++i) {
        char evName[64];
        sprintf_s(evName, READY_EVENT_FMT, i);
        readyEvents[i] = CreateEventA(NULL, TRUE, FALSE, evName); // ручной сброс

        char cmd[512];
        sprintf_s(cmd, "Sender.exe \"%s\" %d", fileName.c_str(), i);

        STARTUPINFOA si = { sizeof(si) };
        ZeroMemory(&procs[i], sizeof(PROCESS_INFORMATION));
        if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE, NULL, NULL, &si, &procs[i])) {
            cerr << "Не удалось запустить Sender #" << i << " (ошибка " << GetLastError() << ")\n";
            return 1;
        }
    }

    cout << "Ожидание готовности всех процессов Sender...\n";
    WaitForMultipleObjects(senderCount, readyEvents.data(), TRUE, INFINITE);
    cout << "Все процессы Sender готовы к работе.\n";

    fstream file(fileName, ios::in | ios::out | ios::binary);
    while (true) {
        cout << "\nКоманды: 1 - прочитать сообщение, 0 - завершить работу\n> ";
        int cmd;
        if (!(cin >> cmd)) { cin.clear(); cin.ignore(10000, '\n'); continue; }
        if (cmd == 0) break;
        if (cmd != 1) { cout << "Неизвестная команда.\n"; continue; }

        if (WaitForSingleObject(hFull, 0) == WAIT_TIMEOUT) {
            cout << "Файл пуст, ожидание нового сообщения...\n";
            WaitForSingleObject(hFull, INFINITE);
        }

        WaitForSingleObject(hMutex, INFINITE);

        Header h;
        file.clear();
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&h), sizeof(h));

        char msg[MSG_LEN] = { 0 };
        file.seekg(sizeof(Header) + (streamoff)h.head * MSG_LEN);
        file.read(msg, MSG_LEN);

        h.head = (h.head + 1) % h.capacity;  
        file.seekp(0);
        file.write(reinterpret_cast<char*>(&h), sizeof(h));
        file.flush();

        ReleaseMutex(hMutex);
        ReleaseSemaphore(hEmpty, 1, NULL);   

        msg[MSG_LEN - 1] = '\0';
        cout << "Получено сообщение: " << msg << endl;
    }

    file.close();
    for (int i = 0; i < senderCount; ++i) {
        TerminateProcess(procs[i].hProcess, 0); 
        CloseHandle(procs[i].hProcess);
        CloseHandle(procs[i].hThread);
        CloseHandle(readyEvents[i]);
    }
    CloseHandle(hMutex);
    CloseHandle(hEmpty);
    CloseHandle(hFull);
    return 0;
}
