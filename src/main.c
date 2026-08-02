#include <windows.h>
#include <winsock.h>
#include "login.h"

int winsock_init() {
    WORD wVersionRequested = MAKEWORD(1, 1);
    WSADATA wsaData;

    return WSAStartup(wVersionRequested, &wsaData);
}

int PASCAL WinMain(HANDLE hInst, HANDLE hPrevInst, LPSTR cmdLine, int cmdShow) {   
        if(winsock_init()) {
                return 1;
        }
        return Login_main(hInst, hPrevInst, cmdLine, cmdShow);
}
