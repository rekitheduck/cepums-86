#include "cepumspch.h"
#include "KeyboardController.h"

namespace Cepums {

    struct SDLtoScancodeSet1
    {
        SDL_Scancode scancode;
        uint8_t byte;
    };

    const SDLtoScancodeSet1 s_SDLtoScancodeSet1[] = {
        { SDL_SCANCODE_ESCAPE,      0x01 },
        { SDL_SCANCODE_1,           0x02 },
        { SDL_SCANCODE_2,           0x03 },
        { SDL_SCANCODE_3,           0x04 },
        { SDL_SCANCODE_4,           0x05 },
        { SDL_SCANCODE_5,           0x06 },
        { SDL_SCANCODE_6,           0x07 },
        { SDL_SCANCODE_7,           0x08 },
        { SDL_SCANCODE_8,           0x09 },
        { SDL_SCANCODE_9,           0x0A },
        { SDL_SCANCODE_0,           0x0B },
        { SDL_SCANCODE_MINUS,       0x0C },
        { SDL_SCANCODE_EQUALS,      0x0D },
        { SDL_SCANCODE_BACKSPACE,   0x0E },
        { SDL_SCANCODE_TAB,         0x0F },

        { SDL_SCANCODE_Q,           0x10 },
        { SDL_SCANCODE_W,           0x11 },
        { SDL_SCANCODE_E,           0x12 },
        { SDL_SCANCODE_R,           0x13 },
        { SDL_SCANCODE_T,           0x14 },
        { SDL_SCANCODE_Y,           0x15 },
        { SDL_SCANCODE_U,           0x16 },
        { SDL_SCANCODE_I,           0x17 },
        { SDL_SCANCODE_O,           0x18 },
        { SDL_SCANCODE_P,           0x19 },
        { SDL_SCANCODE_LEFTBRACKET, 0x1A },
        { SDL_SCANCODE_RIGHTBRACKET,0x1B },
        { SDL_SCANCODE_RETURN,      0x1C },
        { SDL_SCANCODE_LCTRL,       0x1D },
        { SDL_SCANCODE_A,           0x1E },
        { SDL_SCANCODE_S,           0x1F },

        { SDL_SCANCODE_D,           0x20 },
        { SDL_SCANCODE_F,           0x21 },
        { SDL_SCANCODE_G,           0x22 },
        { SDL_SCANCODE_H,           0x23 },
        { SDL_SCANCODE_J,           0x24 },
        { SDL_SCANCODE_K,           0x25 },
        { SDL_SCANCODE_L,           0x26 },
        { SDL_SCANCODE_SEMICOLON,   0x27 },
        { SDL_SCANCODE_APOSTROPHE,  0x28 },
        { SDL_SCANCODE_GRAVE,       0x29 },
        { SDL_SCANCODE_LSHIFT,      0x2A },
        { SDL_SCANCODE_BACKSLASH,   0x2B },
        { SDL_SCANCODE_Z,           0x2C },
        { SDL_SCANCODE_X,           0x2D },
        { SDL_SCANCODE_C,           0x2E },
        { SDL_SCANCODE_V,           0x2F },

        { SDL_SCANCODE_B,           0x30 },
        { SDL_SCANCODE_N,           0x31 },
        { SDL_SCANCODE_M,           0x32 },
        { SDL_SCANCODE_COMMA,       0x33 },
        { SDL_SCANCODE_PERIOD,      0x34 },
        { SDL_SCANCODE_SLASH,       0x35 },
        { SDL_SCANCODE_RSHIFT,      0x36 },
        { SDL_SCANCODE_PRINTSCREEN, 0x37 },
        { SDL_SCANCODE_LALT,        0x38 },
        { SDL_SCANCODE_SPACE,       0x39 },
        { SDL_SCANCODE_CAPSLOCK,    0x3A },
        { SDL_SCANCODE_F1,          0x3B },
        { SDL_SCANCODE_F2,          0x3C },
        { SDL_SCANCODE_F3,          0x3D },
        { SDL_SCANCODE_F4,          0x3E },
        { SDL_SCANCODE_F5,          0x3F },

        { SDL_SCANCODE_F6,          0x40 },
        { SDL_SCANCODE_F7,          0x41 },
        { SDL_SCANCODE_F8,          0x42 },
        { SDL_SCANCODE_F9,          0x43 },
        { SDL_SCANCODE_F10,         0x44 },
        { SDL_SCANCODE_NUMLOCKCLEAR,0x45 }, // NUMLOCKCLEAR?
        { SDL_SCANCODE_SCROLLLOCK,  0x46 },
        { SDL_SCANCODE_HOME,        0x47 },
        { SDL_SCANCODE_UP,          0x48 },
        { SDL_SCANCODE_PAGEUP,      0x49 },
        { SDL_SCANCODE_KP_MINUS,    0x4A },
        { SDL_SCANCODE_LEFT,        0x4B },
        { SDL_SCANCODE_KP_5,        0x4C },
        { SDL_SCANCODE_RIGHT,       0x4D },
        { SDL_SCANCODE_KP_PLUS,     0x4E },
        { SDL_SCANCODE_END,         0x4F },

        { SDL_SCANCODE_DOWN,        0x50 },
        { SDL_SCANCODE_PAGEDOWN,    0x51 },
        { SDL_SCANCODE_INSERT,      0x52 },
        { SDL_SCANCODE_DELETE,      0x53 },

        { SDL_SCANCODE_F11,         0x57 },
        { SDL_SCANCODE_F12,         0x58 }
    };

