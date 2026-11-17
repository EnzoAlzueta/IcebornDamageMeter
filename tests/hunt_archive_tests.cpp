#include "../src/hunt_archive.hpp"
#include <cstdio>
#include <fstream>
using namespace meter;
static int failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
static HuntRecord record(std::uint64_t stamp){
    HuntRecord r;r.timestamp=stamp;r.questId=461;r.outcome=3;r.partySize=1;r.durationMs=60000;
    r.players[0]={1,L"Hunter 日本語 \U0001F3F9",3000,true,true,11};r.players[0].participationMs=60000;return r;
}
static std::string bytes(const std::filesystem::path& path){std::ifstream in(path,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
static void write(const std::filesystem::path& path,std::string_view data){std::ofstream out(path,std::ios::binary|std::ios::trunc);out.write(data.data(),static_cast<std::streamsize>(data.size()));}
int main(){
    using namespace archive_codec;
    CHECK(checksum("123456789")==0xcbf43926);
    const auto directory=std::filesystem::current_path()/(L"archive-test-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    std::filesystem::create_directories(directory);
    const auto path=directory/L"hunts.bin";
    {
        const auto r=record(1000);const auto encoded=encode(r);HuntRecord decoded;
        CHECK(decode(encoded,decoded));CHECK(decoded.players[0].name==r.players[0].name);
        CHECK(decoded.questId==461&&decoded.players[0].weapon==11&&decoded.players[0].dps()==50);
        CHECK(!decode(encoded.substr(0,encoded.size()-1),decoded));
        CHECK(!decode(encoded+"x",decoded));
        auto largest=r;largest.partySize=4;
        for(size_t i=0;i<largest.players.size();++i){largest.players[i]=r.players[0];largest.players[i].id=i+1;largest.players[i].name.assign(64,L'X');}
        CHECK(encode(largest).size()==MaxPayloadBytes);CHECK(decode(encode(largest),decoded));
    }
    std::string first,both;
    {
        HuntArchive archive;CHECK(archive.open(path));CHECK(archive.bytesRead()==0);
        CHECK(archive.append(record(1000))==HuntArchive::Result::Appended);
        first=bytes(path);CHECK(first.size()==HeaderBytes+encode(record(1000)).size()+FrameOverhead);
        CHECK(archive.append(record(1000))==HuntArchive::Result::Duplicate);CHECK(bytes(path)==first);
        HuntArchive conflicting;CHECK(!conflicting.open(path));
        CHECK(archive.append(record(2000))==HuntArchive::Result::Appended);both=bytes(path);
    }
    // Reopening is bounded and recognizes records imported from recent history.
    {
        HuntArchive archive;CHECK(archive.open(path));CHECK(archive.cachedRecords()==2);
        CHECK(archive.append(record(1000))==HuntArchive::Result::Duplicate);
        CHECK(archive.append(record(2000))==HuntArchive::Result::Duplicate);
        CHECK(bytes(path)==both);
    }
    const auto frameSize=both.size()-first.size();
    for(const size_t cut:{size_t(1),size_t(4),size_t(7),size_t(8),size_t(9),frameSize/2,frameSize-1}){
        write(path,std::string_view(both).substr(0,first.size()+cut));
        HuntArchive archive;CHECK(archive.open(path));CHECK(archive.recoveredBytes()==cut);
        CHECK(bytes(path)==first);CHECK(archive.append(record(2000))==HuntArchive::Result::Appended);CHECK(bytes(path)==both);
    }
    // Also recover an interrupted very first frame, without inventing a record.
    for(const size_t cut:{size_t(1),size_t(7),size_t(8),frameSize-1}){
        write(path,std::string_view(first).substr(0,HeaderBytes+cut));
        HuntArchive archive;CHECK(archive.open(path));CHECK(archive.cachedRecords()==0);
        CHECK(std::filesystem::file_size(path)==HeaderBytes);CHECK(archive.recoveredBytes()==cut);
        CHECK(archive.append(record(1000))==HuntArchive::Result::Appended);CHECK(bytes(path)==first);
    }
    // Complete corrupted records, unknown headers and foreign trailing data are preserved.
    auto corrupt=both;corrupt[first.size()+20]^=1;
    auto badHeader=both;badHeader[12]^=1;
    for(const auto& damaged:{corrupt,badHeader,std::string("unrelated existing file"),both+"foreign tail"}){
        write(path,damaged);HuntArchive archive;CHECK(!archive.open(path));CHECK(bytes(path)==damaged);
    }
    write(path,both);
    {HuntArchive wrongBuild;CHECK(!wrongBuild.open(path,123));CHECK(bytes(path)==both);}
    {
        HuntArchive archive;CHECK(archive.open(path));auto bad=record(3000);bad.players[0].weapon=999;
        CHECK(archive.append(bad)==HuntArchive::Result::Failed);CHECK(bytes(path)==both);
    }
    const auto large=directory/L"large.bin";
    // A crash may leave the archive ahead of the ten-entry history. Importing an
    // old missing entry must not evict anchors and duplicate the entire snapshot.
    {
        History history;for(unsigned i=1;i<=10;++i)history.add(record(i));
        HuntArchive archive;CHECK(archive.open(directory/L"seed-ahead.bin"));
        for(unsigned i=1;i<=11;++i)CHECK(archive.append(record(i),false)==HuntArchive::Result::Appended);
        CHECK(archive.flush());archive.close();CHECK(archive.open(directory/L"seed-ahead.bin"));
        unsigned imported=99;CHECK(archive.importRecent(history,imported));CHECK(imported==0);
        history.add(record(12));CHECK(archive.importRecent(history,imported));CHECK(imported==1);
        CHECK(archive.importRecent(history,imported));CHECK(imported==0);
    }
    {
        History history;for(unsigned i=1;i<=12;++i)history.add(record(i));
        HuntArchive archive;CHECK(archive.open(directory/L"seed-behind.bin"));
        for(unsigned i=1;i<=10;++i)CHECK(archive.append(record(i),false)==HuntArchive::Result::Appended);
        unsigned imported=0;CHECK(archive.importRecent(history,imported));CHECK(imported==2);
        CHECK(archive.importRecent(history,imported));CHECK(imported==0);
    }
    {HuntArchive archive;CHECK(archive.open(large));for(unsigned i=1;i<=1000;++i)CHECK(archive.append(record(i),false)==HuntArchive::Result::Appended);CHECK(archive.flush());}
    {
        HuntArchive archive;CHECK(archive.open(large));CHECK(archive.bytesRead()<=HeaderBytes+TailBytes);
        CHECK(archive.cachedRecords()==10);CHECK(archive.append(record(999))==HuntArchive::Result::Duplicate);
        CHECK(archive.append(record(1001))==HuntArchive::Result::Appended);
    }
    CHECK(failures==0);
    std::printf("Archive codec, Unicode, duplicate suppression, bounded reopening, interrupted-tail recovery and corruption preservation: %s\n",failures?"FAILED":"passed");
    return failures?1:0;
}
