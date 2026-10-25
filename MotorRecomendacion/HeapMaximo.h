#pragma once
#include "Entidades.h"
#include <stdexcept>
#include <utility>

// Heap maximo implementado sobre un arreglo dinamico propio.
class HeapMaximo {
    Candidato* datos_;
    int cantidad_;
    int capacidad_;
    static bool mayor(const Candidato& a, const Candidato& b) {
        if (a.puntuacion != b.puntuacion) return a.puntuacion > b.puntuacion;
        return a.idContenido < b.idContenido; // desempate determinista
    }
    void ampliar() {
        int nueva = capacidad_ * 2;
        Candidato* elementos = new Candidato[nueva];
        for (int i = 0; i < cantidad_; ++i) elementos[i] = datos_[i];
        delete[] datos_;
        datos_ = elementos;
        capacidad_ = nueva;
    }
    void subir(int i) {
        while (i > 0) {
            int padre = (i - 1) / 2;
            if (!mayor(datos_[i], datos_[padre])) break;
            std::swap(datos_[i], datos_[padre]);
            i = padre;
        }
    }
    void bajar(int i) {
        while (true) {
            int izquierdo = 2 * i + 1;
            int derecho = 2 * i + 2;
            int maximo = i;
            if (izquierdo < cantidad_ && mayor(datos_[izquierdo], datos_[maximo]))
                maximo = izquierdo;
            if (derecho < cantidad_ && mayor(datos_[derecho], datos_[maximo]))
                maximo = derecho;
            if (maximo == i) break;
            std::swap(datos_[i], datos_[maximo]);
            i = maximo;
        }
    }
public:
    explicit HeapMaximo(int capacidad = 8)
        : datos_(nullptr), cantidad_(0), capacidad_(capacidad) {
        if (capacidad < 1) throw std::invalid_argument("Capacidad invalida");
        datos_ = new Candidato[capacidad_];
    }
    HeapMaximo(const HeapMaximo&) = delete;
    HeapMaximo& operator=(const HeapMaximo&) = delete;
    ~HeapMaximo() { delete[] datos_; }
    bool vacio() const { return cantidad_ == 0; }
    int cantidad() const { return cantidad_; }
    void insertar(const Candidato& candidato) {
        if (cantidad_ == capacidad_) ampliar();
        datos_[cantidad_] = candidato;
        subir(cantidad_);
        ++cantidad_;
    }
    Candidato extraerMaximo() {
        if (vacio()) throw std::out_of_range("Heap vacio");
        Candidato resultado = datos_[0];
        --cantidad_;
        if (cantidad_ > 0) {
            datos_[0] = datos_[cantidad_];
            bajar(0);
        }
        return resultado;
    }
};
