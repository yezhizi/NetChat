#include "Server.h"



void Server::Init()
{
    this->_van = (Van*)new ServerVan();

}

void Server::Finalize()
{
    delete this->_van;
    LOG(INFO) << "Server closed";
}
