#pragma once
#include "config_parser.h"
#include "input.h"
#include <raylib.h>
#include <unordered_map>

namespace FrameZero {

// A Dynamic Input Mapper that bridges physical hardware (Keyboard/Gamepad)
// to our deterministic Rollback Input bitmask.
// Reads user keybinds directly from the ConfigParser!
class InputMapper {
private:
    // Maps a logical button (e.g. BTN_PUNCH) to a hardware Raylib key (e.g. KEY_J)
    std::unordered_map<uint32_t, int> keyboardMap;
    std::unordered_map<uint32_t, int> gamepadMap;
    int gamepadId;

public:
    InputMapper(int gamepadIndex = 0) : gamepadId(gamepadIndex) {
        // Default Keybinds
        keyboardMap[BTN_UP] = KEY_W;
        keyboardMap[BTN_DOWN] = KEY_S;
        keyboardMap[BTN_LEFT] = KEY_A;
        keyboardMap[BTN_RIGHT] = KEY_D;
        keyboardMap[BTN_PUNCH] = KEY_J;
        keyboardMap[BTN_KICK] = KEY_K;

        // Default Gamepad binds (XInput)
        gamepadMap[BTN_UP] = GAMEPAD_BUTTON_LEFT_FACE_UP;
        gamepadMap[BTN_DOWN] = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
        gamepadMap[BTN_LEFT] = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
        gamepadMap[BTN_RIGHT] = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
        gamepadMap[BTN_PUNCH] = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;  // X / Square
        gamepadMap[BTN_KICK] = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;   // A / Cross
    }

    // Load custom keybinds from the user's config.ini
    void loadFromConfig(ConfigParser& config, const std::string& playerSection) {
        keyboardMap[BTN_UP] = config.getInt(playerSection, "KeyUp", KEY_W);
        keyboardMap[BTN_DOWN] = config.getInt(playerSection, "KeyDown", KEY_S);
        keyboardMap[BTN_LEFT] = config.getInt(playerSection, "KeyLeft", KEY_A);
        keyboardMap[BTN_RIGHT] = config.getInt(playerSection, "KeyRight", KEY_D);
        keyboardMap[BTN_PUNCH] = config.getInt(playerSection, "KeyPunch", KEY_J);
        keyboardMap[BTN_KICK] = config.getInt(playerSection, "KeyKick", KEY_K);
    }

    // Save current keybinds back to config
    void saveToConfig(ConfigParser& config, const std::string& playerSection) {
        config.setInt(playerSection, "KeyUp", keyboardMap[BTN_UP]);
        config.setInt(playerSection, "KeyDown", keyboardMap[BTN_DOWN]);
        config.setInt(playerSection, "KeyLeft", keyboardMap[BTN_LEFT]);
        config.setInt(playerSection, "KeyRight", keyboardMap[BTN_RIGHT]);
        config.setInt(playerSection, "KeyPunch", keyboardMap[BTN_PUNCH]);
        config.setInt(playerSection, "KeyKick", keyboardMap[BTN_KICK]);
    }

    // Bind a specific button on the fly (e.g. in the Options menu)
    void rebindKeyboard(uint32_t logicalButton, int raylibKey) {
        keyboardMap[logicalButton] = raylibKey;
    }

    // Polls hardware and returns a perfectly packed deterministic Input struct for the current frame
    Input poll() const {
        Input inp;
        inp.buttons = 0;

        // Check Keyboard
        for (const auto& pair : keyboardMap) {
            if (IsKeyDown(pair.second)) {
                inp.buttons |= pair.first;
            }
        }

        // Check Gamepad
        if (IsGamepadAvailable(gamepadId)) {
            for (const auto& pair : gamepadMap) {
                if (IsGamepadButtonDown(gamepadId, pair.second)) {
                    inp.buttons |= pair.first;
                }
            }
        } // <--- Added closing brace here
            
        // Convert buttons back to moveX / moveY for the physics code
        if (inp.buttons & BTN_LEFT) inp.moveX = -127;
        if (inp.buttons & BTN_RIGHT) inp.moveX = 127;
        if (inp.buttons & BTN_UP) inp.moveY = 127;
        if (inp.buttons & BTN_DOWN) inp.moveY = -127;
        
        // Analog stick support overrides digital
        if (IsGamepadAvailable(gamepadId)) {
            float axisX = GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_LEFT_X);
            float axisY = GetGamepadAxisMovement(gamepadId, GAMEPAD_AXIS_LEFT_Y);
            
            if (axisX < -0.2f || axisX > 0.2f) {
                inp.moveX = static_cast<int8_t>(axisX * 127.0f);
            }
            if (axisY < -0.2f || axisY > 0.2f) {
                // Raylib gamepad Y is inverted (down is positive), so flip it for standard math
                inp.moveY = static_cast<int8_t>(-axisY * 127.0f);
            }
        }

        return inp;
    }
};

} // namespace FrameZero
