#ifndef LOGIN_H
#define LOGIN_H

#include <windows.h>
#include <sockbuf.h>

extern int Login_main(HANDLE hInst, HANDLE hPrevInst, LPSTR cmdLine, int cmdshow);

extern char Login_targetip[64];
extern short Login_tcpport;

extern int Login_socket;
extern SOCKBUF_T Login_sockbuf;
extern char Login_servername[512];

#endif

