#ifndef PLATAFORMA_RED_H
#define PLATAFORMA_RED_H

// Diferencias entre Windows (Winsock) y Linux (POSIX) para los sockets.
// El resto del proyecto usa solo estos nombres (sock_t, CERRAR_SOCK, ...).

#include <stdexcept>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET sock_t;
    typedef int    lenSock;
    #define SOCK_INVALIDO INVALID_SOCKET
    #define CERRAR_SOCK(s) closesocket(s)
    #define BANDERA_ENVIO 0
#else
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    typedef int       sock_t;
    typedef socklen_t lenSock;
    #define SOCK_INVALIDO (-1)
    #define CERRAR_SOCK(s) close(s)
    #define BANDERA_ENVIO MSG_NOSIGNAL
#endif

// En Windows hay que "encender" Winsock antes de usar sockets (no hace nada en Linux)
struct InicioRed {
    InicioRed() {
#ifdef _WIN32
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            throw std::runtime_error("No se pudo iniciar Winsock (WSAStartup).");
#endif
    }
    ~InicioRed() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

// Timeout de lectura de 5 s para recv(): un cliente mudo no bloquea el servidor
inline void ponerTimeout(sock_t fd) {
#ifdef _WIN32
    DWORD ms = 5000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof(ms));
#else
    timeval tv; tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
}

#endif
