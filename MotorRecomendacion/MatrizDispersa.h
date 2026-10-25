#pragma once
#include "Entidades.h"

// Matriz dispersa ortogonal: cada celda pertenece a una fila (usuario)
// y a una columna (contenido); no almacena combinaciones vacias.
class MatrizDispersa {
    struct Celda {
        Interaccion valor;
        Celda* derecha = nullptr; // siguiente contenido del mismo usuario
        Celda* abajo = nullptr;   // siguiente usuario del mismo contenido
    };
    struct Fila {
        int idUsuario;
        Celda* primera = nullptr;
        Fila* siguiente = nullptr;
    };
    struct Columna {
        int idContenido;
        Celda* primera = nullptr;
        Columna* siguiente = nullptr;
    };
    Fila* filas_ = nullptr;
    Columna* columnas_ = nullptr;
    int cantidad_ = 0;

    Fila* buscarFila(int id) const;
    Columna* buscarColumna(int id) const;
    Fila* obtenerOCrearFila(int id);
    Columna* obtenerOCrearColumna(int id);
    Celda* buscarCelda(int usuario, int contenido) const;
    Celda* obtenerOCrearCelda(int usuario, int contenido);
    void limpiarEncabezadosVacios();
public:
    MatrizDispersa() = default;
    MatrizDispersa(const MatrizDispersa&) = delete;
    MatrizDispersa& operator=(const MatrizDispersa&) = delete;
    ~MatrizDispersa();
    void limpiar();
    int cantidad() const { return cantidad_; }
    bool consultar(int usuario, int contenido, Interaccion& resultado) const;
    bool guardar(const Interaccion& valor); // Para carga de archivos / actualizacion
    bool registrarVisualizacion(int usuario, int contenido);
    bool calificar(int usuario, int contenido, int nota);
    bool marcarFavorito(int usuario, int contenido, bool favorito);
    void eliminarUsuario(int usuario);
    void eliminarContenido(int contenido);
    template<typename Funcion>
    void porUsuario(int id, Funcion funcion) const {
        Fila* fila = buscarFila(id);
        if (!fila) return;
        for (Celda* celda = fila->primera; celda; celda = celda->derecha)
            funcion(celda->valor);
    }
    template<typename Funcion>
    void porContenido(int id, Funcion funcion) const {
        Columna* col = buscarColumna(id);
        if (!col) return;
        for (Celda* celda = col->primera; celda; celda = celda->abajo)
            funcion(celda->valor);
    }
    template<typename Funcion>
    void recorrer(Funcion funcion) const {
        for (Fila* fila = filas_; fila; fila = fila->siguiente)
            for (Celda* celda = fila->primera; celda; celda = celda->derecha)
                funcion(celda->valor);
    }
};
