#include "../src/native_chat_memory.hpp"
#include <cstdio>
#include <cstring>
using namespace meter;
static int failures=0;
#define CHECK(x) do{if(!(x)){std::printf("FAIL %d: %s\n",__LINE__,#x);++failures;}}while(0)
struct FakeUi {virtual int tag(){return 42;}};
int main() {
    NativeChatPlayback queue;NativeChatState state=NativeChatState::Ready;
    std::vector<std::string> submitted;
    const auto probe=[&]{return state;};
    const auto submit=[&](const std::string& text){submitted.push_back(text);state=NativeChatState::Busy;return true;};
    CHECK(queue.start({"one","two"},0));CHECK(!queue.start({"duplicate"},1));
    queue.tick(200,true,true,false,probe,submit);CHECK(submitted.empty());
    queue.tick(201,true,false,false,probe,submit);CHECK(submitted.size()==1);
    queue.tick(1000,true,false,false,probe,submit);CHECK(submitted.size()==1);
    state=NativeChatState::Ready;queue.tick(1001,true,false,false,probe,submit);
    queue.tick(2500,true,false,false,probe,submit);CHECK(submitted.size()==1);
    queue.tick(2501,true,false,false,probe,submit);CHECK(submitted.size()==2);
    state=NativeChatState::Ready;queue.tick(2600,true,false,false,probe,submit);
    CHECK(queue.result()==ChatResult::Completed&&!queue.start({"cooldown"},3000));
    CHECK(queue.start({"timeout"},16000));state=NativeChatState::Busy;
    queue.tick(22000,true,false,false,probe,submit);CHECK(queue.result()==ChatResult::Failed&&submitted.size()==2);
    CHECK(queue.start({"cancel"},32000));queue.tick(32200,false,false,false,probe,submit);CHECK(queue.result()==ChatResult::Cancelled);
    CHECK(!queue.start({std::string(128,'x')},50000));
    CHECK(!queue.start({std::string("x\0y",3)},50000));
    CHECK(queue.start({"unavailable"},50000));state=NativeChatState::Unavailable;
    queue.tick(50200,true,false,false,probe,submit);CHECK(queue.result()==ChatResult::Failed&&submitted.size()==2);
    // Private allocation only: no game process, input events or network.
    auto memory=static_cast<unsigned char*>(VirtualAlloc(nullptr,0x4000,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE));
    CHECK(memory);if(!memory)return 1;
    FakeUi object;std::memcpy(memory+0x28f8,&object,sizeof(void*));
    NativeChatMemory adapter(reinterpret_cast<std::uintptr_t>(memory));
    CHECK(adapter.probe()==NativeChatState::Ready);
    CHECK(adapter.submit("exact UTF-8 text"));CHECK(memory[0x325e]==1);
    CHECK(std::strcmp(reinterpret_cast<char*>(memory+0x28f8+0x165),"exact UTF-8 text")==0);
    CHECK(adapter.probe()==NativeChatState::Busy&&!adapter.submit("duplicate"));
    memory[0x325e]=0;CHECK(!adapter.submit("preserve draft"));
    CHECK(!adapter.submit("new report","other message"));
    CHECK(adapter.submit("city report","exact UTF-8 text"));
    CHECK(memory[0x325e]==1&&!adapter.submit("duplicate","city report"));
    memory[0x325e]=0;
    memory[0x28f8+0x165]='X';
    CHECK(!adapter.submit("do not replace edited draft","city report"));
    std::memset(memory+0x28f8+0x165,0,128);
    int visible=1;std::memcpy(memory+0x28f8+0x160,&visible,sizeof(visible));
    CHECK(adapter.probe()==NativeChatState::Unavailable&&!adapter.submit("user typing"));
    visible=0;std::memcpy(memory+0x28f8+0x160,&visible,sizeof(visible));
    DWORD old=0;CHECK(VirtualProtect(memory,0x4000,PAGE_READONLY,&old));
    CHECK(adapter.probe()==NativeChatState::Unavailable&&!adapter.submit("read only"));
    CHECK(VirtualProtect(memory,0x4000,PAGE_READWRITE,&old));
    CHECK(!adapter.submit(std::string(128,'x')));
    CHECK(VirtualFree(memory,0,MEM_RELEASE));
    CHECK(adapter.probe()==NativeChatState::Unavailable);
    std::puts(failures?"Native chat tests FAILED":"Opt-in native queue, no retries, UTF-8 bounds, drafts, focus, vtable and page guards passed");
    return failures?1:0;
}
