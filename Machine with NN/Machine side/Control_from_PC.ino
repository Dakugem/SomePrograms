#include <BluetoothSerial.h>
#include <esp32-hal-ledc.h>

//For quick charge
#define D_P GPIO_NUM_25
#define D_M GPIO_NUM_26

//HC-SR04 Pins
#define ECHO GPIO_NUM_32
#define TRIG GPIO_NUM_33

//#define GYRO_500_DPS
//#define NO_BMI
//BMI 160 Pins - I2C Pins and finds automaticaly
//#define Accel_SCL GPIO_NUM_22
//#define Accel_SDA GPIO_NUM_21
#define Gyro_X_Offset 2
#define Gyro_Y_Offset -3
#define Gyro_Z_Offset 4
#define Accel_X_Offset -700
#define Accel_Y_Offset 40
#define Accel_Z_Offset -800

//PWM
#define PWM_Frequency 4000
#define PWM_Resolution 6
#define PWM_Channel_1 0
#define PWM_Channel_2 1
#define PWM_Channel_3 2
#define PWM_Channel_4 3
#define PWM_Channel_5 4
#define PWM_Channel_6 5
#define PWM_Channel_7 6
#define PWM_Channel_8 7

//Motor Drivers Pins
#define MotorPin_1_1 GPIO_NUM_16
#define MotorPin_1_2 GPIO_NUM_17
#define MotorPin_1_3 GPIO_NUM_18
#define MotorPin_1_4 GPIO_NUM_19
#define MotorPin_2_1 GPIO_NUM_23
#define MotorPin_2_2 GPIO_NUM_27
#define MotorPin_2_3 GPIO_NUM_13
#define MotorPin_2_4 GPIO_NUM_12

#define BT_TIMEOUT 200

int8_t speed = 0;
int16_t x = 0;

String device_name = "ESP32-BT-Machine";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
static size_t ctr = 0;

#if !defined(NO_BMI)
#include <DFRobot_BMI160.h>
DFRobot_BMI160 bmi160;

const int8_t i2c_addr = 0x69;
static int16_t AccelGyro[6]{ 0 };
#endif

#define IN_DATA_SIZE 8 // 2 bytes for x and 1 byte for speedm other reserved
static uint8_t in_data[IN_DATA_SIZE]{ 0 };
#define OUT_DATA_SIZE 16  // (16 bit * 6 axis + reserved 32 bits)/8 = 16 bytes   16th byte = 0x00 if NO_BMI defined & 0xFF if not
static uint8_t out_data[OUT_DATA_SIZE]{ 0 };



void GPIO_Set_Mode();
bool Blink();
void ProcessInputs();
void GetOutputs();

void setup() {
  GPIO_Set_Mode();

  Serial.begin(115200);
  SerialBT.begin(device_name);  //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented

#ifdef USE_PIN
  SerialBT.setPin(pin);
  Serial.println("Using PIN");
#endif

  delay(100);

  /*
if(ledcWrite(QUICK_CHARGE_PIN, 50)) Serial.println("PWM Duty set successful");
else Serial.println("PWM Duty set error");
*/

  dacWrite(D_P, 45);
  gpio_set_direction(D_M, GPIO_MODE_INPUT);
  Serial.println("D+ = 0.6  D- = INPUT_MODE");

  delay(1150);
  Serial.println("Waiting for 0 on D-");
  for (int i = 0; i < 10 && analogRead(D_M) > 400; ++i) {
    delay(100);
    Serial.print("D- = ");
    Serial.println(analogRead(D_M));
  }
  if (analogRead(D_M) < 400) {
    Serial.println("D- is now 0!");

    gpio_set_direction(D_M, GPIO_MODE_OUTPUT);
    delay(10);

    Serial.println("D+ = 3.3  D- = 0.6");
    dacWrite(D_P, 255);
    dacWrite(D_M, 47);

    Serial.println("Quick charge enabled");
  } else{
    gpio_set_direction(D_M, GPIO_MODE_OUTPUT);
    delay(10);
    dacWrite(D_P, 170);
    dacWrite(D_M, 120);
    Serial.println("Quick charge disabled");
  }

#if !defined(NO_BMI)
  if (bmi160.softReset() != BMI160_OK) {
    Serial.println("reset false");
    while (1)
      ;
  }

  // подключение к датчику по указанному адресу
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK) {
    Serial.println("init false");
    while (1)
      ;
  }
