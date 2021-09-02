#include "cepumspch.h"
#include "KeyboardController.h"

namespace Cepums {

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
            DC_CORE_TRACE("KBC Data Port: Reading first port, no clue what to output, returning 0");
            return 0x00;
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
}
