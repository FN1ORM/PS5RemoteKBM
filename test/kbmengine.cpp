// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL
#include "kbmengine.h"
#include "remoteinputgate.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
static int checks=0;
void check(bool value,const char *name) { ++checks; if(!value) { std::fprintf(stderr,"FAIL: %s\n",name); std::exit(1); } }

static void check_remote_input_gate()
{
    RemoteInputGate gate;
    ChiakiControllerState state;
    chiaki_controller_state_set_idle(&state);
    state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
    check(!gate.Suppresses(state), "a new stream accepts input");

    gate.SetViewOnly(true);
    check(gate.IsViewOnly() && gate.Suppresses(state), "watch mode blocks a held button");
    chiaki_controller_state_set_idle(&state);
    check(gate.Suppresses(state), "watch mode stays blocked after release");

    gate.SetViewOnly(false);
    state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
    check(gate.Suppresses(state), "resume waits for held button");
    state.buttons = 0;
    state.l2_state = 255;
    check(gate.Suppresses(state), "resume waits for held trigger");
    state.l2_state = 0;
    state.left_x = -32768;
    check(gate.Suppresses(state), "resume waits for held stick");
    state.left_x = 0;
    auto touch = chiaki_controller_state_start_touch(&state, 100, 100);
    check(gate.Suppresses(state), "resume waits for touch release");
    chiaki_controller_state_stop_touch(&state, touch);
    state.gyro_x = 0.1f;
    state.left_x = 80;
    check(gate.Suppresses(state), "resume sends a neutral re-arm frame");
    state.buttons = CHIAKI_CONTROLLER_BUTTON_CROSS;
    check(!gate.Suppresses(state), "fresh input works after release despite controller drift");
}

int main() {
    check_remote_input_gate();
    using namespace kbm;
    Engine e;
    e.key(1,Up,true); e.key(2,Right,true);
    auto r=e.tick(0); check(r.lx>23000 && r.ly< -23000,"W+D diagonal");
    check(std::hypot(r.lx,r.ly)<=32768,"diagonal circular bound");
    e.key(3,Down,true); r=e.tick(.001); check(r.ly==0 && r.lx==32767,"opposites cancel");
    e.key(4,Walk,true); r=e.tick(.002); check(r.lx>14740 && r.lx<14750,"partial walk");
    e.reset();
    for(int action=0; action<16; ++action) {
        e.key(action,Action(action),true); r=e.tick(.01);
        check((r.buttons&(1u<<action))!=0,"digital mapping");
        e.key(action,Action(action),false);
    }
    e.key(1,L2,true); e.key(2,R2,true); r=e.tick(.02); check(r.l2==255 && r.r2==255,"triggers");
    e.key(3,L2,true); e.key(1,L2,false); r=e.tick(.03); check(r.l2==255,"shared binding hold");
    e.key(3,L2,true); e.key(3,L2,false); r=e.tick(.04); check(r.l2==0,"repeat idempotent");
    e.reset(); e.mouse(32,0,1); r=e.tick(1); check(r.rx==32767 && r.ry==0,"mouse full tilt");
    r=e.tick(1.1); check(r.rx==0 && r.ry==0,"finite recenter");
    Engine heldStick;
    heldStick.key(1,RightStickUp,true);
    check(heldStick.tick(1).ry==-32767,"right stick key tilts fully");
    heldStick.mouse(100,100,1.5);
    r=heldStick.tick(2);
    check(r.rx==0 && r.ry==-32767,"held direction outlasts recenter and overrides mouse");
    heldStick.key(2,RightStickRight,true);
    r=heldStick.tick(2.01);
    check(r.rx>23000 && r.ry< -23000 && std::hypot(r.rx,r.ry)<=32768,"right stick diagonal");
    heldStick.key(3,RightStickDown,true);
    r=heldStick.tick(2.02);
    check(r.rx==32767 && r.ry==0,"opposite right stick directions cancel");
    heldStick.key(1,RightStickUp,false);
    heldStick.key(2,RightStickRight,false);
    heldStick.key(3,RightStickDown,false);
    r=heldStick.tick(2.03);
    check(!r.rx && !r.ry,"key release centers without stale mouse motion");
    heldStick.mouse(-100,0,2.04);
    check(heldStick.tick(2.04).rx<0,"mouse resumes after key release");
    e.reset(); e.config.invertY=true; e.mouse(0,16,2); r=e.tick(2); check(r.ry<0,"invert Y");
    e.reset(); e.config.maximum=.7; e.mouse(2000,2000,3); r=e.tick(3);
    check(std::hypot(r.rx,r.ry)<=32767*.7+1,"radial maximum");
    e.reset(); e.config.deadzone=.2; e.mouse(1,0,4); r=e.tick(4); check(r.rx==0,"deadzone");
    e.reset(); e.config.antiDeadzone=.3; e.mouse(8,0,5); r=e.tick(5); check(r.rx>=9830,"anti deadzone");
    e.reset(); e.config.deadzone=0; e.config.antiDeadzone=0; e.config.maximum=1;
    e.config.smoothingMs=0; e.config.recenterMs=0; e.mouse(1,0,6); r=e.tick(6);
    check(r.rx>0,"zero smoothing input"); r=e.tick(6.009); check(r.rx==0,"zero recenter immediate");
    e.mouse(100,0,7); e.key(1,Cross,true); e.reset(); r=e.tick(7);
    check(!r.buttons && !r.lx && !r.ly && !r.rx && !r.ry && !r.l2 && !r.r2,"focus loss reset");
    e.config.maximum=std::numeric_limits<double>::quiet_NaN(); e.config.exponent=-10;
    e.mouse(100,100,8); r=e.tick(8); check(std::hypot(r.rx,r.ry)<32769,"invalid settings clamped");
    e.mouse(20001,0,9); r=e.tick(9); check(r.rx==0,"absurd deltas ignored");
    // Same physical count per second, different polling cadence, stable output.
    Engine a,b; a.config.smoothingMs=b.config.smoothingMs=40;
    for(int n=0;n<40;++n) { a.mouse(1,0,10+n*.001); if(n%4==0) b.mouse(4,0,10+n*.001); }
    auto ra=a.tick(10.039),rb=b.tick(10.039);
    check(std::abs(ra.rx-rb.rx)<=1,"polling cadence independence");
    // Rolling windows shed old motion as time advances, including reversals.
    Engine precise; precise.config.rollingWindow=true; precise.config.smoothingMs=8; precise.config.recenterMs=0;
    precise.mouse(8,0,11); precise.mouse(-4,0,11.006);
    check(precise.tick(11.006).rx>0,"rolling initial net motion");
    check(precise.tick(11.009).rx<0,"rolling reversal discards stale direction");
    check(precise.tick(11.015).rx==0,"precise stops within one window");
    precise.mouse(1,0,12);check(precise.tick(12).rx>0,"precise micro movement");
    precise.reset();check(precise.tick(12.001).rx==0,"precise focus release neutral");
    // Sweep valid reports and ensure sticks stay inside the circular domain.
    for(int n=0;n<1000;++n) {
        e.mouse(std::sin(n)*100,std::cos(n)*100,20+n*.01); r=e.tick(20+n*.01);
        check(std::hypot(r.rx,r.ry)<=32768,"sweep circular bound");
    }
    std::printf("PASS: %d input checks\n",checks);
}