#endif

  while (!Blink())
    ;

  Serial.println("Blink successful");

  //Serial.println("Gyro X, Gyro Y, Gyro Z, Accel X, Accel Y, Accel Z");
}

void loop() {
  if (SerialBT.available()) {
    ctr = 0;
    for (size_t i = 0; i < IN_DATA_SIZE; ++i) {
      in_data[i] = SerialBT.read();
    }
    //while (SerialBT.available()) SerialBT.read();

    delay(10);
    SerialBT.write('Y');

    GetOutputs();
    #ifdef NO_BMI
      out_data[15] = 0x00;
    #else
      out_data[15] = 0xFF;
    #endif
    for (size_t i = 0; i < OUT_DATA_SIZE; ++i) {
      SerialBT.write(out_data[i]);
      //Serial.print(+out_data[i]);
      //Serial.print(" ");
    }
    //Serial.print("\n");
    for(size_t i = 0; i < 6; i++){
      Serial.print(AccelGyro[i]);
      Serial.print(" ");
    }
    Serial.println("");

    ProcessInputs();
  }else if(ctr > 50){
    gpio_set_level(MotorPin_1_1, LOW);
    gpio_set_level(MotorPin_1_2, LOW);
    gpio_set_level(MotorPin_1_3, LOW);
    gpio_set_level(MotorPin_1_4, LOW);
    gpio_set_level(MotorPin_2_1, LOW);
    gpio_set_level(MotorPin_2_2, LOW);
    gpio_set_level(MotorPin_2_3, LOW);
    gpio_set_level(MotorPin_2_4, LOW);
  }else{
    ctr++;
    //Serial.print("Ctr = ");
    //Serial.println(ctr);
  }

  delay(10);
}

void GPIO_Set_Mode() {
  //Quick charge pins
  gpio_reset_pin(D_P);
  gpio_reset_pin(D_M);
  gpio_set_direction(D_P, GPIO_MODE_OUTPUT);
  gpio_set_direction(D_M, GPIO_MODE_OUTPUT);

  //HC-SR04 Pins
  //gpio_reset_pin(ECHO);
  //gpio_reset_pin(TRIG);
  //gpio_set_direction(ECHO, GPIO_MODE_INPUT);
  //gpio_set_direction(TRIG, GPIO_MODE_OUTPUT);

  //BMI 160 Pins - I2C Auto configure

  gpio_reset_pin(MotorPin_1_1);
  gpio_reset_pin(MotorPin_1_2);
  gpio_reset_pin(MotorPin_1_3);
  gpio_reset_pin(MotorPin_1_4);
  gpio_reset_pin(MotorPin_2_1);
  gpio_reset_pin(MotorPin_2_2);
  gpio_reset_pin(MotorPin_2_3);
  gpio_reset_pin(MotorPin_2_4);
  gpio_set_direction(MotorPin_1_1, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_1_2, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_1_3, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_1_4, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_2_1, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_2_2, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_2_3, GPIO_MODE_OUTPUT);
  gpio_set_direction(MotorPin_2_4, GPIO_MODE_OUTPUT);

  ledcAttach(MotorPin_1_1, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_1_2, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_1_3, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_1_4, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_2_1, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_2_2, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_2_3, PWM_Frequency, PWM_Resolution);
  ledcAttach(MotorPin_2_4, PWM_Frequency, PWM_Resolution);
}

bool Blink() {
  SerialBT.write('H');

  delay(500);

  char input = ' ';
  if (SerialBT.available()) {
    input = SerialBT.read();
    if (input == 'y') return true;
  }

  return false;
}

