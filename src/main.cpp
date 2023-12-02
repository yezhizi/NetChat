#include "logging.h"
#include "Client.h"
#include "Server.h"
#include "uuid/uuid.h"
INITIALIZE_EASYLOGGINGPP
using namespace ntc;

int main(int argc, char* argv[]) {
   el::configureServerLogger();
   // LOG(DEBUG) << "My first info log using default logger";

   Server * server   = Server::Get();
   // Client * client   = Client::getInstance();
   // client->Signup("123","123456");
   // delete server;
   // delete client;
   uuid_t uu;
   uuid_generate(uu);
   std::cout << uu << std::endl;

   return 0;
}