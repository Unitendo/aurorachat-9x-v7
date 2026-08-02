#include <sockets.h>
#include <sys/types.h>
#include <winsock.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int socket_create(const char *ip, short port) {
    struct sockaddr_in addr;
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) return -1;

    addr.sin_addr.s_addr = inet_addr(ip);
    if(addr.sin_addr.s_addr == INADDR_NONE) return -1;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if(connect(s, (const struct sockaddr *) &addr, sizeof(addr))) return -1;
    
    return s;
}

void socket_destroy(int s) {
    close(s);
}

const char *socket_error() {
    return strerror(errno);
}

size_t socket_recv(int s, void *b, size_t n) {
    return recv(s, b, n, 0);
}

size_t socket_recv_nonblock(int s, void *b, size_t n) {
    u_long bufsize;
    ioctlsocket(s, FIONREAD, &bufsize);
    if(!bufsize) return 0;
    return recv(s, b, n, 0);
}

size_t socket_send(int s, const void *b, size_t n) {
    return send(s, b, n, 0);
}

int socket_wouldveblocked() {
    if(errno == EAGAIN) return 1;
    // if(errno == EWOULDBLOCK) return 1;
    return 0;
}

int socket_stillalive(int s) {
    int err = 0;
    int err_size = sizeof(err);
    getsockopt(s, SOL_SOCKET, SO_ERROR, (char *) &err, &err_size);
    return !err;
}
