#include "GestorPersistencia.h"
#include "Sistema.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>

namespace fs = std::filesystem;

namespace {
    constexpr const char* archivos[] = {
        "usuarios.txt", "contenidos.txt", "interacciones.txt", "similitudes.txt"
    };
    bool reemplazar(const fs::path& temporal, const fs::path& destino) {
        std::error_code error;
        fs::rename(temporal, destino, error);
        if (!error) return true;
        // En Windows, rename no siempre sustituye un archivo existente.
        fs::remove(destino, error);
        error.clear();
        fs::rename(temporal, destino, error);
        return !error;
    }
    template<typename Funcion>
    bool escribirArchivo(const fs::path& archivo, Funcion escribir) {
        fs::path temporal = archivo.string() + ".tmp";
        {
            std::ofstream salida(temporal, std::ios::trunc);
            if (!salida) return false;
            escribir(salida);
            salida.flush();
            if (!salida.good()) return false;
        }
        return reemplazar(temporal, archivo);
    }
    bool lineaVacia(const std::string& linea) {
        return linea.find_first_not_of(" \t\r") == std::string::npos;
    }
}

bool GestorPersistencia::hayAlgunArchivo(const std::string& carpeta) {
    for (const char* nombre : archivos)
        if (fs::exists(fs::path(carpeta) / nombre)) return true;
    return false;
}

bool GestorPersistencia::guardar(const Sistema& s, const std::string& carpeta) {
    fs::create_directories(carpeta);
    fs::path dir(carpeta);
    bool a = escribirArchivo(dir / archivos[0], [&](std::ostream& out) {
        s.usuarios_.recorrer([&](int, const Usuario* u) {
            out << u->id << ' ' << std::quoted(u->nombre) << ' '
                << std::quoted(u->clave) << ' ' << static_cast<int>(u->rol)
                << ' ' << u->preferencias.cantidad();
            for (int i = 0; i < u->preferencias.cantidad(); ++i)
                out << ' ' << std::quoted(u->preferencias.obtener(i));
            out << '\n';
        });
    });
    bool b = escribirArchivo(dir / archivos[1], [&](std::ostream& out) {
        s.contenidos_.recorrer([&](int, const Contenido* c) {
            out << c->id << ' ' << std::setprecision(17) << c->popularidad << ' '
                << std::quoted(c->titulo) << ' ' << std::quoted(c->categoria)
                << ' ' << c->etiquetas.cantidad();
            for (int i = 0; i < c->etiquetas.cantidad(); ++i)
                out << ' ' << std::quoted(c->etiquetas.obtener(i));
            out << '\n';
        });
    });
    bool c = escribirArchivo(dir / archivos[2], [&](std::ostream& out) {
        s.interacciones_.recorrer([&](const Interaccion& i) {
            out << i.idUsuario << ' ' << i.idContenido << ' ' << i.visualizaciones
                << ' ' << i.calificacion << ' ' << static_cast<int>(i.favorito) << '\n';
        });
    });
    bool d = escribirArchivo(dir / archivos[3], [&](std::ostream& out) {
        s.grafo_.recorrerRelaciones([&](int idA, int idB, double peso) {
            out << idA << ' ' << idB << ' ' << std::setprecision(17) << peso << '\n';
        });
    });
    return a && b && c && d;
}

bool GestorPersistencia::cargar(Sistema& s, const std::string& carpeta) {
    fs::path dir(carpeta);
    for (const char* archivo : archivos)
        if (!fs::exists(dir / archivo)) return false;

    std::string linea;
    std::ifstream usuarios(dir / archivos[0]);
    if (!usuarios) return false;
    while (std::getline(usuarios, linea)) {
        if (lineaVacia(linea)) continue;
        std::istringstream in(linea);
        int id, rol, cuenta;
        std::string nombre, clave;
        if (!(in >> id >> std::quoted(nombre) >> std::quoted(clave) >> rol >> cuenta) ||
            id <= 0 || nombre.empty() || clave.empty() || (rol != 0 && rol != 1) ||
            cuenta < 0 || cuenta > 1000 || s.usuarios_.buscar(id)) return false;
        Usuario* u = new Usuario;
        u->id = id; u->nombre = nombre; u->clave = clave;
        u->rol = static_cast<RolUsuario>(rol);
        for (int i = 0; i < cuenta; ++i) {
            std::string pref;
            if (!(in >> std::quoted(pref)) || !u->preferencias.agregar(pref)) {
                delete u; return false;
            }
        }
        if (!s.usuarios_.insertar(id, u)) { delete u; return false; }
    }
    if (!usuarios.eof() && usuarios.fail()) return false;

    std::ifstream contenidos(dir / archivos[1]);
    if (!contenidos) return false;
    while (std::getline(contenidos, linea)) {
        if (lineaVacia(linea)) continue;
        std::istringstream in(linea);
        int id, cuenta;
        double pop;
        std::string titulo, categoria;
        if (!(in >> id >> pop >> std::quoted(titulo) >> std::quoted(categoria) >> cuenta) ||
            id <= 0 || titulo.empty() || categoria.empty() || !std::isfinite(pop) ||
            pop < 0 || pop > 100 || cuenta < 0 || cuenta > 1000 || s.contenidos_.buscar(id))
            return false;
        Contenido* c = new Contenido;
        c->id = id; c->titulo = titulo; c->categoria = categoria; c->popularidad = pop;
        for (int i = 0; i < cuenta; ++i) {
            std::string tag;
            if (!(in >> std::quoted(tag)) || !c->etiquetas.agregar(tag)) {
                delete c; return false;
            }
        }
        if (!s.contenidos_.insertar(id, c)) { delete c; return false; }
    }
    if (!contenidos.eof() && contenidos.fail()) return false;

    s.reconstruirIndices();
    // Las conexiones se reconstruyen exactamente como estaban guardadas.
    s.grafo_.limpiar();
    s.contenidos_.recorrer([&](int id, const Contenido*) { s.grafo_.agregarVertice(id); });
    std::ifstream similitudes(dir / archivos[3]);
    if (!similitudes) return false;
    while (std::getline(similitudes, linea)) {
        if (lineaVacia(linea)) continue;
        std::istringstream in(linea);
        int a, b; double peso;
        if (!(in >> a >> b >> peso) || !std::isfinite(peso) ||
            !s.grafo_.agregarRelacion(a, b, peso)) return false;
    }
    if (!similitudes.eof() && similitudes.fail()) return false;

    std::ifstream interacciones(dir / archivos[2]);
    if (!interacciones) return false;
    while (std::getline(interacciones, linea)) {
        if (lineaVacia(linea)) continue;
        std::istringstream in(linea);
        Interaccion i;
        int favorito;
        if (!(in >> i.idUsuario >> i.idContenido >> i.visualizaciones >> i.calificacion >> favorito) ||
            (favorito != 0 && favorito != 1) || !s.usuarios_.buscar(i.idUsuario) ||
            !s.contenidos_.buscar(i.idContenido)) return false;
        i.favorito = favorito == 1;
        Interaccion anterior;
        if (s.interacciones_.consultar(i.idUsuario, i.idContenido, anterior)) return false;
        if (!s.interacciones_.guardar(i)) return false;
    }
    if (!interacciones.eof() && interacciones.fail()) return false;
    return true;
}
