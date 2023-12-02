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
}