#include "SensorPresion.h"

SensorPresion::SensorPresion(int _id, const std::string& _ub, double _lect, double _max)
    : Sensor(_id, _ub, _lect), presionMaxima(_max) {}

void SensorPresion::procesarLectura() {
    std::cout << "[PRESION POLIMORFICO] " << *this << " bar";
    if (lectura > presionMaxima) {
        std::cout << " --> ALERTA: SOBREPRESION!";
    }
    std::cout << std::endl;
}
