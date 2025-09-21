#pragma once
#ifndef WSOCKET__COMPRESS_MANAGER_HPP
#define WSOCKET__COMPRESS_MANAGER_HPP

#include <sstream>
#include <string>
#include <unordered_map>

#ifdef WITH_ZSTD
#include "Zstd.hpp"
#endif


namespace wsocket {

// 字符串分割辅助函数
inline std::vector<std::string> split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream        ss(str);
    std::string              token;
    while(std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

class CompressManager {
    CompressManager() {
#ifdef WITH_ZSTD
        RegisterCompressor(std::make_shared<ZstdContext>());
#endif
    }

public:
    ~CompressManager() = default;

    static CompressManager &Instance() {
        static CompressManager instance;
        return instance;
    }

    std::string GetSupportedCompressors() { return supported_compressors_; }
    std::string GetSupportedCompressors(const std::vector<CompressType> &types) {
        std::string s;
        for(auto type : types) {
            auto it = compress_ctxs_.find(type);
            if(it != compress_ctxs_.end()) {
                s += s.empty() ? (it->second->Name()) : (";" + it->second->Name());
            }
        }
        return s;
    }

    std::shared_ptr<CompressContext> GetCompressContext(CompressType type) {
        auto it = compress_ctxs_.find(type);
        if(it == compress_ctxs_.end()) {
            return nullptr;
        }
        return it->second->Create();
    }

    std::vector<CompressType> GetSupportedCompressTypes(const std::string &message) {
        std::vector<CompressType> types;

        // split with ';'
        for(auto &s : split(message, ';')) {
            auto it = compress_names_.find(s);
            if(it != compress_names_.end()) {
                types.push_back(it->second);
            }
        }

        return types;
    }

private:
    void RegisterCompressor(std::shared_ptr<CompressContext> cxt) {
        if(!supported_compressors_.empty()) {
            supported_compressors_ += ";";
        }
        assert(!cxt->Name().empty());
        supported_compressors_ += cxt->Name();

        compress_names_.insert(std::make_pair(cxt->Name(), cxt->Type()));
        compress_ctxs_.insert(std::make_pair(cxt->Type(), cxt));
    }

private:
    std::string                                                        supported_compressors_;
    std::unordered_map<std::string, CompressType>                      compress_names_;
    std::unordered_map<CompressType, std::shared_ptr<CompressContext>> compress_ctxs_;
};


} // namespace wsocket

#endif // WSOCKET__COMPRESS_MANAGER_HPP
