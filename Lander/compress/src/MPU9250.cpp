#include <math.h>
#include "i2c.h"
#include "MPU9250.h"

int MPU9250::start() {
    if(read8(address, MPU9250_ID_REG) != MPU9250_ID)
        return 1;
    write8(address, MPU9250_INT_PIN_CFG, 0x22);
    write8(address, MPU9250_INT_ENABLE, 0x01);  // Enable data ready (bit 0) interrupt
    return mageno.start();
}

int MPU9250::readAccelometerData(int16_t *output) {
    uint8_t buffer[6];
    readBuffer(address, MPU9250_ACCEL_REG, buffer, 6);
    output[0] = ((uint16_t)buffer[0] << 8) | (uint16_t)buffer[1];
    output[1] = ((uint16_t)buffer[2] << 8) | (uint16_t)buffer[3];
    output[2] = ((uint16_t)buffer[4] << 8) | (uint16_t)buffer[5];
    return 0;
}

int MPU9250::readGyrometerData(int16_t *output) {
    uint8_t buffer[6];
    readBuffer(address, MPU9250_GYRO_REG, buffer, 6);
    output[0] = ((uint16_t)buffer[0] << 8) | (uint16_t)buffer[1];
    output[1] = ((uint16_t)buffer[2] << 8) | (uint16_t)buffer[3];
    output[2] = ((uint16_t)buffer[4] << 8) | (uint16_t)buffer[5];
    return 0;
}

int MPU9250::readMagneoometerData(int16_t *output) {
    return mageno.read(output);
}

//Madgwick's Algorithm
void MPU9250::Quaternion(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz) {
    float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];   // short name local variable for readability
    float norm;
    float hx, hy, _2bx, _2bz;
    float s1, s2, s3, s4;
    float qDot1, qDot2, qDot3, qDot4;

    // Auxiliary variables to avoid repeated arithmetic
    float _2q1mx;
    float _2q1my;
    float _2q1mz;
    float _2q2mx;
    float _4bx;
    float _4bz;
    float _2q1 = 2.0f * q1;
    float _2q2 = 2.0f * q2;
    float _2q3 = 2.0f * q3;
    float _2q4 = 2.0f * q4;
    float _2q1q3 = 2.0f * q1 * q3;
    float _2q3q4 = 2.0f * q3 * q4;
    float q1q1 = q1 * q1;
    float q1q2 = q1 * q2;
    float q1q3 = q1 * q3;
    float q1q4 = q1 * q4;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q2q4 = q2 * q4;
    float q3q3 = q3 * q3;
    float q3q4 = q3 * q4;
    float q4q4 = q4 * q4;

    // Normalise accelerometer measurement
    norm = sqrt(ax * ax + ay * ay + az * az);
    if (norm == 0.0f) return; // handle NaN
    norm = 1.0f/norm;
    ax *= norm;
    ay *= norm;
    az *= norm;

    // Normalise magnetometer measurement
    norm = sqrt(mx * mx + my * my + mz * mz);
    if (norm == 0.0f) return; // handle NaN
    norm = 1.0f/norm;
    mx *= norm;
    my *= norm;
    mz *= norm;

    // Reference direction of Earth's magnetic field
    _2q1mx = 2.0f * q1 * mx;
    _2q1my = 2.0f * q1 * my;
    _2q1mz = 2.0f * q1 * mz;
    _2q2mx = 2.0f * q2 * mx;
    hx = mx * q1q1 - _2q1my * q4 + _2q1mz * q3 + mx * q2q2 + _2q2 * my * q3 + _2q2 * mz * q4 - mx * q3q3 - mx * q4q4;
    hy = _2q1mx * q4 + my * q1q1 - _2q1mz * q2 + _2q2mx * q3 - my * q2q2 + my * q3q3 + _2q3 * mz * q4 - my * q4q4;
    _2bx = sqrt(hx * hx + hy * hy);
    _2bz = -_2q1mx * q3 + _2q1my * q2 + mz * q1q1 + _2q2mx * q4 - mz * q2q2 + _2q3 * my * q4 - mz * q3q3 + mz * q4q4;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;

    // Gradient decent algorithm corrective step
    s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - ax) + _2q2 * (2.0f * q1q2 + _2q3q4 - ay) - _2bz * q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q4 + _2bz * q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - ax) + _2q1 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q2 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + _2bz * q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q3 + _2bz * q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q4 - _4bz * q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - ax) + _2q4 * (2.0f * q1q2 + _2q3q4 - ay) - 4.0f * q3 * (1.0f - 2.0f * q2q2 - 2.0f * q3q3 - az) + (-_4bx * q3 - _2bz * q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (_2bx * q2 + _2bz * q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + (_2bx * q1 - _4bz * q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - ax) + _2q3 * (2.0f * q1q2 + _2q3q4 - ay) + (-_4bx * q4 + _2bz * q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - mx) + (-_2bx * q1 + _2bz * q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - my) + _2bx * q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - mz);
    norm = sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);    // normalise step magnitude
    norm = 1.0f/norm;
    s1 *= norm;
    s2 *= norm;
    s3 *= norm;
    s4 *= norm;

    // Compute rate of change of quaternion
    qDot1 = 0.5f * (-q2 * gx - q3 * gy - q4 * gz) - beta * s1;
    qDot2 = 0.5f * (q1 * gx + q3 * gz - q4 * gy) - beta * s2;
    qDot3 = 0.5f * (q1 * gy - q2 * gz + q4 * gx) - beta * s3;
    qDot4 = 0.5f * (q1 * gz + q2 * gy - q3 * gx) - beta * s4;

    // Integrate to yield quaternion
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;
    q4 += qDot4 * dt;
    norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
    norm = 1.0f/norm;
    q[0] = q1 * norm;
    q[1] = q2 * norm;
    q[2] = q3 * norm;
    q[3] = q4 * norm;
}

//TODO error handling
int MPU9250::getQuaternion(float *output) {
    int16_t gyro[3], acc[3], magno[3];
    readGyrometerData(gyro);
    readAccelometerData(acc);
    readMagneoometerData(magno);
    Quaternion(acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2], magno[0], magno[1], magno[2]);
    output[0] = q[0];
    output[1] = q[1];
    output[2] = q[2];
    output[3] = q[3];
    return 0;
}

int MPU9250::getQuaternion(float *output, int16_t gyro[], int16_t acc[], int16_t magno[]) {
    Quaternion(acc[0], acc[1], acc[2], gyro[0], gyro[1], gyro[2], magno[0], magno[1], magno[2]);
    output[0] = q[0];
    output[1] = q[1];
    output[2] = q[2];
    output[3] = q[3];
    return 0;
}

int AK8963::read(int16_t *output) {
    uint8_t buffer[7];
    readBuffer(address, AK8963_DATA_REG, buffer, 7);
    output[0] = ((uint16_t)buffer[1] << 8) | (uint16_t)buffer[0];
    output[1] = ((uint16_t)buffer[3] << 8) | (uint16_t)buffer[2];
    output[2] = ((uint16_t)buffer[5] << 8) | (uint16_t)buffer[4];
    uint8_t status = buffer[6];
    if(status & 0x08)
        return 2; //Overflow
    return 0;
}

int AK8963::start() {
    if(read8(address, AK8963_ID_REG) != AK8963_ID)
        return 1;
    write8(address, AK8963_CTRL1_REG, 0x06); // continus measurement mode, 0x02 for 8Hz, 0x06 for 100Hz
    return 0;
}
