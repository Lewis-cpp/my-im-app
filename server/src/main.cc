#include <drogon/drogon.h>
#include <iostream>

using namespace drogon;

int main() {
    std::cout << "Starting IM Server..." << std::endl;
    
    // Load configuration file
    app().loadConfigFile("config.json");
    
    // Start the server
    app().run();
    
    return 0;
}