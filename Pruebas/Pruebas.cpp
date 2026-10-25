#include "../MotorRecomendacion/Sistema.h"
#include "../MotorRecomendacion/HeapMaximo.h"
#include "../MotorRecomendacion/TablaHash.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>

namespace fs = std::filesystem;

void probarTablaHash() {
    TablaHash<int> tabla(3);
    int datos[75];
    for (int i = 0; i < 75; ++i) {
        datos[i] = i * 7;
        assert(tabla.insertar(i * 3 + 1, &datos[i]));
    }
    assert(tabla.cantidad() == 75);
    assert(!tabla.insertar(1, &datos[0]));
    for (int i = 0; i < 75; ++i) assert(*tabla.buscar(i * 3 + 1) == i * 7);
    assert(tabla.eliminar(1));
    assert(tabla.eliminar(73));
    assert(!tabla.eliminar(111111));
    assert(tabla.buscar(1) == nullptr);
    assert(tabla.cantidad() == 73);
}

void probarGrafo() {
    Grafo grafo;
    assert(grafo.agregarVertice(1));
    assert(grafo.agregarVertice(2));
    assert(grafo.agregarVertice(3));
    assert(!grafo.agregarVertice(1));
    assert(grafo.agregarRelacion(1, 2, 0.75));
    assert(grafo.agregarRelacion(2, 3, 0.5));
    assert(std::abs(grafo.similitud(2, 1) - 0.75) < 1e-9);
    assert(!grafo.agregarRelacion(1, 9, 0.3));
    assert(!grafo.agregarRelacion(1, 2, 1.5));
    assert(grafo.eliminarVertice(2));
    assert(grafo.similitud(1, 2) == 0);
    assert(grafo.similitud(3, 2) == 0);
}

void probarArbol() {
    ArbolCategorias arbol;
    assert(arbol.agregarRuta("Tecnologia/Programacion/C++"));
    assert(arbol.agregarRuta("Tecnologia/Programacion/Python"));
    assert(arbol.agregarRuta("Arte/Pintura"));
    assert(arbol.existeRuta("Tecnologia/Programacion"));
    assert(arbol.existeRuta("Arte/Pintura"));
    assert(!arbol.existeRuta("Tecnologia/Astronomia"));
    std::ostringstream salida;
    arbol.mostrar(salida);
    assert(salida.str().find("C++") != std::string::npos);
    arbol.limpiar();
    assert(!arbol.existeRuta("Arte/Pintura"));
}

void probarMatriz() {
    MatrizDispersa m;
    assert(m.registrarVisualizacion(10, 100));
    assert(m.registrarVisualizacion(10, 100));
    assert(m.calificar(10, 100, 4));
    assert(m.calificar(20, 100, 5));
    assert(m.marcarFavorito(10, 200, true));
    assert(m.calificar(30, 300, 1));
    assert(m.cantidad() == 4);
    assert(!m.calificar(10, 100, 6));
    Interaccion i;
    assert(m.consultar(10, 100, i));
    assert(i.visualizaciones == 2 && i.calificacion == 4);
    int contUsuario = 0, contContenido = 0;
    m.porUsuario(10, [&](const Interaccion&) { ++contUsuario; });
    m.porContenido(100, [&](const Interaccion&) { ++contContenido; });
    assert(contUsuario == 2 && contContenido == 2);
    m.eliminarUsuario(10);
    assert(m.cantidad() == 2);
    assert(!m.consultar(10, 100, i));
    assert(m.consultar(20, 100, i));
    m.eliminarContenido(100);
    assert(m.cantidad() == 1);
    assert(m.consultar(30, 300, i));
    m.limpiar();
    assert(m.cantidad() == 0);
}

void probarHeap() {
    HeapMaximo heap(2);
    heap.insertar({8, 0.6});
    heap.insertar({6, 0.9});
    heap.insertar({7, 0.9});
    heap.insertar({9, 0.1});
    assert(heap.extraerMaximo().idContenido == 6); // Empate, menor ID.
    assert(heap.extraerMaximo().idContenido == 7);
    assert(heap.extraerMaximo().idContenido == 8);
    assert(heap.extraerMaximo().idContenido == 9);
    assert(heap.vacio());
}

void probarSistema() {
    const std::string ruta = "datos_pruebas_motor";
    fs::remove_all(ruta);
    {
        Sistema s(ruta);
        assert(s.cantidadContenidos() == 19);
        assert(s.cantidadUsuarios() == 3);
        assert(s.autenticar(1, "admin123") != nullptr);
        assert(s.autenticar(2, "error") == nullptr);
        assert(s.grafo().similitud(101, 102) > 0);
        assert(s.categorias().existeRuta("Tecnologia/Programacion"));
        assert(s.cantidadInteracciones() == 6);
        Candidato resultados[20];
        int n = s.recomendar(2, resultados, 20);
        assert(n == 15);
        for (int j = 0; j < n; ++j) {
            assert(resultados[j].puntuacion >= 0 && resultados[j].puntuacion <= 1);
            if (j > 0) assert(resultados[j - 1].puntuacion >= resultados[j].puntuacion);
            Interaccion i;
            assert(!s.matriz().consultar(2, resultados[j].idContenido, i));
        }
        assert(s.registrarUsuario(9, "Eva", "claveEva"));
        assert(!s.registrarUsuario(9, "Duplicado", "otra"));
        assert(s.agregarPreferencia(9, "Tecnologia"));
        assert(!s.agregarPreferencia(9, "tecnologia"));
        assert(s.quitarPreferencia(9, "Tecnologia"));
        Contenido c;
        c.id = 200;
        c.titulo = "Contenido de prueba";
        c.categoria = "Tecnologia/Programacion";
        c.popularidad = 75;
        c.etiquetas.agregar("cpp");
        assert(s.registrarContenido(c));
        assert(s.contenido(200));
        assert(s.grafo().similitud(101, 200) > 0);
        assert(s.visualizar(9, 200));
        assert(s.calificar(9, 200, 5));
        assert(s.marcarFavorito(9, 200, true));
        assert(s.cantidadInteracciones() == 7);
        c.titulo = "Contenido modificado";
        c.categoria = "Arte/Musica";
        assert(s.modificarContenido(c));
        assert(s.contenido(200)->titulo == "Contenido modificado");
        assert(s.categorias().existeRuta("Arte/Musica"));
        assert(s.guardar());
    }
    {
        Sistema s(ruta);
        assert(s.cantidadContenidos() == 20);
        assert(s.cantidadUsuarios() == 4);
        assert(s.autenticar(9, "claveEva") != nullptr);
        assert(s.contenido(200)->titulo == "Contenido modificado");
        assert(s.cantidadInteracciones() == 7);
        Interaccion i;
        assert(s.matriz().consultar(9, 200, i));
        assert(i.visualizaciones == 1 && i.calificacion == 5 && i.favorito);
        assert(s.eliminarContenido(200));
        assert(!s.contenido(200));
        assert(!s.matriz().consultar(9, 200, i));
        assert(s.cantidadInteracciones() == 6);
    }
    {
        Sistema s(ruta);
        assert(s.cantidadContenidos() == 19);
        assert(s.cantidadInteracciones() == 6);
    }
    fs::remove_all(ruta);
}

int main() {
    probarTablaHash();
    probarGrafo();
    probarArbol();
    probarMatriz();
    probarHeap();
    probarSistema();
    std::cout << "TODAS LAS PRUEBAS PASARON: hash, grafo, arbol, matriz, heap, motor y persistencia.\n";
}
