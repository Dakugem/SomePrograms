#include "bt_io.h"

namespace Bluetooth_IO
{
    BT_IO::BT_IO() : handle{INVALID_HANDLE_VALUE} {}

    void BT_IO::Connect(const std::string &port)
    {
        Disconnect();

        handle = CreateFileA(
            (LPCSTR)port.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (handle == INVALID_HANDLE_VALUE)
            throw BTException(CREATE_FILE_ERROR);

        SetCommMask(handle, EV_RXCHAR);
        SetupComm(handle, 1500, 1500);

        COMMTIMEOUTS CommTimeOuts;
        CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
        CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
        CommTimeOuts.ReadTotalTimeoutConstant = BT_TIMEOUT;
        CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
        CommTimeOuts.WriteTotalTimeoutConstant = BT_TIMEOUT;

        if (!SetCommTimeouts(handle, &CommTimeOuts))
        {
            Disconnect();
            throw BTException(COM_TIMEOUTS_ERROR);
        }

        DCB ComDCM;

        memset(&ComDCM, 0, sizeof(ComDCM));
        ComDCM.DCBlength = sizeof(DCB);
        GetCommState(handle, &ComDCM);
        ComDCM.BaudRate = CBR_115200;
        ComDCM.ByteSize = 8;
        ComDCM.Parity = NOPARITY;
        ComDCM.StopBits = ONESTOPBIT;
        ComDCM.fAbortOnError = TRUE;
        ComDCM.fDtrControl = DTR_CONTROL_DISABLE;
        ComDCM.fRtsControl = RTS_CONTROL_DISABLE;
        ComDCM.fBinary = TRUE;
        ComDCM.fParity = FALSE;
        ComDCM.fInX = FALSE;
        ComDCM.fOutX = FALSE;
        ComDCM.XonChar = 0;
        ComDCM.XoffChar = (unsigned char)0xFF;
        ComDCM.fErrorChar = FALSE;
        ComDCM.fNull = FALSE;
        ComDCM.fOutxCtsFlow = FALSE;
        ComDCM.fOutxDsrFlow = FALSE;
        ComDCM.XonLim = 128;
        ComDCM.XoffLim = 128;

        if (!SetCommState(handle, &ComDCM))
        {
            Disconnect();
            throw BTException(COM_STATE_ERROR);
        }
    }
    void BT_IO::ScanAndConnect()
    {
        for (int i = 1; i < 15; i++)
        {
            try
            {
                std::string port = "\\\\.\\COM" + std::to_string(i);
                this->Connect(port);
                uint8_t temp[128];
                this->Read(temp, 128);

                uint8_t answer[]{'y'};
                if (temp[0] == 'H')
                {
                    this->Write(answer, 1);
                    //this->Read(temp, 128);
                    return;
                }
            }
            catch (const BTException &ex)
            {
                // std::cout << ex.exception << std::endl;
            }
        }
    }
    void BT_IO::Disconnect()
    {
        if (handle != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle);
            handle = INVALID_HANDLE_VALUE;
        }
    }

    bool BT_IO::isOkay(){
        return handle != INVALID_HANDLE_VALUE;
    }

    void BT_IO::Read(uint8_t *data, const size_t data_length)
    {
        if (handle == INVALID_HANDLE_VALUE)
            throw BTException(HANDLE_VALUE_ERROR);

        DWORD begin = GetTickCount();
        DWORD bytesRead;

        uint8_t *buf = data;
        size_t buf_length = data_length;
        while (buf_length && (GetTickCount() - begin) < BT_TIMEOUT)
        {

            if (!ReadFile(handle, buf, buf_length, &bytesRead, NULL))
            {
                Disconnect();
                throw BTException(READ_ERROR);
            }

            buf_length -= bytesRead;
            buf += bytesRead;
        }
    }
    void BT_IO::Write(const uint8_t *data, const size_t data_length)
    {
        if (handle == INVALID_HANDLE_VALUE)
            throw BTException(HANDLE_VALUE_ERROR);

        DWORD bytesWritten;
        if (!WriteFile(handle, data, (DWORD)data_length, &bytesWritten, NULL) || bytesWritten != (DWORD)data_length)
        {
            Disconnect();
            throw BTException(WRITE_ERROR);
        }
    }

    BT_IO::~BT_IO() { Disconnect(); }
}