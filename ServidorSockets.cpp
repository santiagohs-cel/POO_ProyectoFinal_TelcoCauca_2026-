#include "ServidorSockets.h"
#include "SensorTemperatura.h"
#include "SensorPresion.h"
#include "SensorCaudal.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

std::string recortar(const std::string& s) {
    size_t ini = 0, fin = s.size();
    while (ini < fin && std::isspace(static_cast<unsigned char>(s[ini]))) ++ini;
    while (fin > ini && std::isspace(static_cast<unsigned char>(s[fin - 1]))) --fin;
    return s.substr(ini, fin - ini);
}

std::string aMayusculas(std::string s) {
    for (size_t i = 0; i < s.size(); ++i)
        s[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[i])));
    return s;
}

} // namespace

ServidorSMTI::ServidorSMTI(int p) : server_fd(SOCK_INVALIDO), puerto(p) {}

ServidorSMTI::~ServidorSMTI() {
    if (server_fd != SOCK_INVALIDO) {
        CERRAR_SOCK(server_fd);
        server_fd = SOCK_INVALIDO;
    }
}

void ServidorSMTI::iniciar() {
    // 1. socket() TCP IPv4
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == SOCK_INVALIDO) {
        throw ErrorRedSMTI("No se pudo instanciar el socket.");
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    // 2. Direccion
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(puerto);

    // 3. bind()
    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        CERRAR_SOCK(server_fd);
        server_fd = SOCK_INVALIDO;
        throw ErrorRedSMTI("Enlace (bind) fallido en puerto " + std::to_string(puerto) +
                           " (puerto ocupado o sin permisos).");
    }

    // 4. listen()
    if (listen(server_fd, 5) < 0) {
        CERRAR_SOCK(server_fd);
        server_fd = SOCK_INVALIDO;
        throw ErrorRedSMTI("Fallo la operacion listen().");
    }

    std::cout << "[SERVIDOR SMTI] Escuchando telemetria en puerto " << puerto
              << "..." << std::endl;
}

MuestraSensor ServidorSMTI::parsearTrama(const std::string& trama, std::string& ubicacion) {
    std::vector<std::string> campos;
    std::stringstream ss(trama);
    std::string campo;
    while (std::getline(ss, campo, ';')) {
        campos.push_back(recortar(campo));
    }

    if (campos.size() < 3 || campos.size() > 4) {
        throw ErrorFormatoTrama("se esperaba TIPO;ID;VALOR[;UBICACION] y llego '" + trama + "'");
    }

    MuestraSensor m;
    std::memset(&m, 0, sizeof(m));

    std::string tipo = aMayusculas(campos[0]);
    if (tipo.empty() || tipo.size() >= sizeof(m.tipo)) {
        throw ErrorFormatoTrama("tipo vacio o demasiado largo '" + campos[0] + "'");
    }
    std::strncpy(m.tipo, tipo.c_str(), sizeof(m.tipo) - 1);

    char* fin = nullptr;
    long id = std::strtol(campos[1].c_str(), &fin, 10);
    if (campos[1].empty() || *fin != '\0' || id < 0) {
        throw ErrorFormatoTrama("ID no valido '" + campos[1] + "'");
    }
    m.idSensor = static_cast<int>(id);

    m.valor = std::strtod(campos[2].c_str(), &fin);
    if (campos[2].empty() || *fin != '\0') {
        throw ErrorFormatoTrama("valor no numerico '" + campos[2] + "'");
    }

    if (campos.size() == 4 && !campos[3].empty()) {
        ubicacion = campos[3];
    } else {
        ubicacion = "Nodo-" + std::to_string(m.idSensor);
    }
    return m;
}

Sensor* ServidorSMTI::crearSensor(const MuestraSensor& m, const std::string& ubicacion) {
    std::string tipo(m.tipo);
    // Instanciacion dinamica en el Heap
    if (tipo == "TEMPERATURA") return new SensorTemperatura(m.idSensor, ubicacion, m.valor);
    if (tipo == "PRESION")     return new SensorPresion(m.idSensor, ubicacion, m.valor);
    if (tipo == "CAUDAL")      return new SensorCaudal(m.idSensor, ubicacion, m.valor);
    throw ErrorFormatoTrama("tipo de sensor desconocido '" + tipo + "'");
}

void ServidorSMTI::atender(ListaGen<Sensor*>& lista) {
    if (server_fd == SOCK_INVALIDO) {
        throw ErrorRedSMTI("El servidor no ha sido iniciado (llame a iniciar()).");
    }

    bool continuar = true;
    while (continuar) {
        // 5. accept(): bloquea hasta que llega un cliente
        sockaddr_in cliente_addr;
        lenSock addr_len = sizeof(cliente_addr);
        sock_t client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&cliente_addr), &addr_len);
        if (client_fd == SOCK_INVALIDO) {
            throw ErrorRedSMTI("No se pudo aceptar la conexion del cliente.");
        }

        std::cout << "[SERVIDOR SMTI] Cliente conectado." << std::endl;
        ponerTimeout(client_fd);

        // 6. recv()
        char buffer[1024];
        std::memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_fd, buffer, static_cast<int>(sizeof(buffer)) - 1, 0);

        if (bytes <= 0) {
            std::cerr << "[SERVIDOR SMTI] Cliente sin datos (cerrado o timeout)." << std::endl;
            CERRAR_SOCK(client_fd);
            continue;
        }

        std::string trama = recortar(std::string(buffer, static_cast<size_t>(bytes)));
        std::cout << "[TRAMA RECIBIDA] " << trama << std::endl;

        std::string respuesta;
        if (aMayusculas(trama) == "FIN") {
            respuesta = "ACK SMTI: servidor finalizando.";
            continuar = false;
        } else {
            try {
                std::string ubicacion;
                MuestraSensor m = parsearTrama(trama, ubicacion);
                Sensor* nuevo = crearSensor(m, ubicacion);
                lista.insertar(nuevo);
                respuesta = "ACK SMTI: sensor " + std::to_string(m.idSensor) + " registrado.";
            } catch (const ErrorFormatoTrama& e) {
                // Trama mala: se informa al cliente, el servidor sigue vivo
                std::cerr << "[SERVIDOR SMTI] " << e.what() << std::endl;
                respuesta = std::string("ERROR SMTI: ") + e.what();
            }
        }

        send(client_fd, respuesta.c_str(), static_cast<int>(respuesta.length()), BANDERA_ENVIO);
        CERRAR_SOCK(client_fd);
    }
}
