#pragma once

// Just the SDL scan codes
#include <SDL_scancode.h>

// Controller commands
#define CMD_READ_CONFIGURATION_BYTE     0x20
#define CMD_WRITE_CONFIGURATION_BYTE    0x60

#define CMD_DISABLE_AUX_INTERFACE       0xA7
#define CMD_ENABLE_AUX_INTERFACE        0xA8
#define CMD_TEST_AUX_INTERFACE          0xA9

#define CMD_TEST_PS2_CONTROLLER         0xAA
#define CMD_TEST_FIRST_PS2_PORT         0xAB
#define CMD_DISABLE_FIRST_PS2_PORT      0xAD
#define CMD_ENABLE_FIRST_PS2_PORT       0xAE

#define CMD_READ_FIRST_PORT             0xC0
#define CMD_SEND_CMD_AUX_DEVICE         0xD4

// Device commands
#define CMD_BAT_PASSED                  0xAA
#define CMD_DEV_ENABLE                  0xF4
#define CMD_DEV_DISABLE                 0xF5
#define CMD_DEV_ACK                     0xFA
#define CMD_DEV_RESET                   0xFF

namespace Cepums {

    enum class KeyboardControllerState
    {
        OK,
        WaitingForCommandByte,
        SelfTest,
        KeyboardTest,
        ReadFirstPort,
        EnableKeyboard,
        DisableKeyboard
    };

    class KeyboardController
    {
    public:
        void writeCommandRegister(uint8_t value);
        void writeDataPort(uint8_t value);

        uint8_t readDataPort();
        uint8_t readStatusRegister();
        void keyPressed(SDL_Scancode scancode);
        void keyReleased(SDL_Scancode scancode);
    private:
        uint8_t getByteFromSimpleScancode(SDL_Scancode scancode);
    private:
        KeyboardControllerState m_state = KeyboardControllerState::OK;
        uint8_t m_commandByte = 0;
        bool m_keyboardEnabled = false;
        std::vector<uint8_t> m_dataBuffer;
        bool m_dataToMouse = false;
    };
}
