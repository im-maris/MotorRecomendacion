#pragma once
#include <string>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <limits>

inline std::string recortar(const std::string& texto) {
    size_t inicio = texto.find_first_not_of(" \t\r\n");
    if (inicio == std::string::npos) return "";
    size_t fin = texto.find_last_not_of(" \t\r\n");
    return texto.substr(inicio, fin - inicio + 1);
}

inline std::string minusculas(std::string texto) {
    for (char& c : texto)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return texto;
}

inline bool contieneTexto(const std::string& texto, const std::string& fragmento) {
    if (fragmento.empty()) return false;
    return minusculas(texto).find(minusculas(fragmento)) != std::string::npos;
}

inline bool textoIgual(const std::string& a, const std::string& b) {
    return minusculas(recortar(a)) == minusculas(recortar(b));
}

inline bool leerEntero(const std::string& linea, int& numero) {
    std::istringstream entrada(linea);
    if (!(entrada >> numero)) return false;
    std::string extra;
    return !(entrada >> extra);
}

inline bool leerDecimal(const std::string& linea, double& numero) {
    std::istringstream entrada(linea);
    if (!(entrada >> numero)) return false;
    std::string extra;
    return !(entrada >> extra);
}
