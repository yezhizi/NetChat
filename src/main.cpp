#include "Server.h"
#include "logging.h"
#include "Client.h"
#include "user_manager.h"
INITIALIZE_EASYLOGGINGPP
using namespace ntc;

int main(int argc, char *argv[]) {
    el::configureServerLogger();
    // LOG(DEBUG) << "My first info log using default logger";

    Server *server = Server::Get();
    Client * client   = Client::getInstance();
    
    client->sendServerStatusRequest();

    UM::Get()->setUserInfo("123", "123456");

    return 0;
}