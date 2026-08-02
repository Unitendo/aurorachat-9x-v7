#include "rooms.h"
#include "common.h"
#include "v7.h"
#include "login.h"
#include "rules.h"
#include <stdio.h>
#include <string.h>
#include <winsock.h>

#define DEFAULTROOMNAME "general"
#define MSGBUFSIZE 16384
#define TIMERINTERVAL 500

char Rooms_roombuf[ROOMS_ROOMBUFSIZE] = {0};
char Rooms_roomname[ROOMS_ROOMBUFSIZE] = DEFAULTROOMNAME;
const char ROOMS_WINCLASS[] = "AUC ROOMS";
char Rooms_messagebuffer[MSGBUFSIZE + 1] = {0};
size_t Rooms_lastmsgbuflen = 0;

WNDPROC Rooms_OldMsgEditBoxProc;

long PASCAL Rooms_MsgEditBoxProc(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_KEYDOWN:
                        switch(wparam) {
                                char message[4096];
                            
                                case VK_RETURN:
                                        GetWindowText(hwnd, message, 4095);
                                        SetWindowText(hwnd, "");
                                        v7_sendMsg(Login_socket, message);
                                break;
                        }
            
                default:
                        return CallWindowProc(Rooms_OldMsgEditBoxProc, hwnd, msg, wparam, lparam);
        }

        return 0;
}

