#include "Grafo.h"

Grafo::~Grafo() { limpiar(); }

Grafo::Vertice* Grafo::buscarVertice(int id) const {
    for (Vertice* v = vertices_; v; v = v->siguiente)
        if (v->id == id) return v;
    return nullptr;
}

void Grafo::limpiar() {
    while (vertices_) {
        Vertice* v = vertices_;
        vertices_ = v->siguiente;
        Arista* a = v->aristas;
        while (a) {
            Arista* borrar = a;
            a = a->siguiente;
            delete borrar;
        }
        delete v;
    }
    totalVertices_ = 0;
}

bool Grafo::agregarVertice(int id) {
    if (id <= 0 || buscarVertice(id)) return false;
    vertices_ = new Vertice{id, nullptr, vertices_};
    ++totalVertices_;
    return true;
}

void Grafo::ponerArista(Vertice* origen, int destino, double peso) {
    for (Arista* a = origen->aristas; a; a = a->siguiente) {
        if (a->destino == destino) { a->peso = peso; return; }
    }
    origen->aristas = new Arista{destino, peso, origen->aristas};
}

void Grafo::quitarArista(Vertice* origen, int destino) {
    Arista** enlace = &origen->aristas;
    while (*enlace) {
        if ((*enlace)->destino == destino) {
            Arista* borrar = *enlace;
            *enlace = borrar->siguiente;
            delete borrar;
            return;
        }
        enlace = &(*enlace)->siguiente;
    }
}

bool Grafo::agregarRelacion(int a, int b, double peso) {
    if (a == b || peso <= 0.0 || peso > 1.0) return false;
    Vertice* va = buscarVertice(a);
    Vertice* vb = buscarVertice(b);
    if (!va || !vb) return false;
    ponerArista(va, b, peso);
    ponerArista(vb, a, peso);
    return true;
}

bool Grafo::eliminarVertice(int id) {
    Vertice** enlace = &vertices_;
    while (*enlace && (*enlace)->id != id) enlace = &(*enlace)->siguiente;
    if (!*enlace) return false;
    Vertice* borrar = *enlace;
    for (Vertice* v = vertices_; v; v = v->siguiente)
        if (v != borrar) quitarArista(v, id);
    Arista* a = borrar->aristas;
    while (a) { Arista* siguiente = a->siguiente; delete a; a = siguiente; }
    *enlace = borrar->siguiente;
    delete borrar;
    --totalVertices_;
    return true;
}

double Grafo::similitud(int a, int b) const {
    Vertice* v = buscarVertice(a);
    if (!v) return 0.0;
    for (Arista* arista = v->aristas; arista; arista = arista->siguiente)
        if (arista->destino == b) return arista->peso;
    return 0.0;
}
