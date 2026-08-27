/*
 * mpu.h
 *
 *  Created on: Aug 23, 2026
 *      Author: mihal
 */

#ifndef INC_MPU_H_
#define INC_MPU_H_

#include <stdint.h>
#include "main.h"

typedef struct
{
	//data
    int16_t Ax_raw;
    int16_t Ay_raw;
    int16_t Az_raw;

    int16_t Gx_raw;
    int16_t Gy_raw;
    int16_t Gz_raw;

    int16_t Temp_raw;

    double Ax; 		// g
    double Ay;
    double Az;

    double Gx;      // deg/s
    double Gy;
    double Gz;

    float Temp_celsius;

    //config
    uint8_t sample_rate_div;
    uint8_t dlpf_cfg;
    uint8_t gyro_fs_sel;
    uint8_t accel_fs_sel;

    float accel_sensitivity;
    float gyro_sensitivity;
} MPU_t;

void mpu_get_dflt_cnfg(MPU_t* mpu_struct);

uint8_t mpu_init(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct);

uint8_t mpu_init_cnfg(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct);

HAL_StatusTypeDef mpu_readall(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct);

HAL_StatusTypeDef mpu_read_accel(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct);

HAL_StatusTypeDef mpu_read_gyro(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct);

HAL_StatusTypeDef mpu_read_temp(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct);

uint8_t mpu_enable_hardware_interrupts(I2C_HandleTypeDef* i2c);

HAL_StatusTypeDef mpu_disable_hardware_interrupts(I2C_HandleTypeDef* i2c);


#endif /* INC_MPU_H_ */
