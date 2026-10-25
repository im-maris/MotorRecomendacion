#pragma once
#include <stdexcept>

// Tabla hash con encadenamiento separado. NO posee los objetos T, solo sus nodos.
template <typename T>
class TablaHash {
    struct Nodo {
        int clave;
        T* dato;
        Nodo* siguiente;
        Nodo(int c, T* d, Nodo* s) : clave(c), dato(d), siguiente(s) {}
    };
    Nodo** cubetas_;
    int capacidad_;
    int cantidad_;

    static int indice(int clave, int capacidad) {
        int resultado = clave % capacidad;
        return resultado < 0 ? resultado + capacidad : resultado;
    }
    void redimensionar(int nuevaCapacidad) {
        Nodo** nuevas = new Nodo*[nuevaCapacidad]();
        for (int i = 0; i < capacidad_; ++i) {
            Nodo* actual = cubetas_[i];
            while (actual) {
                Nodo* siguiente = actual->siguiente;
                int posicion = indice(actual->clave, nuevaCapacidad);
                actual->siguiente = nuevas[posicion];
                nuevas[posicion] = actual;
                actual = siguiente;
            }
        }
        delete[] cubetas_;
        cubetas_ = nuevas;
        capacidad_ = nuevaCapacidad;
    }
public:
    explicit TablaHash(int capacidadInicial = 17)
        : cubetas_(nullptr), capacidad_(capacidadInicial), cantidad_(0) {
        if (capacidadInicial < 1) throw std::invalid_argument("Capacidad invalida");
        cubetas_ = new Nodo*[capacidad_]();
    }
    TablaHash(const TablaHash&) = delete;
    TablaHash& operator=(const TablaHash&) = delete;
    ~TablaHash() {
        for (int i = 0; i < capacidad_; ++i) {
            Nodo* actual = cubetas_[i];
            while (actual) {
                Nodo* siguiente = actual->siguiente;
                delete actual;
                actual = siguiente;
            }
        }
        delete[] cubetas_;
    }
    bool insertar(int clave, T* dato) {
        if (!dato || buscar(clave)) return false;
        if ((cantidad_ + 1) * 4 > capacidad_ * 3) redimensionar(capacidad_ * 2 + 1);
        int pos = indice(clave, capacidad_);
        cubetas_[pos] = new Nodo(clave, dato, cubetas_[pos]);
        ++cantidad_;
        return true;
    }
    T* buscar(int clave) const {
        for (Nodo* n = cubetas_[indice(clave, capacidad_)]; n; n = n->siguiente)
            if (n->clave == clave) return n->dato;
        return nullptr;
    }
    bool eliminar(int clave) {
        int pos = indice(clave, capacidad_);
        Nodo** enlace = &cubetas_[pos];
        while (*enlace) {
            if ((*enlace)->clave == clave) {
                Nodo* borrar = *enlace;
                *enlace = borrar->siguiente;
                delete borrar; // El objeto T permanece vivo.
                --cantidad_;
                return true;
            }
            enlace = &(*enlace)->siguiente;
        }
        return false;
    }
    int cantidad() const { return cantidad_; }
    int capacidad() const { return capacidad_; }
    template <typename Funcion>
    void recorrer(Funcion funcion) const {
        for (int i = 0; i < capacidad_; ++i)
            for (Nodo* n = cubetas_[i]; n; n = n->siguiente)
                funcion(n->clave, n->dato);
    }
};
