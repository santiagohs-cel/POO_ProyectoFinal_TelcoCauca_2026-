#ifndef SENSOR_PRESION_H
#define SENSOR_PRESION_H

#include "Sensor.h"

class SensorPresion : public Sensor {
private:
    double presionMaxima; // en bar

public:
    SensorPresion(int _id, const std::string& _ub, double _lect, double _max = 10.0);
    void procesarLectura() override;
};

#endif
