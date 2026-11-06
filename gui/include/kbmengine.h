// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
// Mouse velocity/window and finite recenter design informed by keyboard-ds5
// gameinput.py (tejasbnshetty, AGPL-3.0-only); native implementation for this fork.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <deque>
#include <map>
#include <set>

namespace kbm {
enum Action { Cross, Circle, Square, Triangle, DLeft, DRight, DUp, DDown,
    L1, R1, L3, R3, Options, Share, Touchpad, PS, L2, R2,
    Up, Down, Left, Right, Walk, Count };
struct Config {
    double sensitivityX=1, sensitivityY=1, exponent=1, deadzone=0,
        antiDeadzone=0, smoothingMs=16, recenterMs=35, maximum=1, walkTilt=.45;
    bool invertY=false, rollingWindow=false;
    void sanitize() {
        auto limit=[](double v,double lo,double hi,double fallback) {
            return std::isfinite(v) ? std::max(lo,std::min(hi,v)) : fallback;
        };
        sensitivityX=limit(sensitivityX,.01,20,1); sensitivityY=limit(sensitivityY,.01,20,1);
        exponent=limit(exponent,.2,4,1); maximum=limit(maximum,.05,1,1);
        deadzone=limit(deadzone,0,.95,0); antiDeadzone=limit(antiDeadzone,0,maximum,0);
        smoothingMs=limit(smoothingMs,0,200,16); recenterMs=limit(recenterMs,0,500,35);
        walkTilt=limit(walkTilt,.05,1,.45);
    }
};
struct Report {
    uint32_t buttons=0; uint8_t l2=0,r2=0;
    int16_t lx=0,ly=0,rx=0,ry=0;
};
class Engine {
    struct Move { double time,dx,dy; };
    std::deque<Move> moves;
    // Track physical inputs separately: repeated down events are idempotent and
    // releasing one of two keys bound to the same action cannot release the other.
    std::map<int,Action> held;
    double lastMovement=-1, previous=-1, lastX=0,lastY=0, releaseX=0,releaseY=0;
public:
    Config config;
    void key(int physical, Action action, bool down) {
        if(down && action>=0 && action<Count) held.emplace(physical,action);
        else if(!down) held.erase(physical);
    }
    void reset() { held.clear(); moves.clear(); lastMovement=previous=-1; lastX=lastY=releaseX=releaseY=0; }
    void mouse(double dx,double dy,double now) {
        if(!std::isfinite(now) || !std::isfinite(dx) || !std::isfinite(dy)
            || std::abs(dx)>20000 || std::abs(dy)>20000 || !(dx || dy)) return;
        if(now<lastMovement) return;
        moves.push_back({now,dx,dy}); lastMovement=now;
        // Bound memory even if a device or test supplies arbitrarily many reports.
        while(moves.size()>8192) moves.pop_front();
    }
    Report tick(double now) {
        config.sanitize(); Report out;
        if(!std::isfinite(now)) return out;
        if(previous>=0 && now<previous) { reset(); return out; }
        previous=now;
        std::set<Action> actions;
        for(auto &item:held) actions.insert(item.second);
        for(auto a:actions) if(a<16) out.buttons |= uint32_t(1)<<a;
        out.l2=actions.count(L2)?255:0; out.r2=actions.count(R2)?255:0;
        double x=double(actions.count(Right))-double(actions.count(Left));
        double y=double(actions.count(Down))-double(actions.count(Up));
        double length=std::hypot(x,y), scale=actions.count(Walk)?config.walkTilt:1;
        if(length) { out.lx=axis(x/length*scale); out.ly=axis(y/length*scale); }
        // 8 ms minimum window at 125 Hz; zero smoothing means one input tick.
        const double window=std::max(.008,config.smoothingMs/1000.0);
        if(lastMovement>=0 && now-lastMovement<=window) {
            while(!moves.empty() && moves.front().time<(config.rollingWindow ? now : lastMovement)-window) moves.pop_front();
            x=y=0;
            for(const auto &m:moves) { x+=m.dx; y+=m.dy; }
            x*=config.sensitivityX/(2000*window);
            y*=config.sensitivityY/(2000*window)*(config.invertY?-1:1);
            length=std::hypot(x,y);
            double magnitude=std::min(1.0,length);
            if(magnitude<=config.deadzone || length<1e-9) lastX=lastY=0;
            else {
                magnitude=(magnitude-config.deadzone)/(1-config.deadzone);
                magnitude=config.antiDeadzone+(config.maximum-config.antiDeadzone)*std::pow(magnitude,config.exponent);
                lastX=x/length*magnitude; lastY=y/length*magnitude;
            }
            releaseX=lastX; releaseY=lastY;
        } else {
            const double elapsed=lastMovement<0 ? 1 : std::max(0.0,now-lastMovement-window);
            const double factor=config.recenterMs<=0 ? 0 : std::max(0.0,1-elapsed/(config.recenterMs/1000));
            lastX=releaseX*factor; lastY=releaseY*factor;
            moves.clear();
        }
        out.rx=axis(lastX); out.ry=axis(lastY);
        return out;
    }
    static int16_t axis(double value) { return int16_t(std::round(std::max(-1.0,std::min(1.0,value))*32767)); }
};
}
