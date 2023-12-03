#include "Server.h"
#include "server_van.h"
namespace ntc{


void Server::Init()
{
    LOG(INFO) << "Server initialized";
    this->_van = (Van*)new ServerVan();
    // init UM
    UM::Get()->BindServer(this); 

}

void Server::Finalize()
{
    delete this->_van;
    LOG(INFO) << "Server closed";
    
}
//收到临时连接池的请求
void Server::processRevcSocket(int client_fd) {
    //TODO 
}
} // namespace ntc