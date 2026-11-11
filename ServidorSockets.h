#ifndef SERVIDOR_SOCKETS_H
#define SERVIDOR_SOCKETS_H

#include <string>
#include "PlataformaRed.h"
#include "Sensor.h"
#include "ListaGen.h"
#include "ExcepcionesSMTI.h"

// Trama industrial ya interpretada
struct MuestraSensor {
    int idSensor;
    double valor;
    char tipo[12];
};

// Servidor TCP de telemetria.
// Formato de trama (texto):  TIPO;ID;VALOR[;UBICACION]
//   Ejemplos: TEMPERATURA;101;92.5;Caldera-01
//             PRESION;201;12.3
//             CAUDAL;301;3.2;Linea-A
// La trama especial "FIN" cierra el servidor.
class ServidorSMTI {
private:
    sock_t server_fd;
    int puerto;

    ServidorSMTI(const ServidorSMTI&) = delete;
    ServidorSMTI& operator=(const ServidorSMTI&) = delete;

public:
    explicit ServidorSMTI(int p);
    ~ServidorSMTI();

    // socket() + bind() + listen(). Lanza ErrorRedSMTI si algo falla.
    void iniciar();

    // Ciclo accept()/recv(): crea sensores en el Heap y los inserta en la lista.
    void atender(ListaGen<Sensor*>& lista);

    static MuestraSensor parsearTrama(const std::string& trama, std::string& ubicacion);
    static Sensor* crearSensor(const MuestraSensor& m, const std::string& ubicacion);
};

#endif
