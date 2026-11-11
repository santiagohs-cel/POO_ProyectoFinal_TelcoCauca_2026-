// =====================================================================
//  SMTI - Servidor TCP de Telemetria (Windows y Linux, C++11)
//  Al ejecutar: arranca el servidor y, en paralelo, un cliente de prueba
//  que le envia varias tramas. Windows: enlazar con -lws2_32.
// =====================================================================
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>

#include "PlataformaRed.h"
#include "Sensor.h"
#include "ListaGen.h"
#include "ServidorSockets.h"
#include "ExcepcionesSMTI.h"

static const int PUERTO_SMTI = 8080;

// ---------------------------------------------------------------------
// Cliente de prueba
// ---------------------------------------------------------------------
static void enviarTrama(int puerto, const std::string& mensaje) {
    sock_t fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == SOCK_INVALIDO) throw std::runtime_error("Cliente: no se pudo crear el socket.");

    sockaddr_in serv;
    std::memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_port = htons(puerto);
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, reinterpret_cast<sockaddr*>(&serv), sizeof(serv)) < 0) {
        CERRAR_SOCK(fd);
        throw std::runtime_error("Cliente: fallo la conexion con el servidor.");
    }

    send(fd, mensaje.c_str(), static_cast<int>(mensaje.length()), BANDERA_ENVIO);

    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));
    int n = recv(fd, buffer, static_cast<int>(sizeof(buffer)) - 1, 0);
    if (n > 0) std::cout << "   [CLIENTE] Respuesta: " << buffer << std::endl;
    CERRAR_SOCK(fd);
}

static void correrCliente(int puerto) {
    const char* tramas[] = {
        "TEMPERATURA;101;92.5;Caldera-01",
        "TEMPERATURA;102;65.0;Horno-02",
        "PRESION;201;12.3;Tuberia-7",
        "CAUDAL;301;3.2;Linea-A",
        "ROBOT;1;5",                 // tipo desconocido -> error controlado
        "FIN"
    };
    try {
        for (size_t i = 0; i < sizeof(tramas) / sizeof(tramas[0]); ++i) {
#ifdef _WIN32
            Sleep(300);
#else
            usleep(300000);
#endif
            enviarTrama(puerto, tramas[i]);
        }
    } catch (const std::exception& e) {
        std::cerr << "[EXCEPCION CLIENTE] " << e.what() << std::endl;
    }
}

// ---------------------------------------------------------------------
// Lanzar el cliente en paralelo (Windows: CreateThread / Linux: fork)
// ---------------------------------------------------------------------
#ifdef _WIN32
    typedef HANDLE ClienteH;
    static const ClienteH CLIENTE_NINGUNO = NULL;
    static DWORD WINAPI hiloEntrada(LPVOID) { correrCliente(PUERTO_SMTI); return 0; }
    static ClienteH lanzarCliente() {
        return CreateThread(NULL, 0, hiloEntrada, NULL, 0, NULL);
    }
    static void esperarCliente(ClienteH h) {
        if (h != NULL) { WaitForSingleObject(h, INFINITE); CloseHandle(h); }
    }
#else
    typedef pid_t ClienteH;
    static const ClienteH CLIENTE_NINGUNO = -1;
    static ClienteH lanzarCliente() {
        pid_t pid = fork();
        if (pid == 0) { correrCliente(PUERTO_SMTI); _exit(0); }
        return pid;
    }
    static void esperarCliente(ClienteH h) {
        if (h > 0) waitpid(h, NULL, 0);
    }
#endif

// ---------------------------------------------------------------------
// MAIN
// ---------------------------------------------------------------------
int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << " SMTI: SERVIDOR DE TELEMETRIA DISTRIBUIDA (UNIDAD 9)" << std::endl;
    std::cout << "==================================================" << std::endl;

    ClienteH cliente = CLIENTE_NINGUNO;

    try {
        InicioRed red;                  // WSAStartup en Windows
        ListaGen<Sensor*> listaPlanta;  // su destructor libera el Heap siempre
        ServidorSMTI servidor(PUERTO_SMTI);
        servidor.iniciar();

        cliente = lanzarCliente();

        servidor.atender(listaPlanta);  // termina cuando llega la trama "FIN"
        esperarCliente(cliente);
        cliente = CLIENTE_NINGUNO;

        std::cout << "\n--- PROCESAMIENTO POLIMORFICO DE TELEMETRIA ("
                  << listaPlanta.tamano() << " sensores) ---" << std::endl;
        listaPlanta.procesarTodo();

        Sensor* s = listaPlanta.buscarPorId(101);
        if (s != nullptr) std::cout << "\nBusqueda por ID 101 -> " << *s << std::endl;

        std::cout << "\n--- Liberando memoria del Heap ---" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[EXCEPCION GRAVE SMTI] " << e.what() << std::endl;
        esperarCliente(cliente);
        return 1;
    }
    return 0;   // ~ListaGen() ya libero todos los sensores
}
