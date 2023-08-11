#pragma once

#include "i2c.hpp"

#define SMPLRT_DIV 0x19   // Gyroscope sampling rate, typical:0x07(125Hz)
#define CONFIG 0x1A       // Low-pass filter frequency, typical：0x06(5Hz)
#define GYRO_CONFIG 0x1B  // Gyro Self-Test and Measurement Range, Typical：0x18(Don't check yourself.，2000deg/s)
#define ACCEL_CONFIG 0x1C // Accelerometer Self-Test, Measurement Range, and High-Pass Filter Frequency, Typical：0x01(Don't check yourself.，2G，5Hz)
#define MPU6050_MOT_THR 0x1F
#define MPU6050_MOT_DUR 0x20
#define INT_PIN_CFG 0x37 // Config INT pin
#define INT_ENABLE 0x38  // Enable INT pin
#define INT_STATUS 0x3A  // Status of INT pin
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48
#define PWR_MGMT_1 0x6B   // Power management, typical: 0x00 (normally enabled)
#define WHO_AM_I 0x75     // IIC address register (default value 0x68, read-only)
#define MPU6050_ADDR 0x68 // Address byte data for IIC write, +1 for read

class MPU6050
{
private:
    I2C *i2c;

public:
    MPU6050(gpio_num_t scl, gpio_num_t sda, i2c_port_t port);
    ~MPU6050();
    bool init();

    bool setFastMotionCapture(bool enabled);

    float getAccX();
    float getAccY();
    float getAccZ();

    float getGyroX();
    float getGyroY();
    float getGyroZ();

    short getTemp();

    bool getInter();
};
