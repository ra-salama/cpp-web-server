#include "Server.h"
#include <iostream>

int main(){
    try{
        Server server(8080);
        if (server.init()){
            server.run();
        }
    } catch (const std::exception& e){
        std::cerr <<"Error: " <<e.what() <<std::endl;
        return 1;
    }
    return 0;
}