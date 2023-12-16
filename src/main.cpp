#include <memory>
#include <thread>

#include "Server.h"
#include "db_access.h"
#include "logging.h"
#include "user_manager.h"
INITIALIZE_EASYLOGGINGPP
using namespace ntc;

int main(int argc, char *argv[]) {
  el::configureServerLogger();
  
  // 创建全局的 Db 操作对象
  g_db = std::make_unique<DataAccess>("chat.db");

  Server *server = Server::Get();
  UM::Get()->setUserInfo("123", 1, "123456");
  UM::Get()->setUserInfo("456", 2, "123456");

  return 0;
}