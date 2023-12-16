#ifndef GROUP_H
#define GROUP_H

#include "user.h"

namespace ntc {
class Group {
 private:
  int id_;
  std::string name_;

 public:
  Group() : id_(0), name_("") {}
  Group(int id, std::string_view name) : id_(id), name_(name) {}
};

}  // namespace ntc

#endif