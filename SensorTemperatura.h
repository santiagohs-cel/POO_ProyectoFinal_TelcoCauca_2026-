#ifndef SENSOR_TEMPERATURA_H
#define SENSOR_TEMPERATURA_H

#include "Sensor.h"

class SensorTemperatura : public Sensor {
private:
    double umbralMaximo;

public:
    SensorTemperatura(int _id, const std::string& _ub, double _lect, double _umbral = 80.0);
    void procesarLectura() override;
};

#endif
