#include "file_storage.h"
#include "message.h"

namespace ntc::file {
    bool FileStorage::checkDiskExistence() {
        //判断path_prefix_ 文件夹是否存在，否则创建
        if (!std::filesystem::exists(path_prefix_)) {
            std::filesystem::create_directory(path_prefix_);
        }

        auto p = path_prefix_ / std::filesystem::path(id_);        

        return std::filesystem::exists(p);
    }

    bool FileStorage::saveToDisk(const netdesign2::File &file) {
        auto p = path_prefix_ / std::filesystem::path(id_);
        

        std::ofstream ofs(p, std::ios::binary);
        if (!ofs.is_open()) {
            return false;
        }
        LOG(DEBUG)<<file.content().size();
        ofs.write(file.content().data(), file.content().size());
        if (!ofs.good()) {
            LOG(ERROR) << "Failed to write file to disk";
            return false;
        }
        return true;
    }

    bool FileStorage::loadIntoProto(netdesign2::File &file) {
        auto p = path_prefix_ / std::filesystem::path(id_);

        std::ifstream ifs(p, std::ios::binary);
        if (!ifs.is_open()) {
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        file.set_content(content);
        file.set_name(name_);

        return true;
    }
}