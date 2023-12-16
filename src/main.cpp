#include <memory>
#include <thread>

#include "Server.h"
#include "logging.h"
#include "user_manager.h"

INITIALIZE_EASYLOGGINGPP

using namespace ntc;

int main(int argc, char *argv[]) {
  el::configureServerLogger();

  // 创建全局的 Db 操作对象
  g_db = std::make_unique<DataAccess>("chat.db");

  Server &server = Server::Get();

  return 0;
}