    /*
    Initialization:
    1. Read status register it's 0 to flush all pending stuff

    2. Do kinda #1 to see if there's nothing in the input buffer
    2. Write out CMD_WRITE_CONFIGURATION_BYTE command
    3. Do kinda #1 to see if there's nothing in the input buffer
    4. Write out the data to the data port

    5. Write out CMD_TEST_PS2_CONTROLLER command;
    */

    void KeyboardController::writeCommandRegister(uint8_t value)
    {
        if (value == CMD_WRITE_CONFIGURATION_BYTE)
        {
            // Okay, we're waiting for the data now
            DC_CORE_TRACE("KBC Command: Write configuration byte");
            m_state = KeyboardControllerState::WaitingForCommandByte;
            m_dataToMouse = false;
            return;
        }

        if (value == CMD_TEST_PS2_CONTROLLER)
        {
            // "Self-test"
            DC_CORE_TRACE("KBC Command: Self-test the controller");
            m_state = KeyboardControllerState::SelfTest;
            return;
        }

        if (value == CMD_TEST_FIRST_PS2_PORT)
        {
            // Test keyboard port
            DC_CORE_TRACE("KBC Command: Self-test the keyboard");
            m_state = KeyboardControllerState::KeyboardTest;
            return;
        }

        if (value == CMD_READ_FIRST_PORT)
        {
            // Read data from the first port
            DC_CORE_TRACE("KBC Command: Read from first port");
            m_state = KeyboardControllerState::ReadFirstPort;
            return;
        }

        if (value == CMD_ENABLE_FIRST_PS2_PORT)
        {
            // Enable keyboard
            DC_CORE_TRACE("KBC Command: Enable Keyboard");
            m_state = KeyboardControllerState::EnableKeyboard;
            return;
        }

        if (value == CMD_DISABLE_FIRST_PS2_PORT)
        {
            DC_CORE_TRACE("KBC Command: Disable Keyboard");
            m_state = KeyboardControllerState::DisableKeyboard;
            return;
        }

        if (value == CMD_ENABLE_AUX_INTERFACE)
        {
            DC_CORE_TRACE("KBC Command: Enable Mouse Port");
            return;
        }
        
        if (value == CMD_SEND_CMD_AUX_DEVICE)
        {
            DC_CORE_TRACE("KBC Command: Sending data to mouse port");
            m_dataToMouse = true;
            return;
        }

        TODO();
    }

