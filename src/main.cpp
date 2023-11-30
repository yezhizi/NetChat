#include "logging.h"
#include "Client.h"
#include "Server.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
   el::configureServerLogger();
   // LOG(DEBUG) << "My first info log using default logger";

   Server * server   = Server::getInstance();
   Client * client   = Client::getInstance();
   client->Signup("123","123456");
   // delete server;
   // delete client;
   

}