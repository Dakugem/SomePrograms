#pragma once

#include <windows.h>
#include <string>
#include <vector>

#define BT_TIMEOUT 500

namespace Bluetooth_IO
{
    class BT_IO
    {
        HANDLE handle;

    public:
        BT_IO();

        void Connect(const std::string &port);
        void ScanAndConnect();
        void Disconnect();

        bool isOkay();

        void Read(uint8_t *data, const size_t data_length);    
        void Write(const uint8_t *data, const size_t data_length); 

        ~BT_IO();
    };

    enum BTExceptions
    {
        NONE,
        CREATE_FILE_ERROR,
        COM_TIMEOUTS_ERROR,
        COM_STATE_ERROR,
        HANDLE_VALUE_ERROR,
        READ_ERROR,
        WRITE_ERROR,
        TIMEOUT,
    };

    struct BTException
    {
        BTExceptions exception;

        constexpr BTException(BTExceptions _exception) : exception(_exception) {}
    };
}