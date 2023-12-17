#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include <memory>
#include <string>
#include <unordered_map>

#include "base.h"
#include "proto/messages.pb.h"
#include "user.h"
namespace ntc {

using namespace netdesign2;

enum class PacketType {
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

class Message {
 public:
  const User& sender;
  const User& receiver;
  std::string content;
};

}  // namespace ntc

#endif