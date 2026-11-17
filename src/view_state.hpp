#pragma once
#include "meter.hpp"
#include <algorithm>
namespace meter {
// Editing uses hardware deltas, independent of an OS cursor warped by the game.
class RelativeCursor {
    float x_ = 0, y_ = 0, width_ = 1, height_ = 1;
    bool left_ = false;
public:
    void reset(float x, float y, float width, float height) {
        width_ = (std::max)(1.0f, width); height_ = (std::max)(1.0f, height);
        absolute(x, y); left_ = false;
    }
    void absolute(float x, float y) { x_ = std::clamp(x, 0.0f, width_-1); y_ = std::clamp(y, 0.0f, height_-1); }
    void move(float dx, float dy) { absolute(x_+dx, y_+dy); }
    void button(bool left) { left_ = left; }
    float x() const { return x_; }
    float y() const { return y_; }
    bool left() const { return left_; }
};
class Visibility {
    enum class Manual { Auto, Show, Hide };
    Manual manual_ = Manual::Auto;
    bool inQuest_ = false, missionArea_ = false;
    bool automatic_ = true;
    int quest_ = 0;
public:
    void automatic(bool enabled){if(automatic_!=enabled){automatic_=enabled;manual_=Manual::Auto;}}
    void update(const Sample& sample) {
        if (!sample.valid) return;
        const bool live = sample.expedition || (sample.questId > 0 && sample.state == 2);
        const int context=sample.expedition?-1:sample.questId;
        if (automatic_ && live && (!inQuest_ || context != quest_)) manual_ = Manual::Auto;
        inQuest_ = live;
        quest_ = context;
        missionArea_ = sample.expedition || (sample.questId > 0 && sample.state >= 2 && sample.state <= 5);
    }
    bool visible() const { return manual_ == Manual::Show || (automatic_ && manual_ == Manual::Auto && missionArea_); }
    void toggle() { manual_ = visible() ? Manual::Hide : Manual::Show; }
    void hide() { manual_ = Manual::Hide; }
};
struct Rect {
    float x = 0, y = 0, width = 0, height = 0;
    bool contains(float px, float py) const { return px >= x && py >= y && px < x + width && py < y + height; }
};
class Placement {
    float nx_ = 0.02f, ny_ = 0.65f, offsetX_ = 0, offsetY_ = 0;
    bool dragging_ = false;
public:
    void load(float nx, float ny) { nx_ = std::clamp(nx, 0.0f, 1.0f); ny_ = std::clamp(ny, 0.0f, 1.0f); }
    Rect bounds(float sw, float sh, float w, float h) const {
        return {std::clamp(nx_ * sw, 0.0f, (std::max)(0.0f, sw-w)),
            std::clamp(ny_ * sh, 0.0f, (std::max)(0.0f, sh-h)), w, h};
    }
    void begin(float px, float py, const Rect& bounds) { offsetX_ = px-bounds.x; offsetY_ = py-bounds.y; dragging_ = true; }
    void move(float px, float py, float sw, float sh, float w, float h) {
        if (!dragging_ || sw <= 0 || sh <= 0) return;
        nx_ = std::clamp(px-offsetX_, 0.0f, (std::max)(0.0f, sw-w)) / sw;
        ny_ = std::clamp(py-offsetY_, 0.0f, (std::max)(0.0f, sh-h)) / sh;
    }
    bool end() { bool was = dragging_; dragging_ = false; return was; }
    bool dragging() const { return dragging_; }
    float x() const { return nx_; }
    float y() const { return ny_; }
};
}
