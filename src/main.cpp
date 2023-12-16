#include <thread>

#include "Server.h"
#include "db.h"
#include "logging.h"
#include "user_manager.h"
INITIALIZE_EASYLOGGINGPP
using namespace ntc;

int main(int argc, char *argv[]) {
  el::configureServerLogger();
  LOG(DEBUG) << "My first info log using default logger";

  // Create DB
  DataAccess db("chat.db");

  Server *server = Server::Get();
  UM::Get()->setUserInfo("123", 1, "123456");
  UM::Get()->setUserInfo("456", 2, "123456");

  return 0;
}