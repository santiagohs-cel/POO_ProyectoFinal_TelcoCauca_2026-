#ifndef SENSOR_H
#define SENSOR_H

#include <iostream>
#include <string>

// Clase base ABSTRACTA (tiene un metodo virtual puro)
class Sensor {
protected:
    int id;
    std::string ubicacion;
    double lectura;

public:
    Sensor(int _id, const std::string& _ub, double _lect);

    // Destructor virtual: obliga a llamar al destructor de la clase hija
    // cuando se hace "delete" sobre un Sensor* (evita Memory Leaks).
    virtual ~Sensor();

    int getId() const { return id; }
    double getLectura() const { return lectura; }

    // Metodo virtual puro -> convierte a Sensor en clase abstracta
    virtual void procesarLectura() = 0;

    // Sobrecarga de operador amiga
    friend std::ostream& operator<<(std::ostream& os, const Sensor& s);
};

#endif
