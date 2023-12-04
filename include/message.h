#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include "base.h"
#include "messages.pb.h"
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include <memory>
#include <string>
#include <unordered_map>
namespace ntc {
using namespace netdesign2;

// class Message {
//   public:
//     std::shared_ptr<google::protobuf::Message>
//     getMessageByName(const std::string &msgType) {
//         auto desc = google::protobuf::DescriptorPool::generated_pool()
//                         ->FindMessageTypeByName(msgType);
//         if (!desc)
//             return std::shared_ptr<google::protobuf::Message>(nullptr);
//         auto instance =
//             google::protobuf::MessageFactory::generated_factory()->GetPrototype(
//                 desc);
//         if (!instance)
//             return std::shared_ptr<google::protobuf::Message>(nullptr);
//         std::shared_ptr<google::protobuf::Message> msg =
//             std::shared_ptr<google::protobuf::Message>(instance->New());
//         return msg;
//     };
// };

// ... (Previous code remains unchanged)

class Message {
  public:
};
enum class MessageType {
    ServerStatusRequest = 0,
    ServerStatusResponse = 1,
    ServerStatusUpdateRequest = 6,
    ServerStatusUpdateResponse = 7,
    LoginPreRequest = 2,
    LoginPreResponse = 3,
    LoginRequest = 4,
    LoginResponse = 5,
    ContactListRequest = 8,
    ClientAckResponse = 9,
    ContactRequest = 10,
    ServerAckResponse = 11,
    ContactMessageRequest = 12,
    ContactMessageResponse = 13,
    FriendSendMessageRequest = 14,
    FriendSendMessageResponse = 15,
    SetupChannelRequest = 16,
    ChannelHeartRequest = 17
};

} // namespace ntc

#endif