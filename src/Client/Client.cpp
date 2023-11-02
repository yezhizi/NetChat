#include "Client.h"
#include "client_van.h"

void Client::Init() {
    this->_client_id = ntc::kvEmpty;
    this->_van = (Van*)new ClientVan();

}
void Client::Finalize() {
    delete this->_van;
    LOG(INFO) << "Client closed";
}

int Client::Signup(std::string phone_number, const std::string& password)
{
    Message msg;
    msg.meta.type = Meta::Type::SIGNUP;
    msg.meta.code = Meta::Code::REQUEST;
    msg.meta.src = this->_client_id;
    msg.meta.dst = ntc::ServerID;

    SArray<char> phone_number_bytes(phone_number);
    SArray<char> pass_bytes(password);
    msg.AddData(phone_number_bytes);
    msg.AddData(pass_bytes);
    LOG(INFO)<<"send signup message phone_number:"<<phone_number;

    if (this->_van->Send(msg) < 0) {
        LOG(ERROR) << "send signup message failed";
        return -1;
    }
    else {
        LOG(INFO) << "send signup message success";
        return 0;
    }
    

}


