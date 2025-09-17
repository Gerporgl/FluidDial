#include "Menu.h"
#include "PieMenu.h"
#ifdef USE_WMB_FSS
#    include "FileMenu.h"
#endif
#include "System.h"

void noop(void* arg) {}

const int buttonRadius = 30;

static const char* menu_help_text[] = { "FluidDial",
                                        "Touch icon for scene",
                                        "Touch center for help",
                                        "Flick left to go back",
                                        "Authors: @bdring,@Mitch",
                                        "Bradley,@bDuthieDev,",
                                        "@Design8Studio",
                                        NULL };

// PieMenu axisMenu("Axes", buttonRadius);

class LB : public RoundButton {
public:
    LB(const char* text, callback_t callback, color_t base_color) :
        RoundButton(text, callback, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
    LB(const char* text, Scene* scene, color_t base_color) : RoundButton(text, scene, buttonRadius, base_color, GREEN, BLUE, WHITE) {}
};

constexpr int LIGHTYELLOW = 0xFFF0;
class IB : public ImageButton {
public:
    IB(const char* text, callback_t callback, const char* filename) : ImageButton(text, callback, filename, buttonRadius, WHITE) {}
    IB(const char* text, Scene* scene, const char* filename) : ImageButton(text, scene, filename, buttonRadius, WHITE) {}
};

#ifndef ALTERNATE_MF_SCENE
extern Scene homingScene;
extern Scene joggingScene;
extern Scene joggingScene2;
extern Scene multiJogScene;
extern Scene probingScene;
#else
extern Scene multiFunctionScene;
#endif
extern Scene toolchangeScene;
extern Scene statusScene;
extern Scene macroMenu;

#ifdef USE_WMB_FSS
extern Scene wmbFileSelectScene;
#else
extern Scene fileSelectScene;
#endif

#ifndef ALTERNATE_MF_SCENE
Scene& jogScene = multiJogScene;
#else
Scene& mfScene = multiFunctionScene;
#endif

extern Scene controlScene;
extern Scene aboutScene;

IB statusButton("Status", &statusScene, "statustp.png");
#ifndef ALTERNATE_MF_SCENE
IB homingButton("Homing", &homingScene, "hometp.png");
IB jogButton("Jog", &jogScene, "jogtp.png");
IB probeButton("Probe", &probingScene, "probetp.png");
#else
IB multiFunctionButton("MPG", &multiFunctionScene, "jogtp.png");
#endif
IB toolchangeButton("Tools", &toolchangeScene, "toolchangetp.png");

#ifdef USE_WMB_FSS
IB filesButton("Files", &wmbFileSelectScene, "filestp.png");
#else
IB filesButton("Files", &fileSelectScene, "filestp.png");
#endif

IB controlButton("Macros", &macroMenu, "macrostp.png");
IB setupButton("About", &aboutScene, "abouttp.png");

class MenuScene : public PieMenu {
public:
    MenuScene() : PieMenu("Main", buttonRadius, menu_help_text) {}
    void disableIcons() {
        statusButton.disable();
#ifndef ALTERNATE_MF_SCENE
        homingButton.disable();
        jogButton.disable();
        probeButton.disable();
#else
        multiFunctionButton.disable();
#endif
        toolchangeButton.disable();
        filesButton.disable();
        controlButton.disable();
        setupButton.enable();
    }
    void enableIcons() {
        statusButton.enable();
#ifndef ALTERNATE_MF_SCENE
        homingButton.enable();
        jogButton.enable();
        probeButton.enable();
#else
        multiFunctionButton.enable();
#endif
        toolchangeButton.enable();
        filesButton.enable();
        controlButton.enable();
        setupButton.enable();
    }
    void onEntry(void* arg) {
        PieMenu::onEntry(arg);
        if (state == Disconnected) {
            disableIcons();
        } else {
            enableIcons();
        }
        // This would be nice as it goes more immediately to this scene and never show the pie menu, 
        // but this somehow prevent to exit that scene (with swipe left) to go back to the pie menu. We need to find a better way to do this.
        //push_scene(&multiFunctionScene);
    }

    void onStateChange(state_t old_state) override {
        static bool never_initialized=true;
        if (state != Disconnected) {
            enableIcons();
        }
        if(never_initialized){
            never_initialized=false;
#ifdef ALTERNATE_MF_SCENE
#ifdef AUTO_MF_SCENE
                push_scene(&multiFunctionScene);
                return;
#endif
#endif
#ifdef AUTO_JOG_SCENE
                if (state == Idle) {
                    push_scene(&multiFunctionScene);
                    return;
                }
#endif
#ifdef AUTO_HOMING_SCENE
                if (state == Alarm && lastAlarm == 14) {  // Unknown or Unhomed
                    push_scene(&homingScene, (void*)"auto");
                    return;
                }
#endif
        }
        //
        reDisplay();
    }
} menuScene;

Scene* initMenus() {
#ifndef ALTERNATE_MF_SCENE
    menuScene.addItem(&statusButton);
    menuScene.addItem(&homingButton);
    menuScene.addItem(&jogButton);
    menuScene.addItem(&probeButton);
#else
    menuScene.addItem(&multiFunctionButton);
    menuScene.addItem(&statusButton);
#endif
    menuScene.addItem(&toolchangeButton);
    menuScene.addItem(&filesButton);
    menuScene.addItem(&controlButton);
    menuScene.addItem(&setupButton);

    return &menuScene;
}
