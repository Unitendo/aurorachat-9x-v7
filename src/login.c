#include "login.h"
#include "common.h"
#include "v7.h"
#include "rooms.h"
#include "ipsetup.h"
#include "credits.h"
#include "rules.h"
#include <stdio.h>

int Login_socket;
SOCKBUF_T Login_sockbuf;
char Login_servername[512] = {0};
const char LOGIN_WINCLASS[] = "AUC LOGIN";
const char Login_versionstr[] = "9x v0.7.0";

char Login_targetip[64] = AUC_DEFAULTIP;
short Login_tcpport = AUC_DEFAULTTCPPORT;

#define LASTUSERFILE "lastuser.txt"

HBITMAP Login_logobmp;

int Login_done = 0;

int Login_startswith(char *s1, char *s2) {
        int l1, l2, i;
        l1 = strlen(s1);
        l2 = strlen(s2);
        if(l1 < l2) return 1;
        for(i=0;i<l2;i++) {
                if(s1[i] != s2[i]) return 1;
        }
        return 0;
}

void Login_saveUserData(const char *username, const char *password) {
        FILE *f = fopen(LASTUSERFILE, "w");
        if(f == NULL) return;

        fputs(username, f);
        fputc('\n', f);
        fputs(password, f);
        fputc('\n', f);
        fputs(Login_targetip, f);
        fputc('\n', f);
        fprintf(f, "%d\n", Login_tcpport);
        
        fclose(f);
}

