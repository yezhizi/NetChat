#include "Client.h"
#include "Server.h"
#include "logging.h"
#include "user_manager.h"
#include <thread>
INITIALIZE_EASYLOGGINGPP
using namespace ntc;

int main(int argc, char *argv[]) {
    el::configureServerLogger();
    // LOG(DEBUG) << "My first info log using default logger";

    Server *server = Server::Get();
    UM::Get()->setUserInfo("123456", 1, "123456");
    UM::Get()->setUserInfo("456", 2, "12345678");
    Client *client1 = new Client();
    client1->sendServerStatusRequest();

    // delete client1;
    // client1 = new Client();
    client1->sendServerStatusUpdateRequest();
    client1->sendServerStatusRequest();

    delete client1;
    Client *client2 = new Client();

    client2->login("123456","123456");
    client2->setupChannel();

    

    return 0;
}