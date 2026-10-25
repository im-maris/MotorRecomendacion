#pragma once
#include "Utilidades.h"
#include <string>
#include <stdexcept>
#include <utility>

// Arreglo dinamico propio para preferencias y etiquetas.
// No reemplaza ninguna de las cinco estructuras obligatorias.
class ListaTextos {
    std::string* elementos_;
    int cantidad_;
    int capacidad_;

    void ampliar() {
        int nuevaCapacidad = capacidad_ == 0 ? 4 : capacidad_ * 2;
        std::string* nuevos = new std::string[nuevaCapacidad];
        for (int i = 0; i < cantidad_; ++i) nuevos[i] = elementos_[i];
        delete[] elementos_;
        elementos_ = nuevos;
        capacidad_ = nuevaCapacidad;
    }
public:
    ListaTextos() : elementos_(nullptr), cantidad_(0), capacidad_(0) {}
    ~ListaTextos() { delete[] elementos_; }
    ListaTextos(const ListaTextos& otro) : ListaTextos() {
        for (int i = 0; i < otro.cantidad_; ++i) agregar(otro.elementos_[i]);
    }
    ListaTextos& operator=(const ListaTextos& otro) {
        if (this != &otro) { ListaTextos copia(otro); intercambiar(copia); }
        return *this;
    }
    ListaTextos(ListaTextos&& otro) noexcept : ListaTextos() { intercambiar(otro); }
    ListaTextos& operator=(ListaTextos&& otro) noexcept {
        if (this != &otro) { ListaTextos temporal(std::move(otro)); intercambiar(temporal); }
        return *this;
    }
    void intercambiar(ListaTextos& otro) noexcept {
        std::swap(elementos_, otro.elementos_);
        std::swap(cantidad_, otro.cantidad_);
        std::swap(capacidad_, otro.capacidad_);
    }
    int cantidad() const { return cantidad_; }
    const std::string& obtener(int indice) const {
        if (indice < 0 || indice >= cantidad_) throw std::out_of_range("Indice invalido");
        return elementos_[indice];
    }
    bool contiene(const std::string& texto) const {
        for (int i = 0; i < cantidad_; ++i)
            if (textoIgual(elementos_[i], texto)) return true;
        return false;
    }
    bool agregar(const std::string& texto) {
        std::string limpio = recortar(texto);
        if (limpio.empty() || contiene(limpio)) return false;
        if (cantidad_ == capacidad_) ampliar();
        elementos_[cantidad_++] = limpio;
        return true;
    }
    bool quitar(const std::string& texto) {
        for (int i = 0; i < cantidad_; ++i) {
            if (textoIgual(elementos_[i], texto)) {
                for (int j = i; j < cantidad_ - 1; ++j) elementos_[j] = elementos_[j + 1];
                --cantidad_;
                return true;
            }
        }
        return false;
    }
};
