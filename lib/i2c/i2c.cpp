#include <Wire.h>
#include <Arduino.h>

#define I2C_WRITE_BIT 0
#define I2C_READ_BIT 1

#define I2C_SLEEP_TIME 2

#define I2C_SDA_PIN 21
#define I2C_SDA_MUX PERIPHS_IO_MUX_GPIO21_U
#define I2C_SDA_FUNC FUNC_GPIO21

#define I2C_SCK_PIN 22
#define I2C_SCK_MUX PERIPHS_IO_MUX_GPIO22_U
#define I2C_SCK_FUNC FUNC_GPIO22

void i2c_sda(uint8_t state) {
    state &= 0x01;
    if (state)
        digitalWrite(I2C_SDA_PIN, HIGH);
    else
        digitalWrite(I2C_SDA_PIN, LOW);
}

void i2c_sck(uint8_t state) {
    if (state)
        digitalWrite(I2C_SCK_PIN, HIGH);
    else
        digitalWrite(I2C_SCK_PIN, LOW);
}

void i2c_init(void) {
    Wire.begin(I2C_SDA_PIN, I2C_SCK_PIN);
}

void i2c_start(void) {
    i2c_sda(1);
    i2c_sck(1);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sda(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);
}

void i2c_stop(void) {
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(1);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sda(1);
    delayMicroseconds(I2C_SLEEP_TIME);
}

void i2c_send_ack(uint8_t state) {
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sda(state ? 0 : 1);
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(1);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sda(1);
    delayMicroseconds(I2C_SLEEP_TIME);
}

uint8_t i2c_check_ack(void) {
    uint8_t ack;
    i2c_sda(1);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(1);
    delayMicroseconds(I2C_SLEEP_TIME);
    ack = digitalRead(I2C_SDA_PIN);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    i2c_sda(0);
    delayMicroseconds(I2C_SLEEP_TIME);
    return (ack ? 0 : 1);
}

uint8_t i2c_readByte(void) {
    uint8_t data = 0;
    uint8_t data_bit;
    uint8_t i;

    i2c_sda(1);

    for (i = 0; i < 8; i++) {
        delayMicroseconds(I2C_SLEEP_TIME);
        i2c_sck(0);
        delayMicroseconds(I2C_SLEEP_TIME);
        i2c_sck(1);
        delayMicroseconds(I2C_SLEEP_TIME);
        data_bit = digitalRead(I2C_SDA_PIN);
        delayMicroseconds(I2C_SLEEP_TIME);
        data_bit <<= (7 - i);
        data |= data_bit;
    }
    i2c_sck(0);
    delayMicroseconds(I2C_SLEEP_TIME);

    return data;
}

void i2c_writeByte(uint8_t data) {
    uint8_t data_bit;
    int8_t i;

    delayMicroseconds(I2C_SLEEP_TIME);

    for (i = 7; i >= 0; i--) {
        data_bit = data >> i;
        i2c_sda(data_bit);
        delayMicroseconds(I2C_SLEEP_TIME);
        i2c_sck(1);
        delayMicroseconds(I2C_SLEEP_TIME);
        i2c_sck(0);
        delayMicroseconds(I2C_SLEEP_TIME);
    }
}

int i2c_writeBytes(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t const *data) {
    Wire.beginTransmission(slave_addr);
    Wire.write(reg_addr);
    for (int i = 0; i < length; ++i) {
        Wire.write(data[i]);
    }
    int result = Wire.endTransmission();
    if (result != 0) {
        Serial.printf("i2c_writeBytes: transmission error %d\n", result);
    }
    return result;
}

int i2c_readBytes(uint8_t slave_addr, uint8_t reg_addr, uint8_t length, uint8_t* data) {
    Wire.beginTransmission(slave_addr);
    Wire.write(reg_addr);
    int result = Wire.endTransmission(false);
    if (result != 0) {
        Serial.printf("i2c_readBytes: transmission error %d\n", result);
        return result;
    }
    Wire.requestFrom(slave_addr, length);
    for (int i = 0; i < length; ++i) {
        if (Wire.available()) {
            data[i] = Wire.read();
        } else {
            Serial.println("i2c_readBytes: data not available");
            return -1;
        }
    }
    return 0;
}
