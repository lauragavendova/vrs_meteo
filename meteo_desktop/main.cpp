#include <windows.h>
#include <iostream>
#include <string>
#include <time.h>

HANDLE openPort(int argc, char *argv[]);
void configurePort(HANDLE hSerial);
void getTimeMsg(char *msg);

int main(int argc, char *argv[])
{
    // open serial port
    HANDLE hSerial = openPort(argc, argv);

    if (hSerial == INVALID_HANDLE_VALUE)
    {
        std::cerr << "Error opening port\n";
        return 1;
    }

    configurePort(hSerial);

    // declare variables
    char buffer[50];
    char msg[6];
    DWORD bytesRead;
    DWORD bytesWritten;

    std::cout << "UP and ready...\n";

    while (true)
    {
        ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (bytesRead > 0)
        {
            std::cout << "Received: " << buffer << "\n";
            if (strcmp(buffer, "get") == 0)
            {
                std::cout << "Sending time...\n";
                getTimeMsg(msg);
                WriteFile(hSerial, msg, sizeof(msg), &bytesWritten, NULL);
                bytesWritten = 0;
            }
            memset(buffer, 0, sizeof(buffer));
            bytesRead = 0;
        }
    }

    // close the serial port
    CloseHandle(hSerial);
}

void configurePort(HANDLE hSerial)
{
    // configure serial port
    DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);
    GetCommState(hSerial, &dcb);

    dcb.BaudRate = CBR_38400;
    dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity = NOPARITY;

    SetCommState(hSerial, &dcb);

    // Set timeouts
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);
}

HANDLE openPort(int argc, char *argv[])
{
    // get port name from command line or user input
    std::string portName;
    if (argc != 2)
    {
        std::cerr << "Type serial port!" << "\n";
        std::getline(std::cin, portName);
    }
    else
    {
        portName = argv[1];
    }

    std::cout << "Opening port: " << portName << "\n";

    std::string fullPortName;
    if (portName.length() > 4)
    {
        fullPortName = "\\\\.\\";
        fullPortName += portName;
    }
    else
    {
        fullPortName = portName;
    }

    HANDLE hSerial = CreateFile(
        fullPortName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    return hSerial;
}

void getTimeMsg(char *msg)
{
    // get current time
    time_t now = time(NULL);        // get current time in seconds since epoch
    struct tm *t = localtime(&now); // convert to local time

    msg[5] = t->tm_year;    // years since 1900
    msg[4] = t->tm_mon + 1; // months 1-12
    msg[3] = t->tm_mday;    // 1-31
    msg[0] = t->tm_hour;    // 0-23
    msg[1] = t->tm_min;     // 0-59
    msg[2] = t->tm_sec;     // 0-59
}