#include "credits.h"
#include "common.h"
#include "login.h"
#include <stdio.h>

const char CREDITS_WINCLASS[] = "AUC CREDITS";

HBITMAP Credits_creditbmp;

long PASCAL Credits_WP(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_DESTROY:
                break;

                case WM_CREATE: {
                        RECT clrect;
                        BITMAP bm;
                    
                        Credits_creditbmp = LoadBitmap(GetModuleHandle(NULL), "CREDITS");
                        if(Credits_creditbmp == NULL) {
                            PostQuitMessage(1);
                            return 0;
                        }

                        GetClientRect(hwnd, &clrect);
                        GetObject(Credits_creditbmp, sizeof(bm), &bm);

                        CreateWindow(
                                "Button", "All hail jakub!", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, bm.bmHeight, 320, clrect.bottom - bm.bmHeight,
                                hwnd, (HMENU) 1, NULL, NULL
                        );
                } break;

                case WM_PAINT: {
                        HDC hdc;
                        PAINTSTRUCT ps;
                        HDC hdcmem;
                        HBITMAP hbmold;
                        BITMAP bm;
                    
                        hdc = BeginPaint(hwnd, &ps);
                        hdcmem = CreateCompatibleDC(hdc);
                        hbmold = SelectObject(hdcmem, Credits_creditbmp);

                        GetObject(Credits_creditbmp, sizeof(bm), &bm);
                        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcmem, 0, 0, SRCCOPY);

                        SelectObject(hdcmem, hbmold);
                        DeleteDC(hdcmem);
                        EndPaint(hwnd, &ps);
                } break;

                case WM_COMMAND: {
                        switch(HIWORD(wparam)) {
                                case BN_CLICKED:
                                        switch(LOWORD(wparam)) {
                                                case 1:
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


void Credits_init(WNDCLASS *wc, HANDLE hi) {
        wc->hInstance = hi;
        wc->lpszClassName = CREDITS_WINCLASS;
        wc->lpfnWndProc = Credits_WP;
        wc->hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc->hCursor = LoadCursor(0, IDC_ARROW);
        wc->hIcon = LoadIcon(hi, "APPICON");

        RegisterClass(wc);
}

void Credits_spawn(HANDLE hInst) {
        HWND hw;
        hw = CreateWindow(
                CREDITS_WINCLASS, "AuroraChat 9x Credits",
                WS_OVERLAPPED | WS_CAPTION | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, 320, 240 + 64,
                NULL, NULL, hInst, NULL
        );
}

