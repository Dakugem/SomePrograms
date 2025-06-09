#include "server.h"
#include "gamepad.h"
#include "bt_io.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>

#define FILENAME "raw_data.csv"

using namespace HID;
using namespace Socket_IO;
using namespace Bluetooth_IO;

using namespace std;
using namespace std::chrono;

string writeVector(vector<uint8_t> vec)
{
    ostringstream oss;
    for (size_t i = 0; i < vec.size() && i < 21; i++)
    {
        oss << setw(3) << +vec.at(i) << " ";
    }

    return oss.str();
}

// MAKE ASYNC IO!
// repeat connection if lost, non blocking for main thread with flags of availability
// time devision
// write py and processing servers

int main(int argc, char *argv[])
{
    std::cout.sync_with_stdio(false);
    Gamepad gamepad;
    Server server;
    BT_IO bt;

    bool NO_Gamepad = false, NO_Server = false, NO_Bluetooth = false, NO_CSV = false, NO_Detailed_Gamepad = true, VOICE_CONTROL = false;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "no_hid") == 0)
            NO_Gamepad = true;
        if (strcmp(argv[i], "no_serv") == 0)
            NO_Server = true;
        if (strcmp(argv[i], "no_bt") == 0)
            NO_Bluetooth = true;
        if (strcmp(argv[i], "no_csv") == 0)
            NO_CSV = true;
        if (strcmp(argv[i], "detail") == 0)
            NO_Detailed_Gamepad = false;
        if (strcmp(argv[i], "voice") == 0)
            VOICE_CONTROL = true;
    }

    if (NO_Gamepad)
        cout << "GAMEPAD_OFF" << "\n";

    if (!NO_Server)
    {
        if (!server.Setup())
            cout << "Server setup error" << "\n";
        else
            cout << "Server setup successfully" << "\n";
    }
    else
    {
        cout << "SERVER_OFF" << "\n";
    }

    if (!NO_Bluetooth)
    {
        bt.ScanAndConnect();
        if (!bt.isOkay())
            cout << "BT connection error" << "\n";
        else
            cout << "BT connected" << "\n";
    }
    else
    {
        cout << "BT_OFF" << "\n";
    }

    fstream file;
    if (!NO_CSV)
    {
        file.open(FILENAME, ios::in);
        if (!file.is_open())
        {
            cout << "Failed to open file: " << FILENAME << "\n";
        }

        string firstLine;
        getline(file, firstLine);
        file.close();
        Sleep(100);

        if (firstLine != "\"sequence_number\",\"delta\",\"gyro_x\",\"gyro_y\",\"gyro_z\",\"accel_x\",\"accel_y\",\"accel_z\",\"control_x\",\"control_speed\"")
        {
            file.open(FILENAME, ios::out);
            if (!file.is_open())
            {
                cout << "Failed to open file: " << FILENAME << "\n";
            }
            file << "\"sequence_number\",\"delta\",\"gyro_x\",\"gyro_y\",\"gyro_z\",\"accel_x\",\"accel_y\",\"accel_z\",\"control_x\",\"control_speed\"\n";
        }
        else
        {
            file.open(FILENAME, ios::app);
            if (!file.is_open())
            {
                cout << "Failed to open file: " << FILENAME << "\n";
            }
			
			for(int i = 0; i < 7; i++){
				file << "\"" << i   << "\",";
                file << "\"" << 0.1 << "\","; 
                file << "\"" << 0   << "\","; 
                file << "\"" << 0   << "\",";
                file << "\"" << 0   << "\",";
                file << "\"" << 0   << "\","; 
                file << "\"" << 0   << "\",";
                file << "\"" << -1  << "\",";
                file << "\"" << 0   << "\","; 
                file << "\"" << 0   << "\"";
                file << "\n";
			}
        }
    }
    else
    {
        cout << "NO_CSV" << "\n";
    }
    bool isEnd = false;
    int8_t speed = 0;
    ostringstream oss;
    auto old_time = high_resolution_clock::now();
    auto new_time = old_time;
    auto duration = duration_cast<microseconds>(new_time - old_time);

    Gamepad_Data Gamepad_data;
    vector<int64_t> data_to_csv{};
    data_to_csv.resize(10);
    vector<uint8_t> msg_from_client; // for test client answer is sequence number of received packet (uint)
    vector<uint8_t> msg_to_client;   // timestamp of last bt inputs 8 bytes, 12 bytes for accelGyro 1 or powerOff client
    msg_to_client.resize(21);
    uint8_t temp_input[1];
    uint8_t input[16]{}; // 16 bytes used
    uint8_t output[8]{}; // 2 bytes for x, 1 byte for speed + 5 reserved
    int16_t AccelGyro[6];
    vector<vector<int>> mean; // 5 el for each axis
    mean.resize(6);
    bool MANUAL_CONTROL = !VOICE_CONTROL;
    while (!isEnd)
    {
        if (!NO_Gamepad)
        {
            if (!gamepad.Refresh())
            {
                if (!gamepad.CheckConnection())
                    cout << "Xbox controller not connected" << "\n";
            }
            else
            {
                Gamepad_data = gamepad.GetData();

                if (!NO_Detailed_Gamepad)
                {
                    oss << "Left thumb stick: (" << setw(10) << Gamepad_data.LX << ", " << setw(10) << Gamepad_data.LY << ")   Right thumb stick : ("
                        << setw(10) << Gamepad_data.RX << ", " << setw(10) << Gamepad_data.RY << ")" << "\n";
                    oss << "Left analog trigger: " << setw(10) << Gamepad_data.LT << "   Right analog trigger: " << setw(10) << Gamepad_data.RT << "\n";
                    oss << "Buttons Pressed: 0x" << std::hex << Gamepad_data.buttonsPressed << std::dec << "\n";
                }

                speed = (speed > 111) ? 127 : speed + Gamepad_data.RT * 16;
                speed = (speed < -111) ? -127 : speed - Gamepad_data.LT * 16;

                if (gamepad.IsPressed(XINPUT_GAMEPAD_BACK))
                {
                    gamepad.SetVibration(32000, 16000);
                    Sleep(200);
                    gamepad.SetVibration(0, 0);
                    speed = 0;
                    isEnd = true;
                }
                if (gamepad.IsPressed(XINPUT_GAMEPAD_A))
                {
                    gamepad.SetVibration(16000, 32000);
                    Sleep(200);
                    gamepad.SetVibration(0, 0);
                }
                if (gamepad.IsPressed(XINPUT_GAMEPAD_B))
                {
                    speed = 0;
                }
                if (gamepad.IsPressed(XINPUT_GAMEPAD_X))
                {
                    speed = 127;
                }
                if (gamepad.IsPressed(XINPUT_GAMEPAD_Y))
                {
                    MANUAL_CONTROL = !MANUAL_CONTROL;
                }

                if (VOICE_CONTROL && !MANUAL_CONTROL)
                {
                }
                else
                {
                    output[0] = (int16_t)(Gamepad_data.LX * 512) >> 8;
                    output[1] = (int16_t)(Gamepad_data.LX * 512) & 0xFF;
                    output[2] = speed; //(speed & 0x8000) >> 8 | (speed & 0x007F);
                    output[3] = 0;
                    output[4] = 0;
                    output[5] = 0;
                    output[6] = 0;
                    output[7] = 0;

                    oss << setw(16) << "X: " << setw(5) << (int16_t)(Gamepad_data.LX * 512)
                        << "\n"
                        << setw(16) << "Speed: " << setw(5) << (int)(int8_t)output[2] << "\n";
                }
            }
        }

        if (!NO_Server)
        {
            if (!server.isClientOkay())
            {
                if (server.ConnectToClient())
                    cout << "Can't connect to client" << "\n";
            }
            else
            {
                for (size_t i = 0; i < 8; i++)
                {
                    msg_to_client.at(i) = (duration.count() >> 8 * (7 - i)) & 0xFF;
                }
                for (size_t i = 8; i < 20; i++)
                {
                    msg_to_client.at(i) = input[i - 8];
                }

                if (isEnd)
                    msg_to_client.at(20) = 0xFF;
                server.sendMessage(msg_to_client);
                server.recvMessage(msg_from_client);

                oss << setw(16) << "To Client: "
                    << writeVector(msg_to_client) << "\n";

                oss << setw(16) << "From Client: "
                    << writeVector(msg_from_client) << "\n";
                if (VOICE_CONTROL && !MANUAL_CONTROL)
                {
                    switch (msg_from_client.at(4))
                    {
                    case 0:
                        output[0] = 0;
                        output[1] = 0;
                        output[2] = 0;
                        break;
                    case 1:
                        if (speed >= 0)
                        {
                            if (speed > (127 - 16 * (msg_from_client.at(6) + 1)))
                                speed = 127;
                            else
                                speed += 16 * (msg_from_client.at(6) + 1);
                        }
                        else
                        {
                            if (speed < -(127 - 16 * (msg_from_client.at(6) + 1)))
                                speed = -127;
                            else
                                speed -= 16 * (msg_from_client.at(6) + 1);
                        }
                        output[2] = speed;
                        break;
                    case 2:
                        if (speed >= 0)
                        {
                            if (speed < (16 * (msg_from_client.at(6) + 1)))
                                speed = 0;
                            else
                                speed -= 16 * (msg_from_client.at(6) + 1);
                        }
                        else
                        {
                            if (speed > -(16 * (msg_from_client.at(6) + 1)))
                                speed = 0;
                            else
                                speed += 16 * (msg_from_client.at(6) + 1);
                        }
                        output[2] = speed;
                        break;
                    case 3:
                        if (speed >= 0)
                        {
                            speed = (16 * msg_from_client.at(6) < 127) ? 16 * msg_from_client.at(6) : 127;
                        }
                        else
                        {
                            speed = (16 * msg_from_client.at(6) < 127) ? -16 * msg_from_client.at(6) : -127;
                        }
                        output[2] = speed;
                        break;
                    case 4:
                        speed = (speed > 0) ? speed : -speed;
                        output[2] = speed;
                        break;
                    case 5:
                        speed = (speed > 0) ? -speed : speed;
                        output[2] = speed;
                        break;
                    default:
                        break;
                    }
                    switch (msg_from_client.at(5))
                    {
                    case 0:
                        output[0] = 0;
                        output[1] = 0;
                        break;
                    case 1:
                        output[0] = (int16_t)((msg_from_client.at(6) > 0) ? -50 * msg_from_client.at(6) : -400) >> 8;
                        output[1] = (int16_t)((msg_from_client.at(6) > 0) ? -50 * msg_from_client.at(6) : -400) & 0xFF;
                        break;
                    case 2:
                        output[0] = (int16_t)((msg_from_client.at(6) > 0) ? 50 * msg_from_client.at(6) : 400) >> 8;
                        output[1] = (int16_t)((msg_from_client.at(6) > 0) ? 50 * msg_from_client.at(6) : 400) & 0xFF;
                        break;
                    default:
                        break;
                    }

                    isEnd = msg_from_client.at(7);
                    if (isEnd)
                        output[2] = 0;

                    oss << setw(16) << "X: " << setw(5) << (int16_t)(output[0] << 8 | output[1])
                        << "\n"
                        << setw(16) << "Speed: " << setw(5) << (int)(int8_t)output[2] << "\n";
                }
            }
        }

        if (!NO_Bluetooth)
        {
            if (!bt.isOkay())
            {
                cout << "BT connection error" << "\n";
                bt.ScanAndConnect();
            }
            else
            {
                bt.Write(output, 8);

                bt.Read(temp_input, 1);
                if (temp_input[0] == 'Y')
                {
                    bt.Read(input, 16);

                    old_time = new_time;
                    new_time = high_resolution_clock::now();
                    duration = duration_cast<microseconds>(new_time - old_time);

                    if (input[15] == 0xFF)
                    {
                        oss << setw(16) << "BT BMI Input: ";
                        for (size_t i = 0; i < 6; i++)
                        {
                            AccelGyro[i] = (input[2 * i] << 8) | input[2 * i + 1];

                            if (mean.at(i).size() < 5)
                                mean.at(i).push_back(AccelGyro[i]);
                            else
                            {
                                for (int j = 0; j < 4; j++)
                                {
                                    mean.at(i).at(j) = mean.at(i).at(j + 1);
                                }
                                mean.at(i).at(4) = AccelGyro[i];
                            }
                            int temp = 0;
                            for (int j = 0; j < mean.at(i).size(); j++)
                            {
                                temp += mean.at(i).at(j);
                            }
                            temp = (int)((double)temp / mean.at(i).size());
                            AccelGyro[i] = temp;
                            double temp2 = (i < 3) ? (double)(AccelGyro[i] / (16.4)) : (double)(AccelGyro[i] / 16384.);
                            oss << temp2 << " ";
                        }
                        oss << '\n';
                    }
                    else if (input[15] == 0x00)
                    {
                        oss << setw(16) << "BT Input: " << +input[0] << " " << +input[1] << " " << +input[2] << " " << +input[3] << " "
                            << +input[5] << " " << +input[5] << " " << +input[6] << " " << +input[7] << " " << +input[8] << " " << +input[9]
                            << +input[10] << " " << +input[11] << " " << +input[12] << " " << +input[13] << " " << +input[14] << " " << +input[15] << '\n';
                    }
                    else
                    {
                        oss << "BMI flag error" << '\n';
                    }
                }
                else
                    oss << "BT Start flag Error: " << +temp_input[0] << '\n';
            }
        }

        if (!NO_CSV && !NO_Bluetooth && !NO_Gamepad)
        {
            data_to_csv.at(0)++;
            data_to_csv.at(1) = duration.count();
            for (size_t i = 2; i < 8; i++)
            {
                data_to_csv.at(i) = AccelGyro[i - 2];
            }
            data_to_csv.at(8) = (int16_t)(Gamepad_data.LX * 512);
            data_to_csv.at(9) = (int8_t)speed;

            file << "\"" << +data_to_csv[0] << "\",";
            file << "\"" << (double)(data_to_csv[1] / 1000000.) << "\","; // 10^6 per 1 second [-x; +x] -> [-x/10^6: +x/10^6] ~ [-1; +1]
            file << "\"" << (double)(data_to_csv[2] / 16384.) << "\",";   // 16.384 per 1 degree  [-2000; +2000] -> [-2; +2]
            file << "\"" << (double)(data_to_csv[3] / 16384.) << "\",";
            file << "\"" << (double)(data_to_csv[4] / 16384.) << "\",";
            file << "\"" << (double)(data_to_csv[5] / 16384.) << "\","; // 16384 per 1g [-2g; +2g] -> [-2; +2]
            file << "\"" << (double)(data_to_csv[6] / 16384.) << "\",";
            file << "\"" << (double)(data_to_csv[7] / 16384.) << "\",";
            file << "\"" << (double)(data_to_csv[8] / 512.) << "\","; // 512 per 1 [-1; +1] -> [-1; +1] for learning
            file << "\"" << (double)(data_to_csv[9] / 127.) << "\"";  // 127 per 1 [-1; +1] -> [-1; +1] for learning
            file << "\n";
        }

        cout << oss.str() << endl;
        oss.str("");

        Sleep(20);
    }

    if (!NO_CSV)
    {
        file.close();
        cout << "CSV file created successfully: " << FILENAME << "\n";
    }

    return 0;
}