    void KeyboardController::writeDataPort(uint8_t value)
    {
        if (m_dataToMouse)
        {
            if (value == CMD_DEV_RESET)
                DC_CORE_TRACE("KBC Data Port: Received mouse reset");
            else
                DC_CORE_ERROR("KBC Data Port: Received unhandled mouse command");
            return;
        }

        switch (m_state)
        {
        case KeyboardControllerState::OK:
            if (value == CMD_DEV_ENABLE)
            {
                m_state = KeyboardControllerState::EnableKeyboard;
                m_dataBuffer.push_back(CMD_DEV_ACK);
                m_keyboardEnabled = true;
                return;
            }
            DC_CORE_TRACE("KBC Data Port: Getting a new value while not really in a mode expecting anything", value);
            break;
        case KeyboardControllerState::WaitingForCommandByte:
            DC_CORE_TRACE("KBC Data Port: writing new command byte 0b{0:b}", value);
            if (m_keyboardEnabled)
                m_state = KeyboardControllerState::EnableKeyboard;
            else
                m_state = KeyboardControllerState::OK;
            m_commandByte = value;
            return;
        case KeyboardControllerState::EnableKeyboard:
            // Here we get keyboard commands
            switch (value)
            {
            case CMD_DEV_RESET:
                DC_CORE_TRACE("KB: Reset :)");
                m_dataBuffer.push_back(CMD_BAT_PASSED);
                m_dataBuffer.push_back(CMD_DEV_ACK);
                m_keyboardEnabled = false;
                return;
            case CMD_DEV_DISABLE:
                DC_CORE_TRACE("KB: Disable");
                m_dataBuffer.push_back(CMD_DEV_ACK);
                return;
            case CMD_DEV_ENABLE:
                DC_CORE_TRACE("KB: Enable");
                m_dataBuffer.push_back(CMD_DEV_ACK);
                return;
            default:
                TODO();
                return;
            }

        default:
            break;
        }
        TODO();
    }

    uint8_t KeyboardController::readDataPort()
    {
        if (m_dataToMouse)
        {
            DC_CORE_TRACE("KBC Data Port: Reading mouse port, pretending we're not home");
            return 0;
        }
        switch (m_state)
        {
        case KeyboardControllerState::OK:
            break;
        case KeyboardControllerState::WaitingForCommandByte:
            break;
        case KeyboardControllerState::SelfTest:
            // 0x55 - test passed
            // 0xFC - test failed
            return 0x55;
        case KeyboardControllerState::KeyboardTest:
            // 0x00 - test passed
            // 0x01 - clock line stuck low
            // 0x02 - clock line stuck high
            // 0x03 - data line stuck low
            // 0x04 - data line stuck high
            return 0x00;
        case KeyboardControllerState::ReadFirstPort:
            DC_CORE_TRACE("KBC Data Port: Computer equipment somehow is stored here");
            // MDA
            return 0;
            // CGA
            return 0x40;
        case KeyboardControllerState::EnableKeyboard:

            if (m_dataBuffer.size() > 0)
            {
                uint8_t data = m_dataBuffer.back();
                m_dataBuffer.pop_back();

                return data;
            }
            return 0x00;
        case KeyboardControllerState::DisableKeyboard:
            // Keyboard was disabled, return ACK
            return 0xFA;
        default:
            break;
        }
        TODO();
        return 0;
    }

    uint8_t KeyboardController::readStatusRegister()
    {
        switch (m_state)
        {
        case KeyboardControllerState::OK:
            return 0;

        case KeyboardControllerState::WaitingForCommandByte:
            return 0;

        case KeyboardControllerState::SelfTest:
            return 1; // Return 1 to signify that we are ready to send an output :)

        case KeyboardControllerState::KeyboardTest:
            return 1;

        case KeyboardControllerState::ReadFirstPort:
            return 1;

        case KeyboardControllerState::EnableKeyboard:
            if (m_dataToMouse)
            {
                return 0x01;
            }
            return 1;

        case KeyboardControllerState::DisableKeyboard:
            return 1;

        default:
            break;
        }
        // Return 0 on startup
        // Return 0 when input buffer is empty afterwards
        TODO();
        return 1;
    }

    void KeyboardController::keyPressed(SDL_Scancode scancode)
    {
        // TODO: Mutex this somehow

        uint8_t byte = getByteFromSimpleScancode(scancode);
        if (byte != 0)
        {
            m_dataBuffer.push_back(byte);
            return;
        }

        // Handle more difficult stuff
        TODO();
    }

    void KeyboardController::keyReleased(SDL_Scancode scancode)
    {
        // TODO: Mutex this somehow

        uint8_t byte = getByteFromSimpleScancode(scancode);
        if (byte != 0)
        {
            m_dataBuffer.push_back(byte + 0x80); // Released has highest bit set
            return;
        }

        // Handle more difficult stuff
        TODO();
    }

    uint8_t KeyboardController::getByteFromSimpleScancode(SDL_Scancode scancode)
    {
        for (auto& code : s_SDLtoScancodeSet1)
        {
            if (code.scancode == scancode)
                return code.byte;
        }
        return 0x00;
    }
}