void ProcessInputs() {
  //in_data[0] - in_data[7] is motorPins controls, in_data[8] - in_data[IN_DATA_SIZE - 1] - reserved for LEDs, maybe.
  /*
  gpio_set_level(MotorPin_1_1, (in_data[0]) ? HIGH : LOW);
  gpio_set_level(MotorPin_1_2, (in_data[1]) ? HIGH : LOW);
  gpio_set_level(MotorPin_1_3, (in_data[2]) ? HIGH : LOW);
  gpio_set_level(MotorPin_1_4, (in_data[3]) ? HIGH : LOW);
  gpio_set_level(MotorPin_2_1, (in_data[4]) ? HIGH : LOW);
  gpio_set_level(MotorPin_2_2, (in_data[5]) ? HIGH : LOW);
  gpio_set_level(MotorPin_2_3, (in_data[6]) ? HIGH : LOW);
  gpio_set_level(MotorPin_2_4, (in_data[7]) ? HIGH : LOW);
  */
  //Process next in_data[]

  size_t MP1_Odd;
  size_t MP2_Odd;
  size_t MP1_Even;
  size_t MP2_Even;

  speed = (int8_t)in_data[2] / 2;
  if(speed > 63) speed = 63;
  else if (speed < -63) speed = -63;
  x = in_data[0] << 8 | in_data[1];

  if (speed >= 50/4.) {
    if (x >= 0) {
      MP1_Odd  = speed * (1 - (double)x / 512.);
      MP2_Odd  = speed;
      MP1_Even = 0;
      MP2_Even = 0;
    } else if (x <= 0) {
      MP1_Odd  = speed;
      MP2_Odd  = speed * (1 + (double)x / 512.);
      MP1_Even = 0;
      MP2_Even = 0;
    } else {
      MP1_Odd  = speed;
      MP2_Odd  = speed;
      MP1_Even = 0;
      MP2_Even = 0;
    }
  } else if (speed <= -50/4.) {
    if (x >= 0) {
      MP1_Odd  = 0;
      MP2_Odd  = 0;
      MP1_Even = -1 * speed * (1 - (double)x / 512.);
      MP2_Even = -1 * speed;
    } else if (x <= 0) {
      MP1_Odd  = 0;
      MP2_Odd  = 0;
      MP1_Even = -1 * speed;
      MP2_Even = -1 * speed * (1 + (double)x / 512.);
    } else {
      MP1_Odd  = 0;
      MP2_Odd  = 0;
      MP1_Even = -1 * speed;
      MP2_Even = -1 * speed;
    }
  } else {
    MP1_Odd  = 0;
    MP2_Odd  = 0;
    MP1_Even = 0;
    MP2_Even = 0;
  }

  /*
  Serial.printf(
    "Speed=%d, MP1_Odd=%d, MP1_Even=%d, MP2_Odd=%d, MP2_Even=%d\n",
    speed, MP1_Odd, MP1_Even, MP2_Odd, MP2_Even);
  */

  ledcWrite(MotorPin_1_1, MP1_Odd);
  ledcWrite(MotorPin_1_2, MP1_Even);
  ledcWrite(MotorPin_1_3, MP1_Odd);
  ledcWrite(MotorPin_1_4, MP1_Even);
  ledcWrite(MotorPin_2_1, MP2_Odd);
  ledcWrite(MotorPin_2_2, MP2_Even);
  ledcWrite(MotorPin_2_3, MP2_Odd);
  ledcWrite(MotorPin_2_4, MP2_Even);
}

#if !defined(NO_BMI)
void GetOutputs() {
  int8_t result = bmi160.getAccelGyroData(AccelGyro);

  if (result != 0) Serial.println("BMI160 read error");
  else {
    int16_t temp = AccelGyro[0];
    AccelGyro[0] = AccelGyro[1];
    AccelGyro[1] = temp;
            temp = AccelGyro[3];
    AccelGyro[3] = AccelGyro[4];
    AccelGyro[4] = temp;
	  //change x with y
    AccelGyro[0] *=  1;
    AccelGyro[1] *=  1;
    AccelGyro[2] *= -1;   
    AccelGyro[3] *=  1;
    AccelGyro[4] *=  1;
    AccelGyro[5] *=  1; 

    AccelGyro[0] -= Gyro_X_Offset;
    AccelGyro[1] -= Gyro_Y_Offset;
    AccelGyro[2] -= Gyro_Z_Offset;
    AccelGyro[3] -= Accel_X_Offset;
    AccelGyro[4] -= Accel_Y_Offset;
    AccelGyro[5] -= Accel_Z_Offset;

    for (size_t i = 0; i < 6; ++i) {

      out_data[i * 2] = AccelGyro[i] >> 8;
      out_data[i * 2 + 1] = AccelGyro[i] & 0xFF;
    }
  }
}
#else
void GetOutputs() {
  for (size_t i = 0; i < OUT_DATA_SIZE; ++i) {
    out_data[i] += 1;
  }
}
#endif