#include "SensorCaudal.h"

SensorCaudal::SensorCaudal(int _id, const std::string& _ub, double _lect, double _min)
    : Sensor(_id, _ub, _lect), caudalMinimo(_min) {}

void SensorCaudal::procesarLectura() {
    std::cout << "[CAUDAL POLIMORFICO] " << *this << " L/min";
    if (lectura < caudalMinimo) {
        std::cout << " --> ALERTA: CAUDAL BAJO!";
    }
    std::cout << std::endl;
}
