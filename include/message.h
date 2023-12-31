#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "proto/messages.pb.h"
namespace ntc {

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
  SendMessageRequest = 14,
  SendMessageResponse = 15,
  SetupChannelRequest = 16,
  ChannelHeartRequest = 17,
  ContactMessageListRequest = 18,
  FileUploadRequest = 19,
  FileUploadResponse = 24,
  FileDownloadRequest = 20,
  FileDownloadResponse = 21,
  RegisterRequest = 22,
  RegisterResponse = 23,
  DeleteMessageRequest = 25,
  DeleteMessageResponse = 26,
};

}  // namespace ntc

#endif