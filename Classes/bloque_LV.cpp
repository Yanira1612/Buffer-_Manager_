#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <vector>

void writeHeader(std::ofstream& outFile, size_t num, size_t width) {
    std::string numStr = std::to_string(num);
    size_t padding = width - numStr.length();
    if (padding > 0) {
        outFile << std::string(padding, '-') << numStr;
    } else {
        outFile << numStr.substr(0, width); // En caso de que el número tenga más dígitos que el ancho deseado
    }
}

void inicializarBloque(const std::string& filename, size_t blockSize) {
    std::ofstream outFile(filename);
    if (outFile) {
        // Escribir número de registros (0) y final del espacio libre (blockSize) en la cabecera
        writeHeader(outFile, 0, 4);       // Número de registros
        writeHeader(outFile, blockSize, 4);     // Final del espacio libre
        // Rellenar el bloque con '-' para representar bytes vacíos
        outFile << std::string(blockSize - 8, '-') << "\n";  // Restamos 9 para considerar los 9 bytes de la cabecera
        outFile.close();
        std::cout << "Bloque inicializado correctamente en el archivo " << filename << "\n";
    } else {
        std::cerr << "Error al abrir el archivo " << filename << " para escritura.\n";
    }
}

struct Cabecera {
    size_t numRegistros;
    size_t finFreeSpace;
    std::vector<size_t> offsets;
    std::vector<size_t> longitudes;
};

Cabecera leerCabecera(const std::string& filename) {
    std::ifstream inFile(filename);
    Cabecera cabecera;

    if (inFile) {
        std::string line;
        // Leer número de registros y fin del free space
        if (std::getline(inFile, line)) {
            // Convertir los caracteres '-' a '0' para facilitar la conversión a número
            std::replace(line.begin(), line.end(), '-', '0');
            
            cabecera.numRegistros = std::stoi(line.substr(0, 4));
            cabecera.finFreeSpace = std::stoi(line.substr(4, 4));

            // Leer offsets y longitudes si hay registros
            if (cabecera.numRegistros > 0) {
                size_t offsetStart = 8; // Posición donde comienzan los offsets en el archivo
                size_t recordInfoLength = 8; // Tamaño en bytes para cada conjunto de offset y longitud

                for (size_t i = 0; i < cabecera.numRegistros; ++i) {
                    size_t offset = std::stoi(line.substr(offsetStart + i * recordInfoLength, 4));
                    size_t length = std::stoi(line.substr(offsetStart + i * recordInfoLength + 4, 4));
                    cabecera.offsets.push_back(offset);
                    cabecera.longitudes.push_back(length);
                }
            }
        }
        inFile.close();
    } else {
        std::cerr << "Error al abrir el archivo " << filename << " para lectura.\n";
    }

    return cabecera;
}

void addRecord(const std::string& filename, const std::string& registro) {
    std::fstream outFile(filename, std::ios::in | std::ios::out);
    if (outFile) {
        Cabecera cabecera = leerCabecera(filename);
        if((cabecera.finFreeSpace - (cabecera.numRegistros*8+8)) < registro.size()) {
        std::cerr << "No hay suficiente espacio para agregar el registro.\n";
        return;
        }

        int posicion=cabecera.finFreeSpace-registro.size();

        if (posicion < cabecera.finFreeSpace) {
            // Escribir el registro en la posición especificada
            outFile.seekp(posicion);
            outFile.write(registro.c_str(), registro.size());

            // Reescribir la cabecera con los datos actualizados
            outFile.seekp(0);
            cabecera.numRegistros++;
            outFile << std::setw(4) << std::setfill('-') << cabecera.numRegistros;
            outFile << std::setw(4) << std::setfill('-') << cabecera.finFreeSpace-registro.size();

            cabecera.offsets.push_back(posicion+1);
            cabecera.longitudes.push_back(registro.size());
            
            for (int i = 0; i < cabecera.numRegistros; ++i) {
                outFile << std::setw(4) << std::setfill('-') << cabecera.offsets[i];
                outFile << std::setw(4) << std::setfill('-') << cabecera.longitudes[i];
            }
        } else {
            std::cerr << "La posición especificada está fuera del espacio libre del bloque.\n";
        }

        outFile.close();
    } else {
        std::cerr << "Error al abrir el archivo " << filename << " para escritura.\n";
    }
}

void agregarRegistro(const std::string& filename, const std::string& registro, size_t posicion) {
    std::fstream outFile(filename, std::ios::in | std::ios::out);
    if (outFile) {
        Cabecera cabecera = leerCabecera(filename);

        if (posicion < cabecera.finFreeSpace) {
            // Escribir el registro en la posición especificada
            outFile.seekp(cabecera.finFreeSpace-registro.size());
            outFile.write(registro.c_str(), registro.size());

            // Actualizar la longitud del registro si es necesario
            size_t registroSize = registro.size();
            if (cabecera.numRegistros > 0 && cabecera.numRegistros <= cabecera.longitudes.size()) {
                cabecera.longitudes[posicion / 8] = registroSize;
            }

            // Reescribir la cabecera con los datos actualizados
            outFile.seekp(0);
            outFile << std::setw(4) << std::setfill('-') << cabecera.numRegistros;
            outFile << std::setw(4) << std::setfill('-') << cabecera.finFreeSpace;

            for (size_t i = 0; i < cabecera.numRegistros; ++i) {
                outFile << std::setw(4) << std::setfill('-') << cabecera.offsets[i];
                outFile << std::setw(4) << std::setfill('-') << cabecera.longitudes[i];
            }
        } else {
            std::cerr << "La posición especificada está fuera del espacio libre del bloque.\n";
        }

        outFile.close();
    } else {
        std::cerr << "Error al abrir el archivo " << filename << " para escritura.\n";
    }
}

void mostrarContenidoBloque(const std::string& filename) {
    Cabecera cabecera = leerCabecera(filename);
    std::ifstream inFile(filename);

    if (inFile) {
        // Mostrar offsets y longitudes
        if (cabecera.numRegistros > 0) {
            std::cout << "Offsets y longitudes de registros:\n";
            for (size_t i = 0; i < cabecera.numRegistros; ++i) {
                std::cout << "Registro " << i + 1 << ": Offset = " << cabecera.offsets[i]
                          << ", Longitud = " << cabecera.longitudes[i] << "\n";
                std::cout << "Contenido: ";

                // Leer y mostrar el contenido del registro
                inFile.seekg(cabecera.offsets[i]-1);
                std::string contenido(cabecera.longitudes[i], ' ');
                inFile.read(&contenido[0], cabecera.longitudes[i]);
                std::cout << contenido << "\n";
            }
        }
    } else {
        std::cerr << "Error al abrir el archivo " << filename << " para lectura.\n";
    }

    inFile.close();
}

int main() {
    std::string filename = "bloque.txt";
    size_t blockSize = 100;

    inicializarBloque(filename, blockSize);

    addRecord(filename, "10101-Srinivasan-Comp. Sci.-65000");
    addRecord(filename, "10102-Smith-Mathematics-7000");
    addRecord(filename, "10103-Smith-Mathematics-7000");

    mostrarContenidoBloque(filename);

    return 0;
}