#include <memory>
#include <thread>

#include "Server.h"
#include "db_access.h"
#include "user_manager.h"
#include "utils/logging.h"

INITIALIZE_EASYLOGGINGPP

namespace ntc {
std::unique_ptr<DataAccess> g_db = nullptr;
}

int main(int argc, char *argv[]) {
  el::configureServerLogger();

  // 创建全局的 Db 操作对象
  ntc::g_db = std::make_unique<ntc::DataAccess>("chat.db");

  ntc::Server &server = ntc::Server::Get();

  return 0;
}