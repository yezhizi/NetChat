#include "Server.h"
#include "logging.h"
#include "user_manager.h"
#include <thread>
INITIALIZE_EASYLOGGINGPP
using namespace ntc;

int main(int argc, char *argv[]) {
    el::configureServerLogger();
    LOG(DEBUG) << "My first info log using default logger";

    Server *server = Server::Get();
    UM::Get()->setUserInfo("123", 1, "123456");
    UM::Get()->setUserInfo("456", 2, "123456");

    return 0;
}