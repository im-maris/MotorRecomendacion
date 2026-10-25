#pragma once
#include <string>
#include <iosfwd>

// Arbol N-ario, representacion primer hijo / siguiente hermano.
class ArbolCategorias {
    struct Nodo {
        std::string nombre;
        Nodo* hijo = nullptr;
        Nodo* hermano = nullptr;
        explicit Nodo(const std::string& n) : nombre(n) {}
    };
    Nodo* raiz_ = nullptr;
    static void liberar(Nodo* n);
    static void imprimir(const Nodo* n, int profundidad, std::ostream& salida);
public:
    ArbolCategorias();
    ArbolCategorias(const ArbolCategorias&) = delete;
    ArbolCategorias& operator=(const ArbolCategorias&) = delete;
    ~ArbolCategorias();
    void limpiar();
    bool agregarRuta(const std::string& ruta); // ejemplo: Arte/Musica/Rock
    bool existeRuta(const std::string& ruta) const;
    void mostrar(std::ostream& salida) const;
};
