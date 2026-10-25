#include "MatrizDispersa.h"

MatrizDispersa::~MatrizDispersa() { limpiar(); }

MatrizDispersa::Fila* MatrizDispersa::buscarFila(int id) const {
    for (Fila* f = filas_; f && f->idUsuario <= id; f = f->siguiente)
        if (f->idUsuario == id) return f;
    return nullptr;
}

MatrizDispersa::Columna* MatrizDispersa::buscarColumna(int id) const {
    for (Columna* c = columnas_; c && c->idContenido <= id; c = c->siguiente)
        if (c->idContenido == id) return c;
    return nullptr;
}

MatrizDispersa::Fila* MatrizDispersa::obtenerOCrearFila(int id) {
    Fila** enlace = &filas_;
    while (*enlace && (*enlace)->idUsuario < id) enlace = &(*enlace)->siguiente;
    if (*enlace && (*enlace)->idUsuario == id) return *enlace;
    Fila* f = new Fila{id, nullptr, *enlace};
    *enlace = f;
    return f;
}

MatrizDispersa::Columna* MatrizDispersa::obtenerOCrearColumna(int id) {
    Columna** enlace = &columnas_;
    while (*enlace && (*enlace)->idContenido < id) enlace = &(*enlace)->siguiente;
    if (*enlace && (*enlace)->idContenido == id) return *enlace;
    Columna* c = new Columna{id, nullptr, *enlace};
    *enlace = c;
    return c;
}

MatrizDispersa::Celda* MatrizDispersa::buscarCelda(int usuario, int contenido) const {
    Fila* f = buscarFila(usuario);
    if (!f) return nullptr;
    for (Celda* c = f->primera; c && c->valor.idContenido <= contenido; c = c->derecha)
        if (c->valor.idContenido == contenido) return c;
    return nullptr;
}

MatrizDispersa::Celda* MatrizDispersa::obtenerOCrearCelda(int usuario, int contenido) {
    if (usuario <= 0 || contenido <= 0) return nullptr;
    Celda* existente = buscarCelda(usuario, contenido);
    if (existente) return existente;
    Fila* f = obtenerOCrearFila(usuario);
    Columna* col = obtenerOCrearColumna(contenido);
    Celda* nueva = new Celda{};
    nueva->valor.idUsuario = usuario;
    nueva->valor.idContenido = contenido;

    Celda** horizontal = &f->primera;
    while (*horizontal && (*horizontal)->valor.idContenido < contenido)
        horizontal = &(*horizontal)->derecha;
    nueva->derecha = *horizontal;
    *horizontal = nueva;

    Celda** vertical = &col->primera;
    while (*vertical && (*vertical)->valor.idUsuario < usuario)
        vertical = &(*vertical)->abajo;
    nueva->abajo = *vertical;
    *vertical = nueva;
    ++cantidad_;
    return nueva;
}

bool MatrizDispersa::consultar(int usuario, int contenido, Interaccion& resultado) const {
    Celda* c = buscarCelda(usuario, contenido);
    if (!c) return false;
    resultado = c->valor;
    return true;
}

bool MatrizDispersa::guardar(const Interaccion& valor) {
    if (valor.idUsuario <= 0 || valor.idContenido <= 0 ||
        valor.calificacion < 0 || valor.calificacion > 5 || valor.visualizaciones < 0)
        return false;
    Celda* c = obtenerOCrearCelda(valor.idUsuario, valor.idContenido);
    if (!c) return false;
    c->valor = valor;
    return true;
}

bool MatrizDispersa::registrarVisualizacion(int usuario, int contenido) {
    Celda* c = obtenerOCrearCelda(usuario, contenido);
    if (!c) return false;
    ++c->valor.visualizaciones;
    return true;
}

bool MatrizDispersa::calificar(int usuario, int contenido, int nota) {
    if (nota < 1 || nota > 5) return false;
    Celda* c = obtenerOCrearCelda(usuario, contenido);
    if (!c) return false;
    c->valor.calificacion = nota;
    return true;
}

bool MatrizDispersa::marcarFavorito(int usuario, int contenido, bool favorito) {
    Celda* c = obtenerOCrearCelda(usuario, contenido);
    if (!c) return false;
    c->valor.favorito = favorito;
    return true;
}

void MatrizDispersa::limpiarEncabezadosVacios() {
    Fila** f = &filas_;
    while (*f) {
        if ((*f)->primera) { f = &(*f)->siguiente; continue; }
        Fila* borrar = *f; *f = borrar->siguiente; delete borrar;
    }
    Columna** c = &columnas_;
    while (*c) {
        if ((*c)->primera) { c = &(*c)->siguiente; continue; }
        Columna* borrar = *c; *c = borrar->siguiente; delete borrar;
    }
}

void MatrizDispersa::eliminarUsuario(int usuario) {
    Fila* f = buscarFila(usuario);
    if (!f) return;
    Celda* actual = f->primera;
    while (actual) {
        Celda* siguiente = actual->derecha;
        Columna* col = buscarColumna(actual->valor.idContenido);
        Celda** enlace = &col->primera;
        while (*enlace && *enlace != actual) enlace = &(*enlace)->abajo;
        if (*enlace) *enlace = actual->abajo;
        delete actual;
        --cantidad_;
        actual = siguiente;
    }
    f->primera = nullptr;
    limpiarEncabezadosVacios();
}

void MatrizDispersa::eliminarContenido(int contenido) {
    Columna* col = buscarColumna(contenido);
    if (!col) return;
    Celda* actual = col->primera;
    while (actual) {
        Celda* siguiente = actual->abajo;
        Fila* fila = buscarFila(actual->valor.idUsuario);
        Celda** enlace = &fila->primera;
        while (*enlace && *enlace != actual) enlace = &(*enlace)->derecha;
        if (*enlace) *enlace = actual->derecha;
        delete actual;
        --cantidad_;
        actual = siguiente;
    }
    col->primera = nullptr;
    limpiarEncabezadosVacios();
}

void MatrizDispersa::limpiar() {
    while (filas_) {
        Fila* f = filas_;
        filas_ = f->siguiente;
        Celda* c = f->primera;
        while (c) { Celda* siguiente = c->derecha; delete c; c = siguiente; }
        delete f;
    }
    while (columnas_) {
        Columna* c = columnas_;
        columnas_ = c->siguiente;
        delete c;
    }
    cantidad_ = 0;
}
