#include <iostream>

#include "sheet.hpp"

int main() {
    std::cout << "Start" << std::endl;
    try {
        sheet::Client client;
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}