long PASCAL Rooms_WP(HWND hwnd, unsigned msg, UINT wparam, LONG lparam) {
        switch(msg) {
                case WM_DESTROY:
                        PostQuitMessage(0);
                break;

                case WM_CREATE: {
                        HWND hmsginput;
                        RECT clrect;
                        GetClientRect(hwnd, &clrect);

                        CreateWindow(
                                "Edit", NULL,
                                WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                                0, 32, clrect.right, clrect.bottom - 64,
                                hwnd, (HMENU) 1, NULL, NULL
                        );

                        CreateWindow(
                                "Static", "#", WS_CHILD | WS_VISIBLE | SS_RIGHT,
                                0, 4, 8, 16,
                                hwnd, 0, NULL, NULL
                        );

                        CreateWindow(
                                "Edit", DEFAULTROOMNAME, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                8, 0, clrect.right - (128 + 64 + 8), 24,
                                hwnd, (HMENU) 2, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Join Room", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - (128 + 64), 0, 128, 24,
                                hwnd, (HMENU) 3, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Clear", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - 64, 0, 64, 24,
                                hwnd, (HMENU) 7, NULL, NULL
                        );

                        hmsginput = CreateWindow(
                                "Edit", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER,
                                0, clrect.bottom - 24, clrect.right - 64, 24,
                                hwnd, (HMENU) 5, NULL, NULL
                        );

                        CreateWindow(
                                "Button", "Send", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                                clrect.right - 64, clrect.bottom - 24, 64, 24,
                                hwnd, (HMENU) 6, NULL, NULL
                        );

                        Rooms_OldMsgEditBoxProc = (WNDPROC) SetWindowLong(hmsginput, GWL_WNDPROC, (LONG) Rooms_MsgEditBoxProc);

                        SetTimer(hwnd, 1337, TIMERINTERVAL, NULL);
                } break;

                case WM_COMMAND: {
                        switch(HIWORD(wparam)) {
                                case BN_CLICKED:
                                        switch(LOWORD(wparam)) {
                                                case 3: {
                                                        GetDlgItemText(hwnd, 2, Rooms_roomname, ROOMS_ROOMBUFSIZE - 1);
                                                        strncpy(Rooms_messagebuffer, "Auc9x: Joined #", MSGBUFSIZE);
                                                        strncat(Rooms_messagebuffer, Rooms_roomname, MSGBUFSIZE);
                                                        strncat(Rooms_messagebuffer, ".\r\n", MSGBUFSIZE);
                                                        v7_joinRoom(Login_socket, Rooms_roomname);
                                                } break;

                                                case 6: {
                                                        char message[4096];
                                                        GetDlgItemText(hwnd, 5, message, 4095);
                                                        SetDlgItemText(hwnd, 5, "");
                                                        v7_sendMsg(Login_socket, message);
                                                } break;

                                                case 7: {
                                                        Rooms_messagebuffer[0] = 0;
                                                } break;
                                        }
                                break;
                        }
                } break;

                case WM_TIMER: {
                        switch(wparam) {
                                case 1337: {
                                        char buffer[4096] = {0};
                                        HWND edit;
                                        DWORD startsel, endsel;

                                        size_t recvd;
                                        while( (recvd = sockbuf_getline_nonblock(&Login_sockbuf, buffer, sizeof(buffer) - 1)) ) {
                                                char *token;
                                                char author[64] = {0};
                                                char content[1024] = {0};
                                                char msgbuf[2048] = {0};
                                                size_t count, left, i;
                                                if(recvd == -1)
                                                        break;

                                                buffer[recvd] = 0;
                                                token = strtok(buffer, "|");

                                                if(token == NULL) continue;
                                                if(strncmp(token, "msg", sizeof(buffer))) 
                                                        continue;

                                                token = strtok(NULL, "|");
                                                if(token == NULL) continue;
                                                v7_decode(author, token, sizeof(author));
                                                token = strtok(NULL, "|");
                                                if(token == NULL) continue;
                                                v7_decode(content, token, sizeof(content));
                                                _snprintf(msgbuf, sizeof(msgbuf) - 1, "<%s> %s\r\n", author, content);
                                                count = strlen(Rooms_messagebuffer);
                                                left = ROOMS_ROOMBUFSIZE - count;
                                                for(i=0;i<left && msgbuf[i];i++) {
                                                        Rooms_messagebuffer[i + count] = msgbuf[i];
                                                }
                                                Rooms_messagebuffer[i + count] = 0;
                                                left = ROOMS_ROOMBUFSIZE - strlen(Rooms_roombuf);
                                                if(left <= (ROOMS_ROOMBUFSIZE / 4)) {
                                                        int i;
                                                        int m = (MSGBUFSIZE / 4);
                                                        int n = strlen(Rooms_messagebuffer) - m;
                                                        for(i=0;i<m;i++) {
                                                                Rooms_messagebuffer[i] = Rooms_messagebuffer[i + n];
                                                        }
                                                        Rooms_messagebuffer[m] = 0;
                                                }       
                                        }

                                        edit = GetDlgItem(hwnd, 1);
                                        if(strlen(Rooms_messagebuffer) != Rooms_lastmsgbuflen) {
                                                SendMessage(edit, EM_GETSEL, (WPARAM) &startsel, (LPARAM) &endsel);
                                                SetDlgItemText(hwnd, 1, Rooms_messagebuffer);
                                                SendMessage(edit, EM_SETSEL, startsel, endsel);
                                                SendMessage(edit, EM_SCROLLCARET, 0, 0);
                                                Rooms_lastmsgbuflen = strlen(Rooms_messagebuffer);
                                        }
                                } break;
                        }
                } break;

                default:
                        return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        return 0;
}

void Rooms_init(WNDCLASS *wc, HANDLE hi) {
        wc->hInstance = hi;
        wc->lpszClassName = ROOMS_WINCLASS;
        wc->lpfnWndProc = Rooms_WP;
        wc->hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc->hCursor = LoadCursor(0, IDC_ARROW);
        wc->hIcon = LoadIcon(hi, "APPICON");
        
        RegisterClass(wc);
}

int Rooms_main(HANDLE hInst) {
        HWND hw;
        MSG msg = {0};

        Rules_main(hInst);

        hw = CreateWindow(
                ROOMS_WINCLASS, "AuroraChat 9x",
                WS_AUC,
                CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
                NULL, NULL, hInst, NULL
        );

        if(hw == NULL) return 1;
        v7_joinRoom(Login_socket, "general");

        while(GetMessage(&msg, NULL, 0, 0) > 0) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
        }

        return 0;
}

