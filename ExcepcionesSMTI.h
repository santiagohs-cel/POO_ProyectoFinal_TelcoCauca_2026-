#ifndef EXCEPCIONES_SMTI_H
#define EXCEPCIONES_SMTI_H

#include <stdexcept>
#include <string>

// Error grave de red (socket, bind, listen, accept...). Detiene el servidor.
class ErrorRedSMTI : public std::runtime_error {
public:
    explicit ErrorRedSMTI(const std::string& msg)
        : std::runtime_error("Error de red: " + msg) {}
};

// Trama con formato invalido. El servidor la rechaza pero sigue funcionando.
class ErrorFormatoTrama : public std::runtime_error {
public:
    explicit ErrorFormatoTrama(const std::string& msg)
        : std::runtime_error("Trama invalida: " + msg) {}
};

#endif
