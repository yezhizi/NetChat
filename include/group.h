#ifndef GROUP_H
#define GROUP_H

#include "base.h"

namespace ntc {
class Group {
 private:
  int id_;
  std::string name_;

 public:
  Group() : id_(0), name_("") {}
  Group(int id, std::string_view name) : id_(id), name_(name) {}

  int getId() const { return id_; }
  std::string getName() const { return name_; }

  void setId(int id) { id_ = id; }
  void setName(std::string_view name) { name_ = std::string{name}; }
};

}  // namespace ntc

#endif