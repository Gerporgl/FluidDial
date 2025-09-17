// Copyright (c) 2023 Mitch Bradley
// Use of this source code is governed by a GPLv3 license that can be found in the LICENSE file.

#include "Config.h"

#include "Scene.h"
#include "ConfirmScene.h"
#include "e4math.h"

extern Scene helpScene;
extern Scene fileSelectScene;

static const char* multi_help_text[] = { "Help",
                                       "Touch:",
                                       "Hold axis to zero it",
                                       "Turn: jog by digit",
                                       "Red/Grn: hold to jog",
                                       "Swipe left to exit",
                                       NULL };

class MultiFunctionScene : public Scene {
private:
    int          _dist_index[3] = { 2, 2, 2 };
    int          max_index() { return 6; }  // 10^3 = 1000;
    int          min_index() { return 0; }  // 10^3 = 1000;
    int          _selected_mask = 1 << 0;
    const int    num_axes       = 3;
    bool         _cancelling    = false;
    bool         _cancel_held   = false;
    bool         _continuous    = false;
    LGFX_Sprite* _bg_image      = nullptr;
    LGFX_Sprite* _img_home      = nullptr;
    LGFX_Sprite* _img_homing    = nullptr;

public:
    MultiFunctionScene() : Scene("MPG", 4, multi_help_text) {}

    e4_t distance(int axis) { return e4_power10(_dist_index[axis] - num_digits()); }
    void unselect_all() { _selected_mask = 0; }
    bool selected(int axis) { return _selected_mask & (1 << axis); }
    bool only(int axis) { return _selected_mask == (1 << axis); }

    int  next(int axis) { return (axis < 2) ? axis + 1 : 0; }
    void select(int axis) { _selected_mask |= 1 << axis; }
    void unselect(int axis) { _selected_mask &= ~(1 << axis); }

