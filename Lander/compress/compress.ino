//MPU9250 - 9DOF
//TSL2591 - Luminosity
//BME280 - Temperature, barometric pressure and humidity

//Testing Code

#include <Wire.h>
#include <SD.h>

#include "src/TSL2591.h"
#include "src/BME280.h"
#include "src/MPU9250.h"
#include "src/TinyGPS.h"

TSL2591 tsl = TSL2591();
BME280 bme = BME280(0x77);//0x76 if SDI grounded, or 0x77 if SDI is attached to logic level
MPU9250 mpu = MPU9250(0x68);// 0x68 if AD0 is grounded, 0x69 if AD0 is at logic level
TinyGPS gps;

template<class T> String &operator<<(String &lhs, T &rhs) {
    return lhs += rhs;
}

template<> String &operator<<(String &lhs, TSL2591 &rhs) {
    float lux;
    if (rhs.getLux(&lux)) {
        lhs << "Error with TSL2591 sensor";
    } else {
        lhs << "{Lux:" << lux << "}";
    }
    return lhs;
}

template<> String &operator<<(String &lhs, BME280 &rhs) {
    double temp, pres, humd;
    if (rhs.read_processed(&temp, &pres, &humd)) {
        lhs << "Error with BME280 sensor";
    } else {
        lhs << "{";
        lhs << "Temperature:" << temp << ",";
        lhs << "Pressure:" << pres << ",";
        lhs << "Humidity:" << humd << "}";
    }
    return lhs;
}

template<> String &operator<<(String &lhs, MPU9250 &rhs) {
    int16_t gyro[3], acc[3], magno[3];
    //float orientation[] = {0.0, 0.0, 0.0, 0.0};
    if (rhs.readGyrometerData(gyro) || rhs.readAccelometerData(acc) || rhs.readMagneoometerData(magno)){// || rhs.getQuaternion(orientation, gyro, acc, magno)) {
        lhs << "Error with MPU9250 sensor";
    } else {
        lhs << "{";
        lhs << "Gyrometer:[" << gyro[0] << "," << gyro[1] << "," << gyro[2] << "],";
        lhs << "Accelerometer:[" << acc[0] << "," << acc[1] << "," << acc[2] << "],";
        lhs << "Magnetometer:[" << magno[0] << "," << magno[1] << "," << magno[2] << "]}";
        //lhs << "Orientation:[" << orientation[0] << "," << orientation[1] << "," << orientation[2] << "," << orientation[3] << "]}";
    }
    return lhs;
}

void setup(){
    //On-Board led
    pinMode(13, OUTPUT);
    //SD Card
    if(!SD.begin(BUILTIN_SDCARD)){
        //turn on led if failed
        digitalWrite(13, HIGH);
        return;
    }
    File dataFile = SD.open("log.txt", FILE_WRITE);
    // Pam7Q
    /*
    Serial2.begin(9600);
    while(!Serial2) {}
     */
    //I2c
    Wire.begin();
    //TSL2591
    /*
    if(tsl.start(TSL2591_GAIN_1X, TSL2591_INTEGRATION_TIME_100MS)){
        dataFile.println("Couldn't connect to TSL2591 sensor");
    }
     */
    //BME280
    if (bme.start()) {
        dataFile.println("Couldn't connect to BME280 sensor");
    }
    else {
        delay(300);
        int i = 0;
        while(bme.isReadingCalibration() && i++ < 100) delay(100);
        bme.set(BME280_16x_OVERSAMPLING, BME280_16x_OVERSAMPLING, BME280_16x_OVERSAMPLING);
    }
    //MPU9250
    if(mpu.start()){
        dataFile.println("Couldn't connect to MPU9250 sensor");
    }
    dataFile.close();
}

void loop() {
    File dataFile = SD.open("log.txt", FILE_WRITE);
    String jsonData;
    // jsonData << "{TSL:" << tsl << ",BME:" << bme << ",MPU: " << mpu << "}";
    jsonData << "{BME:" << bme << ",MPU: " << mpu << "}";
    dataFile.println(jsonData);
    //Pam7Q
    /*
    bool newdata = false;
    unsigned long start = millis();
    while (millis() - start < 5000) {
        if (feedgps()) {
            newdata = true;
        }
    }
    if (newdata) {
        Serial.print("Pam7Q: ");
        gpsdump(gps);
    }*/
    dataFile.close();
    delay(50);
}



// Get and process GPS data
void gpsdump(TinyGPS &gps) {
  float flat, flon;
  unsigned long age;
  gps.f_get_position(&flat, &flon, &age);
  Serial.print(flat, 4);
  Serial.print(", ");
  Serial.println(flon, 4);
}

// Feed data as it becomes available
bool feedgps() {
  while (Serial2.available()) {
    if (gps.encode(Serial2.read())) {
      return true;
    }
  }
  return false;
}
