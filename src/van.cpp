#include "van.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "logging.h"

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

int Van::Send(const Message& msg)
{
    int send_bytes = SendMesg(msg);
    if (send_bytes == -1) return -1;
    this->send_bytes_ += send_bytes;
    // for(auto r:msg.data){
    //     LOG(DEBUG)<<"send "<<r;
    // }
    return send_bytes;
}

Van::Van()
{
    this->_socket = socket(AF_INET, SOCK_STREAM, 0);
    LOG_IF(this->_socket < 0, FATAL) << "socket() failed";
    LOG(INFO) << "create socket success";
}

void Van::packMeta(const Meta& meta, char** buf, int* size)
{
    /*
    protobuf use varint to encode so the meta size is not fixed,
    so the server dont kown how many bytes to recv, so we need to send the meta size first
    solution: https://stackoverflow.com/questions/9496101/protocol-buffer-over-socket-in-c
    */
    ntc::PBMETA pb;
    pb.set_type(meta.type);
    pb.set_code(meta.code);
    pb.set_dst(meta.dst);
    pb.set_src(meta.src);
    for (auto r : meta.data_size)
    {
        pb.add_data_size(r);
    }

    int meta_size = pb.ByteSize();

    LOG(DEBUG) << "size after pack " << meta_size;

    meta_size += 4;
    *buf = new char[meta_size];
    google::protobuf::io::ArrayOutputStream aos(*buf, meta_size);
    google::protobuf::io::CodedOutputStream coded_output(&aos);
    coded_output.WriteVarint32(pb.ByteSize());   //write the size to the ahead of buffer
    pb.SerializeToCodedStream(&coded_output);

    *size = meta_size;
    LOG(DEBUG) << "size after add the size of the meta length " << *size;

}

