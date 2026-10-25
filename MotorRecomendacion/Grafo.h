#pragma once

class Grafo {
    struct Arista {
        int destino;
        double peso;
        Arista* siguiente;
    };
    struct Vertice {
        int id;
        Arista* aristas;
        Vertice* siguiente;
    };
    Vertice* vertices_ = nullptr;
    int totalVertices_ = 0;
    Vertice* buscarVertice(int id) const;
    static void ponerArista(Vertice* origen, int destino, double peso);
    static void quitarArista(Vertice* origen, int destino);
public:
    Grafo() = default;
    Grafo(const Grafo&) = delete;
    Grafo& operator=(const Grafo&) = delete;
    ~Grafo();
    void limpiar();
    bool agregarVertice(int id);
    bool agregarRelacion(int a, int b, double peso); // No dirigido, peso (0,1].
    bool eliminarVertice(int id);
    double similitud(int a, int b) const;
    int totalVertices() const { return totalVertices_; }
    template<typename Funcion>
    void recorrerRelaciones(Funcion funcion) const {
        for (Vertice* v = vertices_; v; v = v->siguiente)
            for (Arista* a = v->aristas; a; a = a->siguiente)
                if (v->id < a->destino) funcion(v->id, a->destino, a->peso);
    }
};
