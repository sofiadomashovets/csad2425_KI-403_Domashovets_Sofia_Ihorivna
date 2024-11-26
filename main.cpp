#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

HANDLE serialHandle;

/**
 * @brief Налаштування послідовного порту.
 * 
 * @param portName Ім'я порту (наприклад, L"\\\\.\\COM6").
 * @return true Якщо налаштування пройшло успішно.
 * @return false Якщо сталася помилка під час налаштування.
 */
bool setupSerialPort(const wstring& portName) {
    serialHandle = CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (serialHandle == INVALID_HANDLE_VALUE) {
        cerr << "Error opening COM port.\n";
        return false;
    }

    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    if (!GetCommState(serialHandle, &serialParams)) {
        cerr << "Error getting COM state.\n";
        return false;
    }

    serialParams.BaudRate = CBR_9600;
    serialParams.ByteSize = 8;
    serialParams.StopBits = ONESTOPBIT;
    serialParams.Parity = NOPARITY;

    if (!SetCommState(serialHandle, &serialParams)) {
        cerr << "Error setting COM state.\n";
        return false;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serialHandle, &timeouts)) {
        cerr << "Error setting timeouts.\n";
        return false;
    }
    return true;
}

/**
 * @brief Відправка даних до Arduino із завершенням рядка.
 * 
 * @param data Дані, які потрібно відправити.
 */
void sendData(const string& data) {
    DWORD bytesWritten;
    string dataWithEndline = data + "\n";  // Завершуємо кожен рядок символом \n
    WriteFile(serialHandle, dataWithEndline.c_str(), dataWithEndline.length(), &bytesWritten, NULL);
    this_thread::sleep_for(chrono::milliseconds(100)); // Коротка пауза для стабільності
}

/**
 * @brief Отримання даних від Arduino з очікуванням завершення рядка.
 * 
 * @return string Отримані дані у вигляді рядка.
 */
string receiveData() {
    DWORD bytesRead;
    char buffer[256];
    string result;
    do {
        ReadFile(serialHandle, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        buffer[bytesRead] = '\0';
        result += buffer;
    } while (bytesRead > 0 && result.find('\n') == string::npos);  // Чекаємо кінця рядка
    return result;
}

/**
 * @brief Основна функція програми для взаємодії з Arduino через послідовний порт.
 * 
 * @return int Код завершення програми (0 - успішно, 1 - помилка).
 */
int main() {
    wstring portName = L"\\\\.\\COM6";
    if (!setupSerialPort(portName)) {
        return 1;
    }

    cout << "Enter Player X name: ";
    string player1;
    getline(cin, player1);

    cout << "Enter Player O name: ";
    string player2;
    getline(cin, player2);

    // Надсилаємо імена гравців з очікуванням підтвердження
    sendData(player1);
    cout << "Arduino response:\n" << receiveData();  // Очікуємо відповідь

    sendData(player2);
    cout << "Arduino response:\n" << receiveData();  // Очікуємо відповідь

    // Основний ігровий цикл
    while (true) {
        int row, col;
        cout << "Enter move (row and column): ";
        cin >> row >> col;

        // Надсилаємо координати ходу
        sendData(to_string(row) + " " + to_string(col));

        // Отримуємо відповідь від Arduino
        string response = receiveData();
        cout << "Arduino response:\n" << response << endl;
    }

    CloseHandle(serialHandle);
    return 0;
}