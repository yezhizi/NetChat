#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H
#include <filesystem>
#include <iostream>
#include <fstream>

#include "message.h"
#include "easylogging++.h"

namespace ntc::file {
class FileStorage {
 public:
  FileStorage() = default;
  ~FileStorage() = default;

  // Getters
  std::string getId() const { return id_; }
  std::string getName() const { return name_; }
  std::string getHash() const { return hash_; }

  // Setters
  void setId(std::string id) { id_ = id; }
  void setName(std::string name) { name_ = name; }
  void setHash(std::string hash) { hash_ = hash; }

  // Methods
  bool checkDiskExistence();
  bool saveToDisk(const netdesign2::File &file);
  bool loadIntoProto(netdesign2::File &file);

 private:
  std::string id_;    // UUID
  std::string name_;  // filename on client side
  std::string hash_;  // hash of the file

  const std::filesystem::path path_prefix_{"files"};
};
}  // namespace ntc::file

#endif  // FILE_STORAGE_H
