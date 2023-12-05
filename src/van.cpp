#include "van.h"
#include "logging.h"
#include "message.h"
#include <arpa/inet.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <sys/socket.h>
#include <unistd.h>
namespace ntc {

int Van::Send(const Packet &msg, const std::string to, const std::string from) {
    int send_bytes = SendMesg(msg, to, from);
    if (send_bytes == -1)
        return -1;
    this->send_bytes_ += send_bytes;
    LOG(INFO) << "successfully send a packet send bytes: " << send_bytes;
    return send_bytes;
}
int Van::Send(const Packet &mag, const int fd) {
    int send_bytes = SendMesg(mag, fd);
    if (send_bytes == -1)
        return -1;
    this->send_bytes_ += send_bytes;
    LOG(INFO) << "successfully send a packet send bytes: " << send_bytes;
    return send_bytes;
}
int Van::Recv(const int client_fd, Packet *msg) {
    int recv_bytes = RecvMesg(client_fd, msg);
    if (recv_bytes == -1)
        return -1;
    this->recv_bytes_ += recv_bytes;
    return recv_bytes;
}

Van::Van() {
    this->_socket = socket(AF_INET, SOCK_STREAM, 0);
    LOG_IF(this->_socket < 0, FATAL) << "socket() failed";
}

} // namespace ntc
