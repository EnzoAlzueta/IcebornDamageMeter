#pragma once
#include "history.hpp"
#include <Windows.h>
#include <array>
#include <filesystem>
#include <string_view>
#include <vector>

namespace meter {
namespace archive_codec {
inline constexpr size_t HeaderBytes=24, MinPayloadBytes=525, MaxPayloadBytes=2573;
inline constexpr size_t FrameOverhead=20, MaxFrameBytes=MaxPayloadBytes+FrameOverhead;
inline constexpr size_t RecentRecords=10, TailBytes=(RecentRecords+1)*MaxFrameBytes;
std::uint32_t checksum(std::string_view data);
std::string encode(const HuntRecord& record);
bool decode(std::string_view data,HuntRecord& record);
}

// Owned exclusively by the storage worker. Never called by the render loop.
class HuntArchive {
public:
    enum class Result { Appended, Duplicate, Failed };
    HuntArchive()=default;
    ~HuntArchive(){close();}
    HuntArchive(const HuntArchive&)=delete;
    HuntArchive& operator=(const HuntArchive&)=delete;
    bool open(const std::filesystem::path& path,std::uint32_t gameBuild=421810);
    Result append(const HuntRecord& record,bool durable=true);
    bool importRecent(const History& history,unsigned& imported);
    bool flush();
    void close();
    const char* error() const{return error_;}
    std::uint64_t bytesRead() const{return bytesRead_;}
    std::uint64_t recoveredBytes() const{return recoveredBytes_;}
    size_t cachedRecords() const{return recent_.size();}
private:
    HANDLE file_=INVALID_HANDLE_VALUE;
    std::uint64_t end_=0,bytesRead_=0,recoveredBytes_=0;
    std::vector<std::string> recent_;
    const char* error_="none";
    bool fail(const char* reason);
    bool readAt(std::uint64_t offset,char* data,DWORD count);
    bool writeAt(std::uint64_t offset,const char* data,DWORD count);
    void remember(std::string payload);
};
}
