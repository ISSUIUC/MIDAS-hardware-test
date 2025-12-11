#include "stddef.h"

#include "Wire.h"
#include "lsm6dsv320x_reg.h"
#include "esp32-hal-i2c.h"

#define I2C_NUM 0
#define TIME_OUT_MS 25

class LSM6DSV320X {
public:
    
    // ????
    LSM6DSV320X() = default;

    void LSM6DSV320X::readAcceleration_HighG(float_t &x, float_t &y, float_t &z);

    void LSM6DSV320X::readAcceleration_LowG(float_t &x, float_t &y, float_t &z);

    void LSM6DSV320X::readAngularVelocity(float_t &xv, float_t &vy, float_t &vz);


    // are these necessary??
    static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len);

    static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len);

};