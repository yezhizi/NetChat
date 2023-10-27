#include "Client.h"

void Client::Init(){
    _client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (_client_socket == -1){
        std::cout << "Could not create socket" << std::endl;
    }
    std::cout << "Socket created" << std::endl;
    _server.sin_addr.s_addr = inet_addr(SERV::ip.c_str());
    
    
}