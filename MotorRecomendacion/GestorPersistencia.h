#pragma once
#include <string>

class Sistema;

class GestorPersistencia {
public:
    static bool hayAlgunArchivo(const std::string& carpeta);
    static bool cargar(Sistema& sistema, const std::string& carpeta);
    static bool guardar(const Sistema& sistema, const std::string& carpeta);
};
