//
// AUTOR: Yanira Suni & Alonso Chullunquia
//
#include <iostream>
#include <fstream>
#include "Classes/Page.h"
#include "Classes/Frame.h"
#include "Classes/BufferManager.h"


int main() {
    // inicializando buffer manager
    BufferManager buffer_manager(3);

    // inicializando mainFrame puntero a Frame para mostrar la página
    Frame* mainFrame;

    // menú de acciones
    int option;
    do {
        std::cout << "1. Solicitar página\n";
        std::cout << "2. Liberar página\n";
        std::cout << "3. Mostrar páginas\n";
        std::cout << "4. Salir\n";
        std::cin >> option;
        switch (option) {

            // solicitar página
            case 1:
                int block_id;
                std::cout << "Ingrese el block_id: ";
                std::cin >> block_id;
                mainFrame = buffer_manager.requestPage(block_id);

                // si la página no se pudo cargar por falta de espacio
                if(mainFrame == nullptr) {
                    std::cout << "No se pudo cargar la página\n";
                    break;
                }

                mainFrame->showPage();
                // modificar la data
                std::cout << "desea modificar la data? (1: si, 0: no): ";
                int mod;
                std::cin >> mod;
                if (mod == 1) {
                    std::string data;
                    std::cout << "Ingrese la nueva data: ";
                    //cin de linea entera
                    std::cin.ignore();
                    std::getline(std::cin, data);
                    mainFrame->page->data = data;
                    mainFrame->page->dirty = true;
                }
                break;

            // liberar página
            case 2:
                int page_id;
                std::cout << "Ingrese el page_id: ";
                std::cin >> page_id;
                buffer_manager.releasePage(page_id,buffer_manager.getPage(page_id)->dirty);
                break;

            // mostrar páginas
            case 3:
                buffer_manager.showFrames();
                break;

            // salir
            case 4:
                break;
            default:
                std::cout << "Opción inválida\n";
                break;
        }
    } while (option != 4);
    return 0;
}