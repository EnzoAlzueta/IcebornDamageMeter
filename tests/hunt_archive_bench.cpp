#include "../src/hunt_archive.hpp"
#include <psapi.h>
#include <chrono>
#include <cstdio>
using namespace meter;
using Clock=std::chrono::steady_clock;
static double ms(Clock::time_point start){return std::chrono::duration<double,std::milli>(Clock::now()-start).count();}
static HuntRecord record(std::uint64_t stamp){
    HuntRecord r;r.timestamp=stamp;r.questId=461;r.outcome=3;r.partySize=4;r.durationMs=60000;
    for(size_t i=0;i<4;++i){r.players[i]={i+1,L"SamplePlayer",3000,i==0,true,static_cast<int>(i)};r.players[i].participationMs=60000;}
    return r;
}
int main(){
    const auto directory=std::filesystem::current_path()/(L"archive-bench-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    std::filesystem::create_directories(directory);const auto path=directory/L"hunts.bin";
    HuntArchive archive;if(!archive.open(path))return 1;
    unsigned written=0;
    std::puts("Background-writer benchmark; warm filesystem cache, not an in-game FPS measurement. Setup writes are batched; measured appends each flush to disk.");
    for(const unsigned target:{1000u,10000u,100000u}){
        while(written<target){if(archive.append(record(++written),false)!=HuntArchive::Result::Appended)return 2;}
        if(!archive.flush())return 3;archive.close();
        const auto start=Clock::now();if(!archive.open(path))return 4;const auto openMs=ms(start);
        const auto read=archive.bytesRead();
        if(read>archive_codec::HeaderBytes+archive_codec::TailBytes||archive.cachedRecords()>10)return 5;
        PROCESS_MEMORY_COUNTERS_EX memory{};memory.cb=sizeof(memory);
        if(!GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memory),sizeof(memory)))return 6;
        std::array<double,32> samples{};
        for(auto& duration:samples){const auto before=Clock::now();if(archive.append(record(++written))!=HuntArchive::Result::Appended)return 7;duration=ms(before);}
        std::sort(samples.begin(),samples.end());
        std::printf("{\"hunts\":%u,\"archiveBytes\":%llu,\"openMs\":%.4f,\"startupBytesRead\":%llu,\"cachedRecords\":%zu,\"processPrivateBytes\":%zu,\"durableAppendMedianMs\":%.4f,\"durableAppendP95Ms\":%.4f,\"durableAppendMaxMs\":%.4f}\n",
            target,static_cast<unsigned long long>(std::filesystem::file_size(path)),openMs,read,archive.cachedRecords(),memory.PrivateUsage,samples[16],samples[30],samples[31]);
    }
    return 0;
}
