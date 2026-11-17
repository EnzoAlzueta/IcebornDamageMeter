#include "hunt_archive.hpp"
#include <cstring>
#include <sstream>

namespace meter {
namespace {
void put32(char* p,std::uint32_t n){for(unsigned i=0;i<4;++i)p[i]=static_cast<char>((n>>(8*i))&255);}
std::uint32_t get32(const char* p){std::uint32_t n=0;for(unsigned i=0;i<4;++i)n|=static_cast<std::uint32_t>(static_cast<unsigned char>(p[i]))<<(8*i);return n;}
bool incomplete(std::string_view tail){
    if(tail.empty())return true;
    const auto prefix=(std::min)(size_t(4),tail.size());
    if(std::memcmp(tail.data(),"HNT1",prefix))return false;
    if(tail.size()<8)return true;
    const auto bytes=get32(tail.data()+4);
    return bytes>=archive_codec::MinPayloadBytes&&bytes<=archive_codec::MaxPayloadBytes&&tail.size()<bytes+archive_codec::FrameOverhead;
}
// Return the start of a fully validated frame ending at `end` within this window.
bool frameAt(std::string_view window,size_t end,size_t& start,std::string& payload){
    using namespace archive_codec;
    if(end<FrameOverhead||window.substr(end-4,4)!="END1")return false;
    const auto size=get32(window.data()+end-8);
    if(size<MinPayloadBytes+FrameOverhead||size>MaxFrameBytes||size>end)return false;
    start=end-size;
    if(window.substr(start,4)!="HNT1"||get32(window.data()+start+4)!=size-FrameOverhead)return false;
    const auto body=window.substr(start+8,size-FrameOverhead);
    if(checksum(body)!=get32(window.data()+end-12))return false;
    HuntRecord record;
    if(!decode(body,record))return false;
    payload.assign(body);
    return true;
}
}
namespace archive_codec {
std::uint32_t checksum(std::string_view data){
    static constexpr auto table=[](){
        std::array<std::uint32_t,256> values{};
        for(unsigned i=0;i<256;++i){auto c=static_cast<std::uint32_t>(i);for(int b=0;b<8;++b)c=(c>>1)^((c&1)?0xedb88320u:0u);values[i]=c;}
        return values;
    }();
    std::uint32_t crc=0xffffffffu;
    for(const unsigned char c:data)crc=table[(crc^c)&255]^(crc>>8);
    return crc^0xffffffffu;
}
std::string encode(const HuntRecord& record){
    static_assert(sizeof(wchar_t)==2,"Archive v1 uses Windows UTF-16 code units.");
    std::ostringstream out(std::ios::out|std::ios::binary);
    if(!history_codec::writeRecord(out,record))return {};
    return out.str();
}
bool decode(std::string_view data,HuntRecord& record){
    if(data.size()<MinPayloadBytes||data.size()>MaxPayloadBytes)return false;
    std::istringstream in(std::string(data),std::ios::in|std::ios::binary);
    HuntRecord next;
    if(!history_codec::readRecord(in,next)||in.peek()!=std::char_traits<char>::eof())return false;
    record=std::move(next);return true;
}
}
void HuntArchive::close(){if(file_!=INVALID_HANDLE_VALUE){CloseHandle(file_);file_=INVALID_HANDLE_VALUE;}recent_.clear();}
bool HuntArchive::fail(const char* reason){error_=reason;close();return false;}
bool HuntArchive::readAt(std::uint64_t offset,char* data,DWORD count){
    LARGE_INTEGER position;position.QuadPart=static_cast<LONGLONG>(offset);
    if(!SetFilePointerEx(file_,position,nullptr,FILE_BEGIN))return false;
    DWORD read=0;if(!ReadFile(file_,data,count,&read,nullptr)||read!=count)return false;
    bytesRead_+=read;return true;
}
bool HuntArchive::writeAt(std::uint64_t offset,const char* data,DWORD count){
    LARGE_INTEGER position;position.QuadPart=static_cast<LONGLONG>(offset);
    if(!SetFilePointerEx(file_,position,nullptr,FILE_BEGIN))return false;
    DWORD written=0;return WriteFile(file_,data,count,&written,nullptr)&&written==count;
}
void HuntArchive::remember(std::string payload){
    if(recent_.size()==archive_codec::RecentRecords)recent_.erase(recent_.begin());
    recent_.push_back(std::move(payload));
}
bool HuntArchive::flush(){
    if(file_==INVALID_HANDLE_VALUE)return false;
    return FlushFileBuffers(file_)!=0||fail("archive flush failed");
}
bool HuntArchive::open(const std::filesystem::path& path,std::uint32_t gameBuild){
    using namespace archive_codec;
    close();bytesRead_=recoveredBytes_=end_=0;error_="none";recent_.reserve(RecentRecords);
    file_=CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,nullptr,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(file_==INVALID_HANDLE_VALUE)return fail("archive open failed");
    LARGE_INTEGER size{};
    if(!GetFileSizeEx(file_,&size)||size.QuadPart<0)return fail("archive size failed");
    end_=static_cast<std::uint64_t>(size.QuadPart);
    std::array<char,HeaderBytes> header{};
    std::memcpy(header.data(),"IDMHARC1",8);put32(header.data()+8,1);put32(header.data()+12,gameBuild);
    put32(header.data()+16,static_cast<std::uint32_t>(MaxPayloadBytes));
    put32(header.data()+20,checksum(std::string_view(header.data(),20)));
    if(!end_){
        if(!writeAt(0,header.data(),static_cast<DWORD>(header.size())))return fail("archive header write failed");
        end_=HeaderBytes;return flush();
    }
    std::array<char,HeaderBytes> existing{};
    if(end_<HeaderBytes||!readAt(0,existing.data(),static_cast<DWORD>(existing.size()))||existing!=header)return fail("archive header incompatible or damaged; file preserved");
    if(end_==HeaderBytes)return true;
    const auto bytes=static_cast<size_t>((std::min)(static_cast<std::uint64_t>(TailBytes),end_-HeaderBytes));
    const auto offset=end_-bytes;
    std::string tail(bytes,'\0');
    if(!readAt(offset,tail.data(),static_cast<DWORD>(bytes)))return fail("archive tail read failed");
    size_t validEnd=tail.size(),start=0;std::string payload;bool found=false;
    while(validEnd){
        if(frameAt(tail,validEnd,start,payload)){found=true;break;}
        --validEnd;
    }
    if(!found&&offset!=HeaderBytes)return fail("archive tail damaged; file preserved");
    if(!incomplete(std::string_view(tail).substr(validEnd)))return fail("archive tail damaged; file preserved");
    if(validEnd<tail.size()){
        const auto recoveredEnd=offset+validEnd;
        LARGE_INTEGER position;position.QuadPart=static_cast<LONGLONG>(recoveredEnd);
        if(!SetFilePointerEx(file_,position,nullptr,FILE_BEGIN)||!SetEndOfFile(file_))return fail("archive partial-tail recovery failed");
        recoveredBytes_=end_-recoveredEnd;end_=recoveredEnd;
        if(!flush())return false;
    }
    if(found){
        recent_.push_back(std::move(payload));
        while(recent_.size()<RecentRecords&&frameAt(tail,start,validEnd,payload)){
            recent_.push_back(std::move(payload));start=validEnd;
        }
        std::reverse(recent_.begin(),recent_.end());
    }
    return true;
}
HuntArchive::Result HuntArchive::append(const HuntRecord& record,bool durable){
    using namespace archive_codec;
    if(file_==INVALID_HANDLE_VALUE)return Result::Failed;
    auto payload=encode(record);HuntRecord checked;
    if(!decode(payload,checked)){fail("invalid archive record; file preserved");return Result::Failed;}
    if(std::find(recent_.begin(),recent_.end(),payload)!=recent_.end())return Result::Duplicate;
    std::array<char,MaxFrameBytes> frame{};
    const auto bytes=static_cast<DWORD>(payload.size()+FrameOverhead);
    std::memcpy(frame.data(),"HNT1",4);put32(frame.data()+4,static_cast<std::uint32_t>(payload.size()));
    std::memcpy(frame.data()+8,payload.data(),payload.size());
    put32(frame.data()+bytes-12,checksum(payload));put32(frame.data()+bytes-8,bytes);std::memcpy(frame.data()+bytes-4,"END1",4);
    if(!writeAt(end_,frame.data(),bytes)){fail("archive append failed");return Result::Failed;}
    end_+=bytes;
    if(durable&&!flush())return Result::Failed;
    remember(std::move(payload));return Result::Appended;
}
bool HuntArchive::importRecent(const History& history,unsigned& imported){
    imported=0;
    // Find the newest common record before appending anything. Importing older
    // misses first could evict common records and cascade duplicate imports when
    // the archive is one hunt ahead of the recent-history snapshot after a crash.
    size_t newer=history.size();
    for(size_t i=0;i<history.size();++i){
        const auto payload=archive_codec::encode(*history.get(i));
        if(std::find(recent_.begin(),recent_.end(),payload)!=recent_.end()){newer=i;break;}
    }
    unsigned appended=0;
    for(size_t i=newer;i>0;--i){
        const auto result=append(*history.get(i-1),false);
        if(result==Result::Failed)return false;
        if(result==Result::Appended)++appended;
    }
    if(appended&&!flush())return false;
    imported=appended;return true;
}
}
