// millis has about 50 days before roll over

// no eeprom code yet

#include <Wire.h>


#define I2C_ADDRESS 0X41        // for empty eeprom write

#define I2C_WAKEMEMINS 0x10
#define I2C_WAKEMEHOURS 0x11
#define I2C_SHUTOFFDELAY 0x12
#define I2C_ADDRESS_CHANGE 0x13
#define I2C_CANCEL 0x14
#define I2C_CAMERA 0x15

#define I2C_OK 0x01
#define I2C_ERROR 0x02

#define POWER_PIN 5
#define POWER_ON LOW
#define POWER_OFF HIGH

#define CAMERA_PIN 4
#define CAMERA_OFF HIGH
#define CAMERA_ON LOW

#define SHUTOFF_DELAY   120000      // two minutes for empty eeprom write
#define MINS_FACTOR     60000
#define HOURS_FACTOR    3600000
#define CAMERA_DELAY    500         // in millis

unsigned long shutOffDelay = SHUTOFF_DELAY;     // shoud be eeprom read
unsigned long wakeDelay = 0;
unsigned long startTime = 0;
unsigned long cameraTime = 0;
unsigned long cameraDelay = CAMERA_DELAY;

int i2cAddress = I2C_ADDRESS;
byte lastState = I2C_ERROR;
byte shutOffFlag = false;
byte wakeFlag = false;
byte addressChangeFlag = false;
byte cameraFlag = false;

void setup() 
{
    pinMode(POWER_PIN, OUTPUT);
    pinMode(CAMERA_PIN, OUTPUT);
    digitalWrite(POWER_PIN, POWER_ON);
    digitalWrite(CAMERA_PIN, CAMERA_OFF);
    
    Wire.onReceive(receiveHandler);
    Wire.onRequest(requestHandler);
    Wire.begin(i2cAddress);
}

void loop()
{
    // shut off check
    if (shutOffFlag == true && (millis() - startTime) > shutOffDelay) {
        digitalWrite(POWER_PIN, POWER_OFF);
        shutOffFlag = false;
    }
    // wake check
    if (wakeFlag == true && (millis() - startTime) > wakeDelay) {
        digitalWrite(POWER_PIN, POWER_ON);
        wakeFlag = false;
    }
    // camera check
    if (cameraFlag == true && (millis() - cameraTime) > cameraDelay) {
        digitalWrite(CAMERA_PIN, CAMERA_OFF);
        cameraFlag = false;
    }
}

void receiveHandler(int numBytes)
{
    byte command;
    int mins, hours = 0;
    command = Wire.read();
    lastState = I2C_ERROR;
    // process i2c commands
    switch (command) {
        case I2C_CANCEL:
            shutOffFlag = false;
            wakeFlag = false;
            lastState = I2C_OK;
            break;
        case I2C_WAKEMEMINS:
            if (numBytes != 2)
                break;
            mins = Wire.read();
            
            wakeDelay = mins * MINS_FACTOR;
            startTime = millis();
            shutOffFlag = true;
            wakeFlag = true;
            lastState = I2C_OK;
            break;
        case I2C_WAKEMEHOURS:
            if (numBytes != 2)
                break;
            hours = Wire.read();
            
            wakeDelay = hours * HOURS_FACTOR;
            startTime = millis();
            shutOffFlag = true;
            wakeFlag = true;
            lastState = I2C_OK;
            break;
        case I2C_SHUTOFFDELAY:  // shutoff delay update and eeprom write
            if (numBytes != 2)
                break;
            mins = Wire.read();
            shutOffDelay = mins * MINS_FACTOR;
            // add eeprom write
            lastState = I2C_OK;
            break;
        case I2C_ADDRESS_CHANGE: //i2c address to eeprom
            if (numBytes != 2)
                break;
            i2cAddress = Wire.read();
            // add eeprom write
            lastState = I2C_OK;
            addressChangeFlag = true;
            break;
        case I2C_CAMERA: //i2c camera for x seconds
            if (numBytes != 2)
                break;
            cameraDelay = Wire.read();
            cameraTime = millis();
            digitalWrite(CAMERA_PIN, CAMERA_ON);
            cameraFlag = true;
            // add eeprom write
            lastState = I2C_OK;
            break;
        default:
            break;
    }
    // should we empty Wire buffer??
}

void requestHandler()
{
    Wire.write(lastState);
    if (addressChangeFlag) {
        Wire.begin(i2cAddress);
        addressChangeFlag = false;
    }
}
