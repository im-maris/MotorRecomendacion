#pragma once
#include "Entidades.h"
#include "TablaHash.h"
#include "Grafo.h"
#include "ArbolCategorias.h"
#include "MatrizDispersa.h"
#include "MotorRecomendacion.h"
#include <string>

class GestorPersistencia;

class Sistema {
    friend class GestorPersistencia;
    TablaHash<Usuario> usuarios_;
    TablaHash<Contenido> contenidos_;
    Grafo grafo_;
    ArbolCategorias arbol_;
    MatrizDispersa interacciones_;
    MotorRecomendacion motor_;
    std::string carpetaDatos_;

    void datosIniciales();
    void reconstruirIndices();
    static double similitudCalculada(const Contenido& a, const Contenido& b);
    bool guardarCambios() const;
public:
    explicit Sistema(const std::string& carpetaDatos = "datos");
    Sistema(const Sistema&) = delete;
    Sistema& operator=(const Sistema&) = delete;
    ~Sistema();

    const Usuario* usuario(int id) const { return usuarios_.buscar(id); }
    const Contenido* contenido(int id) const { return contenidos_.buscar(id); }
    int cantidadUsuarios() const { return usuarios_.cantidad(); }
    int cantidadContenidos() const { return contenidos_.cantidad(); }
    int cantidadInteracciones() const { return interacciones_.cantidad(); }
    const Grafo& grafo() const { return grafo_; }
    const ArbolCategorias& categorias() const { return arbol_; }
    const MatrizDispersa& matriz() const { return interacciones_; }

    const Usuario* autenticar(int id, const std::string& clave) const;
    bool registrarUsuario(int id, const std::string& nombre, const std::string& clave,
                          RolUsuario rol = RolUsuario::NORMAL);
    bool agregarPreferencia(int id, const std::string& texto);
    bool quitarPreferencia(int id, const std::string& texto);
    bool registrarContenido(const Contenido& nuevo);
    bool modificarContenido(const Contenido& nuevo);
    bool eliminarContenido(int id);
    bool visualizar(int idUsuario, int idContenido);
    bool calificar(int idUsuario, int idContenido, int nota);
    bool marcarFavorito(int idUsuario, int idContenido, bool favorito);
    int recomendar(int idUsuario, Candidato* salida, int limite) const;
    bool guardar() const { return guardarCambios(); }

    template<typename Funcion>
    void recorrerUsuarios(Funcion funcion) const { usuarios_.recorrer(funcion); }
    template<typename Funcion>
    void recorrerContenidos(Funcion funcion) const { contenidos_.recorrer(funcion); }
};
