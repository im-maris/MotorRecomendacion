#include "ArbolCategorias.h"
#include "Utilidades.h"
#include <iostream>
#include <sstream>

ArbolCategorias::ArbolCategorias() : raiz_(new Nodo("Catalogo")) {}
ArbolCategorias::~ArbolCategorias() { liberar(raiz_); }

void ArbolCategorias::liberar(Nodo* n) {
    if (!n) return;
    liberar(n->hijo);
    liberar(n->hermano);
    delete n;
}

void ArbolCategorias::limpiar() {
    liberar(raiz_->hijo);
    raiz_->hijo = nullptr;
}

bool ArbolCategorias::agregarRuta(const std::string& ruta) {
    if (recortar(ruta).empty()) return false;
    std::istringstream entrada(ruta);
    std::string parte;
    Nodo* actual = raiz_;
    bool alguna = false;
    while (std::getline(entrada, parte, '/')) {
        parte = recortar(parte);
        if (parte.empty()) return false;
        alguna = true;
        Nodo** enlace = &actual->hijo;
        while (*enlace && !textoIgual((*enlace)->nombre, parte))
            enlace = &(*enlace)->hermano;
        if (!*enlace) *enlace = new Nodo(parte);
        actual = *enlace;
    }
    return alguna;
}

bool ArbolCategorias::existeRuta(const std::string& ruta) const {
    std::istringstream entrada(ruta);
    std::string parte;
    Nodo* actual = raiz_;
    bool alguna = false;
    while (std::getline(entrada, parte, '/')) {
        parte = recortar(parte);
        if (parte.empty()) return false;
        alguna = true;
        Nodo* buscar = actual->hijo;
        while (buscar && !textoIgual(buscar->nombre, parte)) buscar = buscar->hermano;
        if (!buscar) return false;
        actual = buscar;
    }
    return alguna;
}

void ArbolCategorias::imprimir(const Nodo* n, int profundidad, std::ostream& salida) {
    for (const Nodo* actual = n; actual; actual = actual->hermano) {
        for (int i = 0; i < profundidad; ++i) salida << "  ";
        salida << "- " << actual->nombre << '\n';
        imprimir(actual->hijo, profundidad + 1, salida);
    }
}

void ArbolCategorias::mostrar(std::ostream& salida) const {
    salida << "Catalogo\n";
    imprimir(raiz_->hijo, 1, salida);
}
