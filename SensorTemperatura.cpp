#include "SensorTemperatura.h"

SensorTemperatura::SensorTemperatura(int _id, const std::string& _ub, double _lect, double _umbral)
    : Sensor(_id, _ub, _lect), umbralMaximo(_umbral) {}

void SensorTemperatura::procesarLectura() {
    std::cout << "[TEMP POLIMORFICO] " << *this << " C";
    if (lectura > umbralMaximo) {
        std::cout << " --> ALERTA: SOBRETEMPERATURA CRITICA!";
    }
    std::cout << std::endl;
}
