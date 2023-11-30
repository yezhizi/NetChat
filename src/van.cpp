#include "van.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "logging.h"
#include "message.h"
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

int Van::Send(const Packet& msg)
{
    int send_bytes = SendMesg(msg);
    if (send_bytes == -1) return -1;
    this->send_bytes_ += send_bytes;
    return send_bytes;
}

Van::Van()
{
    this->_socket = socket(AF_INET, SOCK_STREAM, 0);
    LOG_IF(this->_socket < 0, FATAL) << "socket() failed";
}



