#include "../src/meter.hpp"
#include "../src/view_state.hpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace meter;
static int failures = 0;
#define CHECK(test) do { if (!(test)) { std::printf("FAIL line %d: %s\n", __LINE__, #test); ++failures; } } while (0)
Sample sample(int state = 2, int id = 42) {
    Sample s;
    s.valid = s.countersValid = true;
    s.state = state; s.questId = id;
    s.players[0] = {1, L"Enzo", 100, true, true};
    s.players[1] = {2, L"Amigo", 300, false, true};
    return s;
}
int main() {
    {
        Meter m;
        m.update(sample(1), 0);
        CHECK(m.phase() == Phase::Waiting);
        m.update(sample(), 250);
        CHECK(m.total() == 400);
        CHECK(!m.partial());
        CHECK(m.percent(m.ranked()[0]) == 75);
        m.update(sample(), 500);
        CHECK(m.total() == 400); // duplicate poll must not add damage
        auto s = sample(); s.players[0].damage = 0; s.players[1].damage = 0;
        m.update(s, 750);
        CHECK(m.total() == 400); // game clearing counters cannot erase the hunt
        m.update(Sample{}, 1000);
        CHECK(m.phase() == Phase::Live); // loading screen is not quest end
        s = sample(); s.players[1] = {}; s.players[0].damage = 200;
        m.update(s, 1250);
        CHECK(m.total() == 500); // departed player retained in denominator
        CHECK(!m.ranked()[0].present);
        s = sample(3); s.players[0].damage = 250;
        m.update(s, 1500);
        CHECK(m.phase() == Phase::Settling);
        CHECK(m.total() == 550); // accept final damage
        s.players[0].damage = 260;
        m.update(s, 2500);
        CHECK(m.total() == 560); // late synchronized update
        m.update(sample(0, 0), 4250);
        CHECK(m.phase() == Phase::Summary);
        CHECK(m.total() == 560);
        m.update(sample(0, 0), 6000);
        CHECK(m.total() == 560); // summary retained in town
        m.update(sample(), 6250);
        CHECK(m.total() == 400); // repeating the same quest starts fresh
    }
    {
        Meter m;
        m.update(sample(), 10);
        CHECK(m.partial()); // attachment in an already running quest
        auto s = sample(); s.players[1] = {3, L"Nuevo", 50, false, true};
        m.update(s, 250);
        CHECK(m.total() == 450); // replacement in the same slot is a new player
        m.update(sample(2, 43), 500);
        CHECK(m.total() == 400); // a new quest id also starts fresh
        s = sample(2, 43); s.countersValid = false;
        m.update(s, 750);
        CHECK(m.total() == 400 && !m.dataAvailable());
    }
    {
        Meter m;
        auto s = sample(); s.players[0].damage = s.players[1].damage = 0;
        m.update(s, 1);
        CHECK(m.total() == 0 && m.percent(m.ranked()[0]) == 0);
        m.update(sample(5), 250);
        m.update(sample(5), 3000);
        CHECK(m.phase() == Phase::Summary); // failed hunts also finish
    }
    {
        Meter m;
        auto s = sample(); s.players[0].weapon = 0;
        m.update(s, 1);
        s.players[0].weapon = 3; s.players[0].damage = 120;
        m.update(s, 250);
        CHECK(m.ranked()[1].weapon == 3 && m.total() == 420);
        s.players[0].weapon = UnknownWeapon;
        m.update(s, 500);
        CHECK(m.ranked()[1].weapon == 3); // unavailable read preserves last known weapon
        s.players[0] = {};
        m.update(s, 750);
        CHECK(m.ranked()[1].weapon == 3 && !m.ranked()[1].present);
        CHECK(!validWeapon(255) && !validWeapon(-1));
        for (int i = 0; i < 14; ++i) {
            CHECK(validWeapon(i));
            for (int j = i+1; j < 14; ++j) CHECK(weaponStyle(i).color != weaponStyle(j).color);
        }
    }
    {
        Visibility v;
        CHECK(!v.visible());
        v.update(sample(0, 0)); CHECK(!v.visible());
        v.toggle(); CHECK(v.visible()); // manual in town
        v.hide(); CHECK(!v.visible());
        v.update(sample()); CHECK(v.visible()); // auto start overrides previous manual hide
        v.toggle(); CHECK(!v.visible()); // manual hide lasts through this mission
        v.update(Sample{}); CHECK(!v.visible());
        v.update(sample()); CHECK(!v.visible());
        v.update(sample(3)); CHECK(!v.visible());
        v.update(sample(0, 0)); CHECK(!v.visible());
        v.update(sample()); CHECK(v.visible());
        v.update(sample(3)); CHECK(v.visible()); // results still on field
        v.update(sample(0, 0)); CHECK(!v.visible()); // hide back in town
        v.toggle(); CHECK(v.visible()); // manually consult last result
        v.update(sample(2, 43)); CHECK(v.visible());
    }
    {
        RelativeCursor cursor;
        cursor.reset(400, 300, 800, 600);
        cursor.move(130, -70);
        CHECK(cursor.x() == 530 && cursor.y() == 230);
        // A game recentering its unrelated OS cursor must not reset these coordinates.
        cursor.move(25, 10);
        CHECK(cursor.x() == 555 && cursor.y() == 240);
        cursor.button(true); CHECK(cursor.left());
        cursor.move(10000, -10000);
        CHECK(cursor.x() == 799 && cursor.y() == 0);
        cursor.button(false); CHECK(!cursor.left());
        cursor.absolute(-20, 900); CHECK(cursor.x() == 0 && cursor.y() == 599);
    }
    {
        Placement p;
        p.load(0.2f, 0.3f);
        auto b = p.bounds(1000, 800, 374, 200);
        CHECK(std::abs(b.x-200) < 0.01f && std::abs(b.y-240) < 0.01f);
        p.begin(210, 250, b);
        p.move(510, 410, 1000, 800, 374, 200);
        b = p.bounds(1000, 800, 374, 200);
        CHECK(b.x == 500 && b.y == 400); // preserve clicked offset
        p.move(5000, -100, 1000, 800, 374, 200);
        b = p.bounds(1000, 800, 374, 200);
        CHECK(b.x == 626 && b.y == 0);
        CHECK(p.end() && !p.dragging());
        Placement restored; restored.load(p.x(), p.y());
        b = restored.bounds(600, 400, 374, 200);
        CHECK(b.x >= 0 && b.x+374 <= 600 && b.y == 0);
    }
    std::printf("%s\n", failures ? "Meter tests FAILED" : "All meter tests passed");
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