    int the_selected_axis() {
        if ((_selected_mask & (_selected_mask - 1)) != 0) {
            return -2;  // Multiple axes are selected
        }
        for (size_t axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                return axis;
            }
        }
        return -1;  // No axis is selected
    }

    void reDisplay() {
        background();
        drawBackground(_bg_image,0, 45);
        drawMenuTitle(current_scene->name());
        drawStatus();

        if (state != Jog && _cancelling) {
            _cancelling = false;
        }
        DRO dro(0, 0, 0, 0);
        for (size_t axis = 0; axis < num_axes; axis++) {
            dro.draw(40 + axis*80, 45+19, 72 + axis*80, 45+41, axis, 4, selected(axis));
        }
        text("X100", 40, 45+64*1+33, _dist_index[0]==2 ? GREEN : WHITE, SMALL, middle_center);
        text("X10", 40+80, 45+64*1+33, _dist_index[0]==1 ? GREEN : WHITE, SMALL, middle_center);
        text("X1", 40+160, 45+64*1+33, _dist_index[0]==0 ? GREEN : WHITE, SMALL, middle_center);

        //text("HOME", 40, 45+64*2+33, state==Homing ? RED : WHITE, TINY, middle_center);


        if(state!=Homing)
        {
            if(!_img_home)
            {
                _img_home = new LGFX_Sprite(&canvas);
                _img_home->setColorDepth(canvas.getColorDepth());
                _img_home->createSprite(38,34);
                drawPngFile(_img_home, "home.png", 0,0);
            }
            _img_home->pushSprite(40-19, 45+64*2+33-17, 0);
        }
        else
        {
            if(!_img_homing)
            {
                _img_homing = new LGFX_Sprite(&canvas);
                _img_homing->setColorDepth(canvas.getColorDepth());
                _img_homing->createSprite(38,34);
                drawPngFile(_img_homing, "homing.png", 0,0);
            }
            _img_homing->pushSprite(40-19, 45+64*2+33-17, 0);
        }
        if (_cancelling || _cancel_held) {
            centered_text("Jog Canceled", 310, RED, TINY);
        } else {

            if (state == Jog) {
                if (!_continuous) {
                    centered_text("Touch or Lock to cancel jog", 310, YELLOW, TINY);
                }
            } else {
                drawButtonLegends("Jog-", "Jog+", "Back");
            }
        }
        refreshDisplay();
    }
    void zero_axes() {
        std::string cmd = "G10L20P0";
        for (int axis = 0; axis < num_axes; axis++) {
            if (selected(axis)) {
                cmd += axisNumToChar(axis);
                cmd += "0";
            }
        }
        send_line(cmd.c_str());
    }
    void zero_axis(char axis) {
        std::string cmd = "G10L20P0";
        cmd+=axis;
        cmd+="0";
        send_line(cmd.c_str());
    }
    void onEntry(void* arg) {
        // if (arg && strcmp((const char*)arg, "Confirmed") == 0) {
        //     zero_axes();
        // }
        if(!_bg_image)
        {
            _bg_image = new LGFX_Sprite(&canvas);
            _bg_image->setColorDepth(canvas.getColorDepth());
            _bg_image->createSprite(240,256);
            drawCommandButtons(_bg_image);
        }

        if (initPrefs()) {

            for (size_t axis = 0; axis < 3; axis++) {
                getPref("DistanceDigit", axis, &_dist_index[axis]);
            }
        }
    }

    void drawCommandButtons(LGFX_Sprite* sprite){
        int i=0;
        Point where;

        // The round boxes could be included with a better design in the png itself, for now they are draw dynamically for simplicty
        for(int y=0;y<256;y+=64){
            for(int x=0;x<240;x+=80){
                sprite->fillRoundRect(x+1, y+1, 78, 62, 12, sprite->color888(80,80,80));
                sprite->fillRoundRect(x+3, y+3, 74, 58, 12, DARKGREY);
                where.x=x+40;
                where.y=y+24;
                where = where.from_display();
                switch (i) {
                    case 7:
                        //sprite_text(sprite, "Probe", x+40,y+25,WHITE,TINY, middle_center);
                        //sprite_text(sprite, "Left", x+40,y+41,WHITE,TINY, middle_center);
                        drawPngFile(_bg_image, "probe_left.png", where.x, where.y);
                        break;
                    case 8:
                        //sprite_text(sprite, "Probe", x+40,y+25,WHITE,TINY, middle_center);
                        //sprite_text(sprite, "Right", x+40,y+41,WHITE,TINY, middle_center);
                        drawPngFile(_bg_image, "probe_right.png", where.x, where.y);
                        break;
                    case 9:
                        //sprite_text(sprite, "Probe", x+40,y+25,WHITE,TINY, middle_center);
                        //sprite_text(sprite, "Z", x+40,y+41,WHITE,TINY, middle_center);
                        drawPngFile(_bg_image, "probe_z.png", where.x, where.y);
                        break;
                    case 10:
                        //sprite_text(sprite, "Probe", x+40,y+25,WHITE,TINY, middle_center);
                        //sprite_text(sprite, "Rear", x+40,y+41,WHITE,TINY, middle_center);
                        drawPngFile(_bg_image, "probe_rear.png", where.x, where.y);
                        break;
                    case 11:
                        //sprite_text(sprite, "Probe", x+40,y+25,WHITE,TINY, middle_center);
                        //sprite_text(sprite, "Front", x+40,y+41,WHITE,TINY, middle_center);
                        drawPngFile(_bg_image, "probe_front.png", where.x, where.y);
                        break;
                }
                i++;
            }
        }    

    }

    void set_dist_index(int axis, int value) {
        _dist_index[axis] = value;
        setPref("DistanceDigit", axis, value);
    }

    void cancel_jog() {
        if (state == Jog) {
            fnc_realtime(JogCancel);
            _continuous = false;
            _cancelling = true;
        }
    }

    void onTouchPress() {
        if (state == Jog) {
            _cancel_held = true;
            cancel_jog();
        }
        reDisplay();
    }

    void onTouchRelease() {
        _cancel_held = false;
        reDisplay();
    }

    int getTouchedButton(){
        if(touchY >= 45 && touchY <= 300)
            return ((touchY-45)/64)*3+(touchX / 80);
        else
            return -1;
            
    }
    void onTouchClick() {
        if (state == Jog || _cancelling || _cancel_held) {
            return;
        }
        int touchedButton=getTouchedButton();
        //dbg_printf("touched button: %d\n",touchedButton);
        switch(touchedButton){
            case 0:
                    unselect_all();
                    select(0);
                    return;
                break;
            case 1:
                    unselect_all();
                    select(1);
                    return;
                break;
            case 2:
                    unselect_all();
                    select(2);
                    return;
                break;
            case 3:
                set_dist_index(0,2);
                set_dist_index(1,2);
                set_dist_index(2,2);
                return;
                break;
            case 4:
                set_dist_index(0,1);
                set_dist_index(1,1);
                set_dist_index(2,1);
                return;
                break;
            case 5:
                set_dist_index(0,0);
                set_dist_index(1,0);
                set_dist_index(2,0);
                return;
                break;
            case 6:
                if (state == Idle || state == Alarm)
                    send_line("$H");
                return;
                break;
            case 7:
                if (state == Idle)
                    send_linef("$Localfs/Run=%s", "macros/probe_left.g");
                return;
                break;
            case 8:
                if (state == Idle)
                    send_linef("$Localfs/Run=%s", "macros/probe_right.g");
                return;
                break;
            case 9:
                if (state == Idle)
                    send_linef("$Localfs/Run=%s", "macros/probe_z.g");
                return;
                break;
            case 10:
                if (state == Idle)
                    send_linef("$Localfs/Run=%s", "macros/probe_rear.g");
                return;
                break;
            case 11:
                if (state == Idle)
                    send_linef("$Localfs/Run=%s", "macros/probe_front.g");
                    return;
                break;
        }
        push_scene(&helpScene, (void*)_help_text);
    }

    void onTouchHold() {
       int tb=getTouchedButton();
        if(tb==0)
            zero_axis('X');
        else if(tb==1)
            zero_axis('Y');
        else if(tb==2)
            zero_axis('Z');
    }

    void onRightFlick() override {
        activate_scene(&fileSelectScene);
    }

    void onDialButtonPress() {
        pop_scene();
    }

        void start_mpg_jog(int delta) {

        // e.g. $J=G91F1000X-10000
        //std::string cmd(inInches ? "$J=G91F400" : "$J=G91F");
        // Not sure if inches needs anything different... only testing in mm
        // I whish there was a jog with acceleration override...
        std::string cmd("$J=G91F");
        for (int axis = 0; axis < num_axes; ++axis) {
            if (selected(axis)) {
                int speed;
                int range=distance(axis);
                // This could have more jog speed ranges, but is no longer needed if
                // fluidNC firmware is running with the jog_acceleration_divisor option
                // set to 10.0 for example.
                if (range<=1000)
                    speed=4000;
                else
                    speed=5000;
                cmd += std::to_string(speed);
                cmd += axisNumToChar(axis);
                cmd += e4_to_cstr(delta * distance(axis), inInches ? 3 : 2);
            }
        }
        send_line(cmd.c_str());
    }

    void onUILocked() {
        cancel_jog();
    }
    void onUIUnlocked() {
    }
    void start_button_jog(bool negative) {
        // e.g. $J=G91F1000X-10000
        e4_t total_distance = 0;
        int  n_axes         = 0;
        for (int axis = 0; axis < num_axes; ++axis) {
            if (selected(axis)) {
                total_distance = e4_magnitude(total_distance, distance(axis));
                ++n_axes;
            }
        }

        e4_t feedrate = total_distance * 300;  // go 5x the highlighted distance in 1 second

        std::string cmd("$J=G91");
        cmd += inInches ? "G20" : "G21";
        cmd += "F";
        cmd += e4_to_cstr(feedrate, 3);
        for (int axis = 0; axis < num_axes; ++axis) {
            if (selected(axis)) {
                e4_t axis_distance;
                if (n_axes == 1) {
                    axis_distance = e4_from_int(inInches ? 200 : 5000);
                } else {
                    axis_distance = distance(axis) * 20;
                }
                if (negative) {
                    axis_distance = -axis_distance;
                }
                cmd += axisNumToChar(axis);
                cmd += e4_to_cstr(axis_distance, 0);
            }
        }
        send_line(cmd.c_str());
        _continuous = true;
    }

    void onGreenButtonPress() {
        if (state == Idle) {
            start_button_jog(false);
        }
    }
    void onGreenButtonRelease() {
        cancel_jog();
    }
    void onRedButtonPress() {
        if (state == Idle) {
            start_button_jog(true);
        }
    }
    void onRedButtonRelease() {
        cancel_jog();
    }

    void onEncoder(int delta) {
        if (delta != 0) {
            start_mpg_jog(delta);
        }
    }

    void onDROChange() {
        reDisplay();
    }
    void onLimitsChange() {
        reDisplay();
    }
    void onAlarm() {
        reDisplay();
    }
    void onExit() {
        cancel_jog();
    }
} multiFunctionScene;
