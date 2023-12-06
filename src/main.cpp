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
    UM::Get()->setUserInfo("123", 1, "123456");
    UM::Get()->setUserInfo("456", 2, "123456");
    Client *client1 = new Client();
    Client *client2 = new Client();

    std::thread t1([&]() {
        client1->sendServerStatusRequest();
        client2->sendServerStatusRequest();
        client1->sendServerStatusUpdateRequest();
        client2->sendServerStatusUpdateRequest();
        client1->login("123", "123456");
        client2->login("456", "123456");
        for (int i = 0; i < 10; ++i) {
            client1->sendServerStatusRequest();
            client2->sendServerStatusRequest();
            client1->sendServerStatusUpdateRequest();
            client2->sendServerStatusUpdateRequest();
        }
        client1->setupChannel();
        client2->setupChannel();
    });
    t1.join();
    delete client1;
    delete client2;

    return 0;
}