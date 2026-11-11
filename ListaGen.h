#ifndef LISTAGEN_H
#define LISTAGEN_H

#include <iostream>
#include <stdexcept>

template <typename T>
struct NodoGen {
    T dato;
    NodoGen<T>* siguiente;
    NodoGen(T _d) : dato(_d), siguiente(nullptr) {}
};

// Lista enlazada generica. IMPORTANTE: T debe ser un PUNTERO a un objeto
// creado con "new" (ej. ListaGen<Sensor*>), porque el destructor hace delete
// sobre cada dato.
template <typename T>
class ListaGen {
private:
    NodoGen<T>* head;
    int cantidad;

    // La lista es duena de los objetos: se prohibe copiarla (evita doble delete)
    ListaGen(const ListaGen&) = delete;
    ListaGen& operator=(const ListaGen&) = delete;

public:
    ListaGen() : head(nullptr), cantidad(0) {}

    // Destructor generico: purga total del Heap
    ~ListaGen() {
        NodoGen<T>* actual = head;
        while (actual != nullptr) {
            NodoGen<T>* aux = actual->siguiente;
            delete actual->dato;   // libera el objeto apuntado (Sensor*)
            delete actual;         // libera el nodo
            actual = aux;
        }
        head = nullptr;
        cantidad = 0;
    }

    // Insercion al inicio
    void insertar(T nuevoDato) {
        NodoGen<T>* nuevo = new NodoGen<T>(nuevoDato);
        nuevo->siguiente = head;
        head = nuevo;
        ++cantidad;
    }

    // Recorrido polimorfico (dynamic binding via vtable)
    void procesarTodo() {
        NodoGen<T>* actual = head;
        while (actual != nullptr) {
            actual->dato->procesarLectura();
            actual = actual->siguiente;
        }
    }

    // Busqueda por ID. Devuelve nullptr si no existe.
    T buscarPorId(int id) const {
        NodoGen<T>* actual = head;
        while (actual != nullptr) {
            if (actual->dato->getId() == id) return actual->dato;
            actual = actual->siguiente;
        }
        return nullptr;
    }

    int tamano() const { return cantidad; }
    bool vacia() const { return head == nullptr; }
};

#endif
