#ifndef __I2C_H__
#define __I2C_H__

#include <Wire.h>

#define I2C_SLEEP_TIME 5

// SDA on GPIO21
#define I2C_SDA_MUX PERIPHS_IO_MUX_GPIO21_U
#define I2C_SDA_FUNC FUNC_GPIO21
#define I2C_SDA_PIN 6

// SCK on GPIO22
#define I2C_SCK_MUX PERIPHS_IO_MUX_GPIO22_U
#define I2C_SCK_FUNC FUNC_GPIO22
#define I2C_SCK_PIN 7

#define esp_i2c_read() digitalRead(I2C_SDA_PIN)

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_send_ack(uint8_t state);
uint8_t i2c_check_ack(void);
uint8_t i2c_readByte(void);
void i2c_writeByte(uint8_t data);

int i2c_readBytes(uint8_t slave_addr, uint8_t reg_addr,
                  uint8_t length, uint8_t* data);

int i2c_writeBytes(uint8_t slave_addr, uint8_t reg_addr,
                   uint8_t length, uint8_t const *data);

#endif
