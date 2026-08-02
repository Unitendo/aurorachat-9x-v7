#include "ipsetup.h"
#include "common.h"
#include "login.h"
#include <stdio.h>

const char IPSETUP_WINCLASS[] = "AUC IPSETUP";

long PASCAL IPSetup_WP(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_DESTROY:
                break;

                case WM_CREATE: {
                        int x = 8;
                        int y = 8;
                        char buffer[64];
                        RECT clrect;
                        GetClientRect(hwnd, &clrect);

                        CreateWindow(
                                "Static", "Server IP:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                x, y, 80 - x, 16,
                                hwnd, 0, NULL, NULL
                        );

                        CreateWindow(
                                "Edit", Login_targetip, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                80, y, 232, 24,
                                hwnd, (HMENU) 3, NULL, NULL
                        );

                        y += 24;

                        CreateWindow(
                                "Static", "Port:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                x, y, 80 - x, 16,
                                hwnd, 0, NULL, NULL
                        );

                        _snprintf(buffer, 63, "%d", Login_tcpport);
                        CreateWindow(
                                "Edit", buffer, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                80, y, 232, 24,
                                hwnd, (HMENU) 5, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, clrect.bottom - 64, 320, 32,
                                hwnd, (HMENU) 1, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Cancel", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, clrect.bottom - 32, 320, 32,
                                hwnd, (HMENU) 2, NULL, NULL
                        );
                } break;

                case WM_COMMAND: {
                        switch(HIWORD(wparam)) {
                                case BN_CLICKED:
                                        switch(LOWORD(wparam)) {
                                                char buffer[64];
                                            
                                                case 1:
                                                        GetDlgItemText(hwnd, 3, Login_targetip, sizeof(Login_targetip));
                                                        GetDlgItemText(hwnd, 5, buffer, 64);
                                                        sscanf(buffer, "%d", &Login_tcpport);
                                                        DestroyWindow(hwnd);
                                                break;

                                                case 2:
                                                        DestroyWindow(hwnd);
                                                break;
                                        }
                                break;
                        }
                } break;

                default:
                        return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        return 0;
}


void IPSetup_init(WNDCLASS *wc, HANDLE hi) {
        wc->hInstance = hi;
        wc->lpszClassName = IPSETUP_WINCLASS;
        wc->lpfnWndProc = IPSetup_WP;
        wc->hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc->hCursor = LoadCursor(0, IDC_ARROW);
        wc->hIcon = LoadIcon(hi, "APPICON");

        RegisterClass(wc);
}

void IPSetup_spawn(HANDLE hInst) {
        HWND hw;
        hw = CreateWindow(
                IPSETUP_WINCLASS, "AuroraChat 9x IP Setup",
                WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,
                NULL, NULL, hInst, NULL
        );
}

