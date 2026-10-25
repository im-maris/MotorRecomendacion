#pragma once
#include "Entidades.h"
#include "TablaHash.h"
#include "Grafo.h"
#include "MatrizDispersa.h"

class MotorRecomendacion {
    static double valoracion(const Interaccion& i);
    static double preferencia(const Usuario& u, const Contenido& c);
    static double categoriaComun(const Contenido& a, const Contenido& b);
public:
    // Formula normalizada en [0,1]: 0.35P + 0.25H + 0.30G + 0.10Popularidad.
    double afinidad(const Usuario& usuario, const Contenido& candidato,
                   const TablaHash<Contenido>& catalogo, const MatrizDispersa& matriz,
                   const Grafo& grafo) const;
    // Devuelve la cantidad escrita en salida (con capacidad >= limite).
    int generar(const Usuario& usuario, const TablaHash<Contenido>& catalogo,
                const MatrizDispersa& matriz, const Grafo& grafo,
                Candidato* salida, int limite) const;
};
