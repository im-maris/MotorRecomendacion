#include "Sistema.h"
#include "Utilidades.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#endif

using std::cout;
using std::cin;
using std::string;

namespace {
    string pedirTexto(const string& mensaje, bool obligatorio = true) {
        while (true) {
            cout << mensaje;
            string linea;
            if (!std::getline(cin, linea)) throw std::runtime_error("Fin de entrada.");
            linea = recortar(linea);
            if (!obligatorio || !linea.empty()) return linea;
            cout << "El dato no puede quedar vacio.\n";
        }
    }
    int pedirEntero(const string& mensaje, int minimo, int maximo) {
        while (true) {
            string linea = pedirTexto(mensaje);
            int n;
            if (leerEntero(linea, n) && n >= minimo && n <= maximo) return n;
            cout << "Ingresa un numero entero entre " << minimo << " y " << maximo << ".\n";
        }
    }
    double pedirDecimal(const string& mensaje, double minimo, double maximo) {
        while (true) {
            string linea = pedirTexto(mensaje);
            double n;
            if (leerDecimal(linea, n) && n >= minimo && n <= maximo) return n;
            cout << "Ingresa un numero entre " << minimo << " y " << maximo << ".\n";
        }
    }
    void cargarLista(ListaTextos& lista, const string& csv) {
        size_t desde = 0;
        while (desde <= csv.size()) {
            size_t hasta = csv.find(',', desde);
            lista.agregar(csv.substr(desde, hasta == string::npos ? hasta : hasta - desde));
            if (hasta == string::npos) break;
            desde = hasta + 1;
        }
    }
    void mostrarLista(const ListaTextos& lista) {
        if (!lista.cantidad()) { cout << "(ninguna)"; return; }
        for (int i = 0; i < lista.cantidad(); ++i) {
            if (i) cout << ", ";
            cout << lista.obtener(i);
        }
    }
    void mostrarContenido(const Contenido& c) {
        cout << "[" << c.id << "] " << c.titulo << " | " << c.categoria
             << " | popularidad " << c.popularidad << "/100 | etiquetas: ";
        mostrarLista(c.etiquetas);
        cout << '\n';
    }
    void mostrarCatalogo(const Sistema& s) {
        cout << "\n=== CATALOGO (" << s.cantidadContenidos() << " contenidos) ===\n";
        s.recorrerContenidos([&](int, const Contenido* c) { mostrarContenido(*c); });
    }
    void buscarContenido(const Sistema& s) {
        int id = pedirEntero("ID a buscar: ", 1, 1000000000);
        const Contenido* c = s.contenido(id);
        if (!c) cout << "No se encontro el contenido.\n";
        else mostrarContenido(*c);
    }
    void administrarPreferencias(Sistema& s, int usuario) {
        while (true) {
            const Usuario* u = s.usuario(usuario);
            cout << "\nMis preferencias: ";
            mostrarLista(u->preferencias);
            cout << "\n1. Agregar | 2. Eliminar | 0. Regresar\n";
            int opcion = pedirEntero("Opcion: ", 0, 2);
            if (opcion == 0) break;
            string texto = pedirTexto("Categoria o etiqueta: ");
            bool exito = opcion == 1 ? s.agregarPreferencia(usuario, texto)
                                     : s.quitarPreferencia(usuario, texto);
            cout << (exito ? "Preferencias actualizadas.\n" : "No se pudo cambiar (duplicada, inexistente o error al guardar).\n");
        }
    }
    void mostrarHistorial(const Sistema& s, int usuario) {
        cout << "\n=== MI HISTORIAL ===\n";
        bool alguno = false;
        s.matriz().porUsuario(usuario, [&](const Interaccion& i) {
            alguno = true;
            const Contenido* c = s.contenido(i.idContenido);
            cout << '[' << i.idContenido << "] " << (c ? c->titulo : "(no disponible)")
                 << " | vistas " << i.visualizaciones << " | nota ";
            if (i.calificacion) cout << i.calificacion << "/5";
            else cout << "sin calificar";
            cout << " | favorito " << (i.favorito ? "si" : "no") << '\n';
        });
        if (!alguno) cout << "Todavia no tienes interacciones.\n";
    }
    void recomendar(const Sistema& s, int usuario) {
        int cantidad = pedirEntero("Cantidad de recomendaciones (1-20): ", 1, 20);
        std::unique_ptr<Candidato[]> resultados(new Candidato[cantidad]);
        int n = s.recomendar(usuario, resultados.get(), cantidad);
        cout << "\n=== RECOMENDACIONES PERSONALIZADAS ===\n";
        if (!n) { cout << "No hay contenidos nuevos disponibles.\n"; return; }
        for (int i = 0; i < n; ++i) {
            const Contenido* c = s.contenido(resultados[i].idContenido);
            if (c) {
                cout << i + 1 << ". [" << c->id << "] " << c->titulo
                     << " | afinidad " << std::fixed << std::setprecision(1)
                     << resultados[i].puntuacion * 100 << "% | " << c->categoria << '\n';
            }
        }
        cout << std::defaultfloat;
    }
    Contenido capturarContenido(int id) {
        Contenido c;
        c.id = id;
        c.titulo = pedirTexto("Titulo: ");
        c.categoria = pedirTexto("Categoria (ej: Tecnologia/Programacion): ");
        c.popularidad = pedirDecimal("Popularidad inicial (0-100): ", 0, 100);
        cargarLista(c.etiquetas, pedirTexto("Etiquetas separadas por comas (puede quedar vacio): ", false));
        return c;
    }
    void menuAdministradorAccion(Sistema& s, int opcion) {
        if (opcion == 10) {
            int id = pedirEntero("Nuevo ID: ", 1, 1000000000);
            if (s.contenido(id)) { cout << "Ya existe ese ID.\n"; return; }
            Contenido c = capturarContenido(id);
            cout << (s.registrarContenido(c) ? "Contenido registrado.\n" : "Datos invalidos o error de guardado.\n");
        } else if (opcion == 11) {
            int id = pedirEntero("ID que deseas editar: ", 1, 1000000000);
            const Contenido* anterior = s.contenido(id);
            if (!anterior) { cout << "No existe ese contenido.\n"; return; }
            cout << "Datos actuales:\n";
            mostrarContenido(*anterior);
            cout << "Introduce todos los datos nuevos.\n";
            Contenido c = capturarContenido(id);
            cout << (s.modificarContenido(c) ? "Contenido modificado.\n" : "Datos invalidos o error de guardado.\n");
        } else if (opcion == 12) {
            int id = pedirEntero("ID que deseas eliminar: ", 1, 1000000000);
            const Contenido* c = s.contenido(id);
            if (!c) { cout << "No existe ese contenido.\n"; return; }
            mostrarContenido(*c);
            string confirmar = pedirTexto("Escribe SI para eliminarlo y borrar sus interacciones: ");
            if (confirmar != "SI") { cout << "Cancelado.\n"; return; }
            cout << (s.eliminarContenido(id) ? "Contenido eliminado.\n" : "No se pudo eliminar.\n");
        } else if (opcion == 13) {
            cout << "\n=== USUARIOS ===\n";
            s.recorrerUsuarios([&](int, const Usuario* u) {
                cout << '[' << u->id << "] " << u->nombre << " | "
                     << (u->rol == RolUsuario::ADMINISTRADOR ? "Administrador" : "Normal") << '\n';
            });
        } else if (opcion == 14) {
            cout << "\n=== RELACIONES DEL GRAFO (primeras 40) ===\n";
            int n = 0;
            s.grafo().recorrerRelaciones([&](int a, int b, double p) {
                if (++n <= 40)
                    cout << a << " <-> " << b << " | similitud "
                         << std::fixed << std::setprecision(2) << p << '\n';
            });
            cout << "Total de relaciones: " << n << '\n' << std::defaultfloat;
        }
    }
    void menuUsuario(Sistema& s, int idUsuario) {
        while (true) {
            const Usuario* usuario = s.usuario(idUsuario);
            bool admin = usuario->rol == RolUsuario::ADMINISTRADOR;
            cout << "\n===== SESION: " << usuario->nombre << (admin ? " [ADMIN]" : "") << " =====\n"
                 << "1. Ver catalogo\n2. Buscar contenido por ID\n"
                 << "3. Mis preferencias\n4. Registrar visualizacion\n"
                 << "5. Calificar contenido\n6. Marcar o desmarcar favorito\n"
                 << "7. Consultar historial\n8. Ver recomendaciones\n"
                 << "9. Ver arbol de categorias\n";
            if (admin)
                cout << "10. Agregar contenido\n11. Modificar contenido\n"
                     << "12. Eliminar contenido\n13. Ver usuarios\n14. Ver similitudes del grafo\n";
            cout << "0. Cerrar sesion\n";
            int opcion = pedirEntero("Opcion: ", 0, admin ? 14 : 9);
            if (opcion == 0) return;
            if (opcion == 1) mostrarCatalogo(s);
            else if (opcion == 2) buscarContenido(s);
            else if (opcion == 3) administrarPreferencias(s, idUsuario);
            else if (opcion == 4) {
                int id = pedirEntero("ID de contenido visualizado: ", 1, 1000000000);
                cout << (s.visualizar(idUsuario, id) ? "Visualizacion registrada.\n" : "Contenido inexistente o error.\n");
            } else if (opcion == 5) {
                int id = pedirEntero("ID de contenido: ", 1, 1000000000);
                int nota = pedirEntero("Calificacion (1-5): ", 1, 5);
                cout << (s.calificar(idUsuario, id, nota) ? "Calificacion guardada.\n" : "Contenido inexistente o error.\n");
            } else if (opcion == 6) {
                int id = pedirEntero("ID de contenido: ", 1, 1000000000);
                int f = pedirEntero("1 = favorito, 0 = quitar favorito: ", 0, 1);
                cout << (s.marcarFavorito(idUsuario, id, f == 1) ? "Favorito actualizado.\n" : "Contenido inexistente o error.\n");
            } else if (opcion == 7) mostrarHistorial(s, idUsuario);
            else if (opcion == 8) recomendar(s, idUsuario);
            else if (opcion == 9) s.categorias().mostrar(cout);
            else if (admin) menuAdministradorAccion(s, opcion);
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try {
        Sistema sistema("datos");
        cout << "============================================\n"
             << "  MOTOR DE RECOMENDACION DE CONTENIDO (C++17)\n"
             << "============================================\n"
             << "Catalogo inicial: " << sistema.cantidadContenidos() << " contenidos\n"
             << "Demo: ID 2 / clave demo123\n"
             << "Administrador: ID 1 / clave admin123\n";
        while (true) {
            cout << "\n=== MENU PRINCIPAL ===\n"
                 << "1. Iniciar sesion\n2. Registrar usuario normal\n"
                 << "3. Ver catalogo (invitado)\n0. Salir\n";
            int opcion = pedirEntero("Opcion: ", 0, 3);
            if (opcion == 0) break;
            if (opcion == 1) {
                int id = pedirEntero("ID de usuario: ", 1, 1000000000);
                string clave = pedirTexto("Clave: ");
                const Usuario* u = sistema.autenticar(id, clave);
                if (!u) cout << "Credenciales incorrectas.\n";
                else menuUsuario(sistema, u->id);
            } else if (opcion == 2) {
                int id = pedirEntero("Nuevo ID de usuario: ", 1, 1000000000);
                string nombre = pedirTexto("Nombre: ");
                string clave = pedirTexto("Clave: ");
                if (!sistema.registrarUsuario(id, nombre, clave)) {
                    cout << "ID duplicado o no fue posible guardar.\n";
                } else {
                    string preferencias = pedirTexto("Preferencias separadas por comas (opcional): ", false);
                    ListaTextos lista;
                    cargarLista(lista, preferencias);
                    for (int i = 0; i < lista.cantidad(); ++i)
                        sistema.agregarPreferencia(id, lista.obtener(i));
                    cout << "Usuario registrado. Ya puedes iniciar sesion.\n";
                }
            } else if (opcion == 3) mostrarCatalogo(sistema);
        }
        cout << "Hasta pronto.\n";
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
