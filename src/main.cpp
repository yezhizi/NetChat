#include "logging.h"
#include "meta.pb.h"
#include "Client.h"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char* argv[]) {
   el::configureClientLogger();
   LOG(DEBUG) << "My first info log using default logger";
   Client * client   = Client::getInstance();
   client->Signup("123","123456");
   

}