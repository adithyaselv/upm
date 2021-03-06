/*
 * Author: Yevgeniy Kiveisha <yevgeniy.kiveisha@intel.com>
 * Copyright (c) 2014 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <unistd.h>
#include <stdlib.h>

#include "mlx90614.h"

using namespace upm;

struct MLX90614Exception : public std::exception {
    std::string message;
    MLX90614Exception (std::string msg) : message (msg) { }
    ~MLX90614Exception () throw () { }
    const char* what() const throw () { return message.c_str(); }
};

MLX90614::MLX90614 (int bus, int devAddr) {
    m_name = "MLX90614";

    m_i2cAddr = devAddr;
    m_bus = bus;

    m_i2Ctx = mraa_i2c_init(m_bus);

    mraa_result_t ret = mraa_i2c_address(m_i2Ctx, m_i2cAddr);
    if (ret != MRAA_SUCCESS) {
        throw MLX90614Exception ("Couldn't initilize I2C.");
    }
}

MLX90614::~MLX90614() {
    mraa_i2c_stop(m_i2Ctx);
}

float
MLX90614::readObjectTempF(void) {
    return (readTemperature (MLX90614_TOBJ1) * 9 / 5) + 32;
}

float
MLX90614::readAmbientTempF(void) {
    return (readTemperature (MLX90614_TA) * 9 / 5) + 32;
}

float
MLX90614::readObjectTempC(void) {
    return readTemperature (MLX90614_TOBJ1);
}

float
MLX90614::readAmbientTempC(void) {
    return readTemperature (MLX90614_TA);
}

/*
 * **************
 *  private area
 * **************
 */
uint16_t
MLX90614::i2cReadReg_N (int reg, unsigned int len, uint8_t * buffer) {
    int readByte = 0;

    if (m_i2Ctx == NULL) {
        throw MLX90614Exception ("Couldn't find initilized I2C.");
    }

    mraa_i2c_address(m_i2Ctx, m_i2cAddr);
    mraa_i2c_write_byte(m_i2Ctx, reg);

    readByte = mraa_i2c_read(m_i2Ctx, buffer, len);
    return readByte;
}

mraa_result_t
MLX90614::i2cWriteReg_N (uint8_t reg, unsigned int len, uint8_t * buffer) {
    mraa_result_t error = MRAA_SUCCESS;

    if (m_i2Ctx == NULL) {
        throw MLX90614Exception ("Couldn't find initilized I2C.");
    }

    error = mraa_i2c_address (m_i2Ctx, m_i2cAddr);
    error = mraa_i2c_write (m_i2Ctx, buffer, len);

    return error;
}

float
MLX90614::readTemperature (uint8_t address) {
    uint8_t     buffer[3];
    float       temperature = 0;

    /*  Reading temperature from sensor.
        Answer contained of 3 bytes (TEMP_LSB | TEMP_MSB | PEC)
     */
    if (i2cReadReg_N (address, 3, buffer) > 2) {
        temperature = buffer[0];
        temperature = buffer[1] << 8;

        temperature *= .02;
        temperature -= 273.15;
    }

    return temperature;
}