long PASCAL Login_WP(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_DESTROY:
                        PostQuitMessage(0);
                break;

                case WM_CREATE: {
                        int x = 8;
                        int y = 128;
                        
                        FILE *lastuser;
                        char lastusername[1024] = {0};
                        char lastpassword[1024] = {0};

                        RECT clrect;
                        GetClientRect(hwnd, &clrect);

                        lastuser = fopen(LASTUSERFILE, "r");
                        if(lastuser) {
                                char buffer[64] = {0};
                            
                                fgets(lastusername, 1023, lastuser);
                                fgets(lastpassword, 1023, lastuser);
                                fgets(Login_targetip, 63, lastuser);

                                lastusername[strlen(lastusername)-1] = 0;
                                lastpassword[strlen(lastpassword)-1] = 0;
                                Login_targetip[strlen(Login_targetip)-1] = 0;

                                fgets(buffer, 63, lastuser);
                                sscanf(buffer, "%d", &Login_tcpport);

                                fclose(lastuser);
                        }

                        Login_logobmp = LoadBitmap(GetModuleHandle(NULL), "LOGO");
                        if(Login_logobmp == NULL) {
                            PostQuitMessage(1);
                            return 0;
                        }

                        CreateWindow(
                                "Static", "Username:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                x, y, 80 - x, 16,
                                hwnd, 0, NULL, NULL
                        );

                        CreateWindow(
                                "Edit", lastusername, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                80, y, 232, 24,
                                hwnd, (HMENU) 4, NULL, NULL
                        );

                        y += 24;

                        CreateWindow(
                                "Static", "Password:", WS_CHILD | WS_VISIBLE | SS_LEFT,
                                x, y, 80 - x, 16,
                                hwnd, 0, NULL, NULL
                        );

                        CreateWindow(
                                "Edit", lastpassword, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
                                80, y, 232, 24,
                                hwnd, (HMENU) 5, NULL, NULL
                        );

                        y += 32;

                        CreateWindow(
                                "Button", "Log in", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, y, 320, 32,
                                hwnd, (HMENU) 2, NULL, NULL
                        );

                        y += 32;

                        CreateWindow(
                                "Button", "Sign up", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, y, 320, 32,
                                hwnd, (HMENU) 3, NULL, NULL
                        );

                        y += 32;

                        CreateWindow(
                                "Button", "Server IP Setup", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, y, 320, 32,
                                hwnd, (HMENU) 7, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "9x Client by JakubKwantowy", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                0, clrect.bottom - 32, 320, 32,
                                hwnd, (HMENU) 6, NULL, NULL
                        );

                        CreateWindow(
                                "Static", Login_versionstr, WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                0, clrect.bottom - 48, clrect.right, 16,
                                hwnd, 0, NULL, NULL
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
                        hbmold = SelectObject(hdcmem, Login_logobmp);

                        GetObject(Login_logobmp, sizeof(bm), &bm);
                        BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcmem, 0, 0, SRCCOPY);

                        SelectObject(hdcmem, hbmold);
                        DeleteDC(hdcmem);
                        EndPaint(hwnd, &ps);
                } break;

                case WM_COMMAND: {
                        switch(HIWORD(wparam)) {
                                case BN_CLICKED:
                                        switch(LOWORD(wparam)) {
                                                char username[1024] = {0};
                                                char password[1024] = {0};
                                                char errmsg[1024] = {0};
                                                char banmsg[1024] = {0};
                                                int err;
                                            
                                                case 2:
                                                        GetDlgItemText(hwnd, 4, username, 1023);
                                                        GetDlgItemText(hwnd, 5, password, 1023);
                                                        SetDlgItemText(hwnd, 5, "");
                                                        Login_socket = socket_create(Login_targetip, Login_tcpport);
                                                        if(Login_socket == -1)
                                                                break;
                                                        sockbuf_init(&Login_sockbuf, Login_socket);
                                                        if(v7_waitforhello(&Login_sockbuf, Login_servername, 512))
                                                                break;
                                                        v7_loginOrRegister(Login_socket, "login", username, password);
                                                        err = v7_loginOKCheck(&Login_sockbuf, errmsg, banmsg, 1024, 1024);
                                                        if(err == 1) break;
                                                        if(err == 2) {
                                                                MessageBox(hwnd, "Login Error, is this a v7 server?", "Login Error", MB_ICONERROR);
                                                                break;
                                                        }
                                                        if(err == 3) {
                                                                char buffer[4096];
                                                                _snprintf(buffer, 4096, "Login Error: %s", errmsg);
                                                                MessageBox(hwnd, buffer, "Login Error", MB_ICONERROR);
                                                                break;
                                                        }
                                                        if(err == 4) {
                                                                char buffer[4096];
                                                                _snprintf(buffer, 4096, "Ban reason: %s", banmsg);
                                                                MessageBox(hwnd, buffer, "You were banned! (lmao)", MB_ICONERROR);
                                                                break;
                                                        }
                                                        Login_done = 1;
                                                        Login_saveUserData(username, password);
                                                        DestroyWindow(hwnd);
                                                break;

                                                case 3:
                                                        GetDlgItemText(hwnd, 4, username, 1023);
                                                        GetDlgItemText(hwnd, 5, password, 1023);
                                                        SetDlgItemText(hwnd, 5, "");
                                                        Login_socket = socket_create(Login_targetip, Login_tcpport);
                                                        if(Login_socket == -1)
                                                                break;
                                                        sockbuf_init(&Login_sockbuf, Login_socket);
                                                        if(v7_waitforhello(&Login_sockbuf, Login_servername, 512))
                                                                break;
                                                        v7_loginOrRegister(Login_socket, "register", username, password);
                                                        err = v7_loginOKCheck(&Login_sockbuf, errmsg, banmsg, 1024, 1024);
                                                        if(err == 1) break;
                                                        if(err == 2) {
                                                                MessageBox(hwnd, "Login Error, is this a v7 server?", "Login Error", MB_ICONERROR);
                                                                break;
                                                        }
                                                        if(err == 3) {
                                                                char buffer[4096];
                                                                _snprintf(buffer, 4096, "Login Error: %s", errmsg);
                                                                MessageBox(hwnd, buffer, "Login Error", MB_ICONERROR);
                                                                break;
                                                        }
                                                        if(err == 4) {
                                                                char buffer[4096];
                                                                _snprintf(buffer, 4096, "Ban reason: %s", banmsg);
                                                                MessageBox(hwnd, buffer, "You were banned! (lmao)", MB_ICONERROR);
                                                                break;
                                                        }
                                                        Login_done = 1;
                                                        Login_saveUserData(username, password);
                                                        DestroyWindow(hwnd);
                                                break;

                                                case 6:
                                                        Credits_spawn(GetModuleHandle(NULL));
                                                break;

                                                case 7:
                                                        IPSetup_spawn(GetModuleHandle(NULL));
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

void Login_createWC(WNDCLASS *wc, HANDLE hi) {
        wc->hInstance = hi;
        wc->lpszClassName = LOGIN_WINCLASS;
        wc->lpfnWndProc = Login_WP;
        wc->hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc->hCursor = LoadCursor(0, IDC_ARROW);
        wc->hIcon = LoadIcon(hi, "APPICON");

        RegisterClass(wc);
}

int Login_main(HANDLE hInst, HANDLE hPrevInst, LPSTR cmdLine, int cmdshow) {
        WNDCLASS wc = {0};
        WNDCLASS wc_rooms = {0};
        WNDCLASS wc_ip = {0};
        WNDCLASS wc_cred = {0};
        WNDCLASS wc_rules = {0};
        HWND hw;
        MSG msg = {0};

        Rooms_init(&wc_rooms, hInst);
        IPSetup_init(&wc_ip, hInst);
        Credits_init(&wc_cred, hInst);
        Rules_init(&wc_rules, hInst);

        Login_createWC(&wc, hInst);
        hw = CreateWindow(
                LOGIN_WINCLASS, "AuroraChat 9x Login",
                WS_AUC,
                CW_USEDEFAULT, CW_USEDEFAULT, 320, 480,
                NULL, NULL, hInst, NULL
        );

        if(hw == NULL) return 1;

        while(GetMessage(&msg, NULL, 0, 0) > 0) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        if(Login_done) Rooms_main(hInst);

        return 0;
}

