#include "mpu6050.hpp"

#define ORIGINAL_OUTPUT (0)
#define ACC_FULLSCALE (2)
#define GYRO_FULLSCALE (250)

#if ORIGINAL_OUTPUT == 0
#if ACC_FULLSCALE == 2
#define AccAxis_Sensitive (float)(16384)
#elif ACC_FULLSCALE == 4
#define AccAxis_Sensitive (float)(8192)
#elif ACC_FULLSCALE == 8
#define AccAxis_Sensitive (float)(4096)
#elif ACC_FULLSCALE == 16
#define AccAxis_Sensitive (float)(2048)
#endif

#if GYRO_FULLSCALE == 250
#define GyroAxis_Sensitive (float)(131.0)
#elif GYRO_FULLSCALE == 500
#define GyroAxis_Sensitive (float)(65.5)
#elif GYRO_FULLSCALE == 1000
#define GyroAxis_Sensitive (float)(32.8)
#elif GYRO_FULLSCALE == 2000
#define GyroAxis_Sensitive (float)(16.4)
#endif

#else
#define AccAxis_Sensitive (1)
#define GyroAxis_Sensitive (1)
#endif

enum
{
    GIRO_CONFIG_250_DEG_S = 0x00,
    GIRO_CONFIG_500_DEG_S = 0x08,
    GIRO_CONFIG_1000_DEG_S = 0x10,
    GIRO_CONFIG_2000_DEG_S = 0x18,
} giro_config_e;

enum
{
    ACCEL_CONFIG_2_G = 0x00,
    ACCEL_CONFIG_4_G = 0x08,
    ACCEL_CONFIG_8_G = 0x10,
    ACCEL_CONFIG_16_G = 0x18,
} accel_config_e;

typedef enum
{
    MPU6050_HIGHPASS_DISABLE,
    MPU6050_HIGHPASS_5_HZ,
    MPU6050_HIGHPASS_2_5_HZ,
    MPU6050_HIGHPASS_1_25_HZ,
    MPU6050_HIGHPASS_0_63_HZ,
    MPU6050_HIGHPASS_UNUSED,
    MPU6050_HIGHPASS_HOLD,
} mpu6050_highpass_t;

MPU6050::MPU6050(gpio_num_t scl, gpio_num_t sda, i2c_port_t port)
{
    i2c = new I2C(scl, sda, port);
}

MPU6050::~MPU6050()
{
    delete (i2c);
}

bool MPU6050::init()
{
    if (!i2c->slave_write(MPU6050_ADDR, PWR_MGMT_1, 0x00))
    {
        return false;
    }
    if (!i2c->slave_write(MPU6050_ADDR, SMPLRT_DIV, 0x07))
    {
        return false;
    }
    if (!i2c->slave_write(MPU6050_ADDR, CONFIG, 0x07))
    {
        return false;
    }

    if (!i2c->slave_write(MPU6050_ADDR, GYRO_CONFIG, GIRO_CONFIG_2000_DEG_S))
    {
        return false;
    }
    if (!i2c->slave_write(MPU6050_ADDR, ACCEL_CONFIG, ACCEL_CONFIG_2_G + MPU6050_HIGHPASS_5_HZ))
    {
        return false;
    }

    // set threshold to 1
    if (!i2c->slave_write(MPU6050_ADDR, MPU6050_MOT_THR, 0x01))
    {
        return false;
    }

    // set detection duration to 20ms
    if (!i2c->slave_write(MPU6050_ADDR, MPU6050_MOT_DUR, 0x14))
    {
        return false;
    }

    uint8_t bufPinConf = 0;
    if (!i2c->slave_read(MPU6050_ADDR, INT_PIN_CFG, &bufPinConf, 1))
    {
        return false;
    }
    bufPinConf ^= 1 << 7;
    bufPinConf ^= 1 << 5;
    if (!i2c->slave_write(MPU6050_ADDR, INT_PIN_CFG, bufPinConf))
    {
        return false;
    }
    uint8_t bufPinIntr = 0;
    if (!i2c->slave_read(MPU6050_ADDR, INT_ENABLE, &bufPinIntr, 1))
    {
        return false;
    }
    bufPinIntr ^= 1 << 6;
    if (!i2c->slave_write(MPU6050_ADDR, INT_ENABLE, bufPinIntr))
    {
        return false;
    }
    return true;
}

bool MPU6050::setFastMotionCapture(bool enabled)
{
    if (enabled)
    {
        if (!i2c->slave_write(MPU6050_ADDR, GYRO_CONFIG, GIRO_CONFIG_250_DEG_S))
        {
            return false;
        }
        if (!i2c->slave_write(MPU6050_ADDR, ACCEL_CONFIG, ACCEL_CONFIG_2_G + MPU6050_HIGHPASS_DISABLE))
        {
            return false;
        }
    }
    else
    {
        if (!i2c->slave_write(MPU6050_ADDR, GYRO_CONFIG, GIRO_CONFIG_2000_DEG_S))
        {
            return false;
        }
        if (!i2c->slave_write(MPU6050_ADDR, ACCEL_CONFIG, ACCEL_CONFIG_2_G + MPU6050_HIGHPASS_5_HZ))
        {
            return false;
        }
    }
    return true;
}

float MPU6050::getAccX()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, ACCEL_XOUT_H, r, 2);
    short accx = r[0] << 8 | r[1];
    return (float)accx / AccAxis_Sensitive;
}

float MPU6050::getAccY()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, ACCEL_YOUT_H, r, 2);
    short accy = r[0] << 8 | r[1];
    return (float)accy / AccAxis_Sensitive;
}

float MPU6050::getAccZ()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, ACCEL_ZOUT_H, r, 2);
    short accz = r[0] << 8 | r[1];
    return (float)accz / AccAxis_Sensitive;
}

float MPU6050::getGyroX()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, GYRO_XOUT_H, r, 2);
    short gyrox = r[0] << 8 | r[1];
    return (float)gyrox / GyroAxis_Sensitive;
}

float MPU6050::getGyroY()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, GYRO_YOUT_H, r, 2);
    short gyroy = r[0] << 8 | r[1];
    return (float)gyroy / GyroAxis_Sensitive;
}

float MPU6050::getGyroZ()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, GYRO_ZOUT_H, r, 2);
    short gyroz = r[0] << 8 | r[1];
    return (float)gyroz / GyroAxis_Sensitive;
}

short MPU6050::getTemp()
{
    uint8_t r[0];
    i2c->slave_read(MPU6050_ADDR, TEMP_OUT_H, r, 2);
    return r[0] << 8 | r[1];
}

bool MPU6050::getInter()
{
    uint8_t r;
    i2c->slave_read(MPU6050_ADDR, INT_STATUS, &r, 1);
    // printf("uint : %d\n", r[0]);
    bool resp = (r >> 6) & 1;
    if (resp)
    {
        return true;
    }
    else
    {
        return false;
    }
}
