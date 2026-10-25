#include "Sistema.h"
#include "GestorPersistencia.h"
#include "Utilidades.h"
#include <stdexcept>
#include <filesystem>
#include <algorithm>
#include <sstream>
#include <cmath>

namespace {
    std::string raiz(const std::string& ruta) {
        return minusculas(recortar(ruta.substr(0, ruta.find('/'))));
    }
    Contenido crearContenido(int id, const char* titulo, const char* categoria,
                            double popularidad, const char* etiquetas) {
        Contenido c;
        c.id = id;
        c.titulo = titulo;
        c.categoria = categoria;
        c.popularidad = popularidad;
        std::string datos = etiquetas;
        size_t inicio = 0;
        while (inicio <= datos.size()) {
            size_t fin = datos.find(',', inicio);
            c.etiquetas.agregar(datos.substr(inicio, fin == std::string::npos ? fin : fin - inicio));
            if (fin == std::string::npos) break;
            inicio = fin + 1;
        }
        return c;
    }
    bool rutaValida(const std::string& ruta) {
        if (recortar(ruta).empty() || ruta.front() == '/' || ruta.back() == '/') return false;
        return ruta.find("//") == std::string::npos;
    }
}

Sistema::Sistema(const std::string& carpetaDatos)
    : usuarios_(17), contenidos_(31), carpetaDatos_(carpetaDatos) {
    std::filesystem::create_directories(carpetaDatos_);
    if (GestorPersistencia::hayAlgunArchivo(carpetaDatos_)) {
        if (!GestorPersistencia::cargar(*this, carpetaDatos_)) {
            usuarios_.recorrer([](int, Usuario* u) { delete u; });
            contenidos_.recorrer([](int, Contenido* c) { delete c; });
            throw std::runtime_error("Datos incompletos o invalidos. Revise la carpeta de datos (no se sobrescribio).");
        }
    } else {
        datosIniciales();
        if (!guardarCambios()) {
            usuarios_.recorrer([](int, Usuario* u) { delete u; });
            contenidos_.recorrer([](int, Contenido* c) { delete c; });
            throw std::runtime_error("No se pudo guardar el catalogo inicial.");
        }
    }
}

Sistema::~Sistema() {
    usuarios_.recorrer([](int, Usuario* u) { delete u; });
    contenidos_.recorrer([](int, Contenido* c) { delete c; });
}

bool Sistema::guardarCambios() const {
    return GestorPersistencia::guardar(*this, carpetaDatos_);
}

const Usuario* Sistema::autenticar(int id, const std::string& clave) const {
    const Usuario* u = usuario(id);
    return u && u->clave == clave ? u : nullptr;
}

bool Sistema::registrarUsuario(int id, const std::string& nombre, const std::string& clave,
                              RolUsuario rol) {
    if (id <= 0 || recortar(nombre).empty() || clave.empty() || usuario(id)) return false;
    Usuario* u = new Usuario;
    u->id = id;
    u->nombre = recortar(nombre);
    u->clave = clave;
    u->rol = rol;
    if (!usuarios_.insertar(id, u)) { delete u; return false; }
    return guardarCambios();
}

bool Sistema::agregarPreferencia(int id, const std::string& texto) {
    Usuario* u = usuarios_.buscar(id);
    if (!u || !u->preferencias.agregar(texto)) return false;
    return guardarCambios();
}

bool Sistema::quitarPreferencia(int id, const std::string& texto) {
    Usuario* u = usuarios_.buscar(id);
    if (!u || !u->preferencias.quitar(texto)) return false;
    return guardarCambios();
}

double Sistema::similitudCalculada(const Contenido& a, const Contenido& b) {
    double peso = 0.0;
    if (textoIgual(a.categoria, b.categoria)) peso += 0.55;
    else if (raiz(a.categoria) == raiz(b.categoria)) peso += 0.20;
    int comunes = 0;
    for (int i = 0; i < a.etiquetas.cantidad(); ++i)
        if (b.etiquetas.contiene(a.etiquetas.obtener(i))) ++comunes;
    int unionEtiquetas = a.etiquetas.cantidad() + b.etiquetas.cantidad() - comunes;
    if (unionEtiquetas) peso += 0.45 * comunes / unionEtiquetas;
    return std::min(1.0, peso);
}

void Sistema::reconstruirIndices() {
    arbol_.limpiar();
    grafo_.limpiar();
    contenidos_.recorrer([&](int id, const Contenido* c) {
        arbol_.agregarRuta(c->categoria);
        grafo_.agregarVertice(id);
    });
    contenidos_.recorrer([&](int idA, const Contenido* a) {
        contenidos_.recorrer([&](int idB, const Contenido* b) {
            if (idA >= idB) return;
            double peso = similitudCalculada(*a, *b);
            if (peso >= 0.20) grafo_.agregarRelacion(idA, idB, peso);
        });
    });
}

bool Sistema::registrarContenido(const Contenido& nuevo) {
    if (nuevo.id <= 0 || contenido(nuevo.id) || recortar(nuevo.titulo).empty() ||
        !rutaValida(nuevo.categoria) || !std::isfinite(nuevo.popularidad) || nuevo.popularidad < 0 || nuevo.popularidad > 100)
        return false;
    Contenido* c = new Contenido(nuevo);
    if (!contenidos_.insertar(c->id, c)) { delete c; return false; }
    reconstruirIndices();
    return guardarCambios();
}

