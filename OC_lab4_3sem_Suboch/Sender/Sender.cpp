#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "common.h"

using namespace std;

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc < 3) {
        cerr << "Использование: Sender <имя файла> <номер процесса>\n";
        return 1;
    }
    string fileName = argv[1];
    int index = atoi(argv[2]);

    fstream file(fileName, ios::in | ios::out | ios::binary);
    if (!file) {
        cerr << "Не удалось открыть файл " << fileName << endl;
        system("pause");
        return 1;
    }

    HANDLE hMutex = OpenMutexA(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, MUTEX_NAME);
    HANDLE hEmpty = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, EMPTY_SEM_NAME);
    HANDLE hFull = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, FULL_SEM_NAME);
    char evName[64];
    sprintf_s(evName, READY_EVENT_FMT, index);
    HANDLE hReady = OpenEventA(EVENT_MODIFY_STATE, FALSE, evName);
    if (!hMutex || !hEmpty || !hFull || !hReady) {
        cerr << "Не удалось открыть объекты синхронизации\n";
        system("pause");
        return 1;
    }

    SetEvent(hReady);
    cout << "Sender #" << index << " готов к работе.\n";

    while (true) {
        cout << "\nКоманды: 1 - отправить сообщение, 0 - завершить работу\n> ";
        int cmd;
        if (!(cin >> cmd)) { cin.clear(); cin.ignore(10000, '\n'); continue; }
        if (cmd == 0) break;
        if (cmd != 1) { cout << "Неизвестная команда.\n"; continue; }

        cout << "Введите сообщение (меньше " << MSG_LEN << " символов): ";
        string text;
        cin >> ws;
        getline(cin, text);

        char msg[MSG_LEN] = { 0 };
        strncpy_s(msg, MSG_LEN, text.c_str(), MSG_LEN - 1);  

        if (WaitForSingleObject(hEmpty, 0) == WAIT_TIMEOUT) {
            cout << "Файл заполнен, ожидание свободного места...\n";
            WaitForSingleObject(hEmpty, INFINITE);
        }

        WaitForSingleObject(hMutex, INFINITE);

        Header h;
        file.clear();
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&h), sizeof(h));

        file.seekp(sizeof(Header) + (streamoff)h.tail * MSG_LEN);
        file.write(msg, MSG_LEN);

        h.tail = (h.tail + 1) % h.capacity;    
        file.write(reinterpret_cast<char*>(&h), sizeof(h));
        file.flush();

        ReleaseMutex(hMutex);
        ReleaseSemaphore(hFull, 1, NULL);   

        cout << "Сообщение отправлено.\n";
    }

    file.close();
    CloseHandle(hMutex);
    CloseHandle(hEmpty);
    CloseHandle(hFull);
    CloseHandle(hReady);
    return 0;
}