bool Sistema::modificarContenido(const Contenido& nuevo) {
    Contenido* anterior = contenidos_.buscar(nuevo.id);
    if (!anterior || recortar(nuevo.titulo).empty() || !rutaValida(nuevo.categoria) ||
        !std::isfinite(nuevo.popularidad) || nuevo.popularidad < 0 || nuevo.popularidad > 100) return false;
    *anterior = nuevo;
    reconstruirIndices();
    return guardarCambios();
}

bool Sistema::eliminarContenido(int id) {
    Contenido* c = contenidos_.buscar(id);
    if (!c) return false;
    interacciones_.eliminarContenido(id);
    contenidos_.eliminar(id);
    delete c;
    reconstruirIndices();
    return guardarCambios();
}

bool Sistema::visualizar(int idUsuario, int idContenido) {
    if (!usuario(idUsuario) || !contenido(idContenido)) return false;
    if (!interacciones_.registrarVisualizacion(idUsuario, idContenido)) return false;
    return guardarCambios();
}

bool Sistema::calificar(int idUsuario, int idContenido, int nota) {
    if (!usuario(idUsuario) || !contenido(idContenido)) return false;
    if (!interacciones_.calificar(idUsuario, idContenido, nota)) return false;
    return guardarCambios();
}

bool Sistema::marcarFavorito(int idUsuario, int idContenido, bool favorito) {
    if (!usuario(idUsuario) || !contenido(idContenido)) return false;
    if (!interacciones_.marcarFavorito(idUsuario, idContenido, favorito)) return false;
    return guardarCambios();
}

int Sistema::recomendar(int idUsuario, Candidato* salida, int limite) const {
    const Usuario* u = usuario(idUsuario);
    if (!u) return 0;
    return motor_.generar(*u, contenidos_, interacciones_, grafo_, salida, limite);
}

void Sistema::datosIniciales() {
    auto crearUsuario = [&](int id, const char* nombre, const char* clave, RolUsuario rol) {
        Usuario* u = new Usuario;
        u->id = id; u->nombre = nombre; u->clave = clave; u->rol = rol;
        usuarios_.insertar(id, u);
        return u;
    };
    crearUsuario(1, "Administrador", "admin123", RolUsuario::ADMINISTRADOR);
    Usuario* fredd = crearUsuario(2, "Usuario Demo", "demo123", RolUsuario::NORMAL);
    fredd->preferencias.agregar("Programacion");
    fredd->preferencias.agregar("Ciencia ficcion");
    fredd->preferencias.agregar("Videojuegos");
    Usuario* alex = crearUsuario(3, "Alex", "demo123", RolUsuario::NORMAL);
    alex->preferencias.agregar("Arte");
    alex->preferencias.agregar("Marvel");

    const struct Datos { int id; const char* titulo; const char* categoria; double pop; const char* tags; } base[] = {
        {101,"C++ desde cero","Tecnologia/Programacion",80,"cpp,algoritmos,desarrollo"},
        {102,"Estructuras de datos","Tecnologia/Programacion",84,"algoritmos,cpp,listas"},
        {103,"Algoritmos de busqueda","Tecnologia/Programacion",78,"algoritmos,busqueda"},
        {104,"Desarrollo de videojuegos","Tecnologia/Videojuegos",89,"videojuegos,motores,programacion"},
        {105,"Introduccion a inteligencia artificial","Tecnologia/Inteligencia artificial",92,"algoritmos,tecnologia,IA"},
        {106,"Bases de datos relacionales","Tecnologia/Bases de datos",75,"SQL,datos,programacion"},
        {107,"Aventuras de ciencia ficcion","Entretenimiento/Ciencia ficcion",90,"espacio,aventura,ciencia ficcion"},
        {108,"Viajes en el tiempo","Entretenimiento/Ciencia ficcion",86,"tiempo,ficcion,misterio"},
        {109,"Cuentos fantasticos","Literatura/Fantasia",72,"magia,fantasia,aventura"},
        {110,"Guia de dibujo","Arte/Ilustracion",68,"dibujo,creatividad,arte"},
        {111,"Pintura al oleo","Arte/Pintura",73,"oleo,pintura,creatividad"},
        {112,"Musica y emociones","Arte/Musica",85,"musica,emociones,creatividad"},
        {113,"Fundamentos de fisica","Ciencia/Fisica",79,"ciencia,fisica,matematicas"},
        {114,"Astronomia para curiosos","Ciencia/Astronomia",88,"espacio,ciencia,astronomia"},
        {115,"Marvel y narrativa","Entretenimiento/Comics",94,"marvel,superheroes,historias"},
        {116,"Estrategias de videojuegos","Entretenimiento/Videojuegos",86,"videojuegos,estrategia,competitivo"},
        {117,"Redes de computadoras","Tecnologia/Redes",77,"redes,informatica,protocolos"},
        {118,"Matematicas discretas","Ciencia/Matematicas",71,"algoritmos,matematicas,logica"},
        {119,"Explorando mundos abiertos","Entretenimiento/Videojuegos",82,"videojuegos,aventura,exploracion"}
    };
    for (const Datos& d : base) {
        Contenido* c = new Contenido(crearContenido(d.id, d.titulo, d.categoria, d.pop, d.tags));
        contenidos_.insertar(c->id, c);
    }
    reconstruirIndices();
    interacciones_.calificar(2, 101, 5);
    interacciones_.registrarVisualizacion(2, 101);
    interacciones_.calificar(2, 107, 5);
    interacciones_.marcarFavorito(2, 107, true);
    interacciones_.calificar(2, 111, 2);
    interacciones_.registrarVisualizacion(2, 116);
    interacciones_.calificar(3, 110, 5);
    interacciones_.calificar(3, 115, 5);
}
