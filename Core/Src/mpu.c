/*
 * mpu.c
 *
 *  Created on: Aug 23, 2026
 *      Author: mihal
 */

#include "mpu.h"

#define WHO_AM_I_REG 0x75
#define CONFIG_REG 0x1A
#define SMPRT_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACC_CONFIG_REG 0x1C
#define PWR_MGMT_REG 0x6B
#define ACC_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41
#define GYRO_XOUT_H_REG 0x43
#define INT_PIN_CFG 0x37
#define INT_EN_REG 0x38

//setup
#define MPU_ADDR 0xD0               //68 left shifted for hal i2c read
uint32_t i2c_timeout = 100;


/*
 * 	populates mpu struct with default config values
 */
void mpu_get_dflt_cnfg(MPU_t* mpu_struct) {
	mpu_struct->sample_rate_div = 7;
	mpu_struct->dlpf_cfg = 0;
	mpu_struct->accel_fs_sel = 0;
	mpu_struct->gyro_fs_sel = 0;
}


/*
 *  initializes mpu with default config values. after this function, the mpu is prepared for reading
 *
 *  returns:
 *  	0: okay
 *  	1: HAL i2c read failed
 *  	2: i2c identification failed
 */
uint8_t mpu_init(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct) {
	mpu_get_dflt_cnfg(mpu_struct);
	return mpu_init_cnfg(i2c, mpu_struct); 		//pass default configs to initializer
}


/*
 *  initializes mpu with custom config values. when using this function, all members of the struct should be initialized!
 *
 *
 *  explanation for each config parameter (for more details view the official mpu6050 datasheet -> registers SMPLRT_DIV, CONFIG, GYRO_CONFIG, ACCEL_CONFIG):
 *
 *  dlpf_cfg: 3bit unsigned value (0-7)  applies digital low pass filter. 0 and 7 = no filter.  1 - 6 -> cutoff lowers, bandwidth lowers, delay rises
 *
 *  sample_rate_div: 8bit unsigned value. determines sample rate:    sample rate = gyro output rate / (1 + sample_rate_div)      with dlpf_cfg at 0, gyro output rate is 8khz, so the default rate is 8/8 = 1khz
 *
 *  accel_fs_sel:   2bit unsigned value (0-3) full scale select for accelerometer. bigger scales -> less sensitivity and viceversa
 *  	full scale range based on input:
 *  	0 -> +- 2g
 *  	1 -> +- 4g
 *  	2 -> +- 8g
 *  	3 -> +- 16g
 *
 *  gyro_fs_sel:   2bit unsigned value (0-3) full scale select for gyroscope. bigger scales -> less sensitivity and viceversa
 *  	full scale range based on input:
 *  	0 -> +- 250 deg/s
 *  	1 -> +- 500 deg/s
 *  	2 -> +- 1000 deg/s
 *  	3 -> +- 2000 deg/s
 *
 *
 *  returns:
 *  	0: okay
 *  	1: HAL i2c read failed
 *  	2: HAL i2c write failed
 *  	3: i2c identification failed
 */
uint8_t mpu_init_cnfg(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct) {
	uint8_t reg;

  	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, MPU_ADDR, WHO_AM_I_REG, 1, &reg, 1, i2c_timeout);

  	if (status != HAL_OK) return 1;
  	if (reg != 0x68) return 3;

	reg = 0x00;
	if (HAL_I2C_Mem_Write(i2c, MPU_ADDR, PWR_MGMT_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 2; 	//0 for wakeup, no sleep or cycle mode

	reg = mpu_struct->sample_rate_div;
	if (HAL_I2C_Mem_Write(i2c, MPU_ADDR, SMPRT_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 2;

	if (HAL_I2C_Mem_Read(i2c, MPU_ADDR, CONFIG_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 1;
	if (mpu_struct->dlpf_cfg > 7) mpu_struct->dlpf_cfg = 0;
	reg &= ~0x7;
	reg |= (mpu_struct->dlpf_cfg);
	if (HAL_I2C_Mem_Write(i2c, MPU_ADDR, CONFIG_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 2;

	if (HAL_I2C_Mem_Read(i2c, MPU_ADDR, ACC_CONFIG_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 1;
	if (mpu_struct->accel_fs_sel > 3) mpu_struct->accel_fs_sel = 0;    //invalid -> set default
	reg &= ~(0x3 << 3);	                      //clear bits 3-4
	reg |= (mpu_struct->accel_fs_sel << 3);   //insert parameter into bits 3-4
	if (HAL_I2C_Mem_Write(i2c, MPU_ADDR, ACC_CONFIG_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 2;

	if (HAL_I2C_Mem_Read(i2c, MPU_ADDR, GYRO_CONFIG_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 1;
	if (mpu_struct->gyro_fs_sel > 3) mpu_struct->gyro_fs_sel = 0;    //invalid -> set default
	reg &= ~(0x3 << 3);	                      //clear bits 3-4
	reg |= (mpu_struct->gyro_fs_sel << 3);   //insert parameter into bits 3-4
	if (HAL_I2C_Mem_Write(i2c, MPU_ADDR, GYRO_CONFIG_REG, 1, &reg, 1, i2c_timeout) != HAL_OK) return 2;

	switch (mpu_struct->accel_fs_sel) {					//set proper sensitivity based on full scale selected
		case 0:
			mpu_struct->accel_sensitivity = 16384;
			break;
		case 1:
			mpu_struct->accel_sensitivity = 8192;
			break;
		case 2:
			mpu_struct->accel_sensitivity = 4096;
			break;
		case 3:
			mpu_struct->accel_sensitivity = 2048;
			break;
		default:
			mpu_struct->accel_sensitivity = 16384; 		//failsafe for misconfig
	}

	switch (mpu_struct->gyro_fs_sel) {
		case 0:
			mpu_struct->gyro_sensitivity = 131;
			break;
		case 1:
			mpu_struct->gyro_sensitivity = 65.5;
			break;
		case 2:
			mpu_struct->gyro_sensitivity = 32.8;
			break;
		case 3:
			mpu_struct->gyro_sensitivity = 16.4;
			break;
		default:
			mpu_struct->gyro_sensitivity = 131;		//failsafe for misconfig
	}

	return 0;
}


/*
 *  reads accel, gyro and temp (of mpu6050 chip) and populates the passed struct with values
 *
 *  returns:
 *  	status of HAL i2c mem read. in case of failure, returns before writing into struct.
 *  	to check if mem read succeeded, check if function returned HAL_OK
 */
HAL_StatusTypeDef mpu_readall(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct) {
	uint8_t data[14];

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, MPU_ADDR, ACC_XOUT_H_REG, 1, data, 14, i2c_timeout);  //acc xyz h/l + temp h/l + gyro xyz h/l  = 6 + 2 + 6 = 14 bytes of data

	if (status != HAL_OK) return status; 	//on fail, return prematurely

	mpu_struct->Ax_raw = (int16_t) (data[0] << 8 | data[1]);   //shift High into 8-15, OR to get Low into 0-7
	mpu_struct->Ay_raw = (int16_t) (data[2] << 8 | data[3]);
	mpu_struct->Az_raw = (int16_t) (data[4] << 8 | data[5]);

	mpu_struct->Temp_raw = (int16_t) (data[6] << 8 | data[7]);

	mpu_struct->Gx_raw = (int16_t) (data[8] << 8 | data[9]);
	mpu_struct->Gy_raw = (int16_t) (data[10] << 8 | data[11]);
	mpu_struct->Gz_raw = (int16_t) (data[12] << 8 | data[13]);

	mpu_struct->Temp_celsius = (float)(mpu_struct->Temp_raw / 340.0f) + 36.53f;

	mpu_struct->Ax = mpu_struct->Ax_raw / mpu_struct->accel_sensitivity;   //range*sens = int16 max bits  ->   raw/sens = val in units inside range     sensitivity: LSB/g  LSB/(LSB/g) = g
	mpu_struct->Ay = mpu_struct->Ay_raw / mpu_struct->accel_sensitivity;
	mpu_struct->Az = mpu_struct->Az_raw/ mpu_struct->accel_sensitivity;

	mpu_struct->Gx = mpu_struct->Gx_raw/ mpu_struct->gyro_sensitivity;
	mpu_struct->Gy = mpu_struct->Gy_raw / mpu_struct->gyro_sensitivity;
	mpu_struct->Gz = mpu_struct->Gz_raw / mpu_struct->gyro_sensitivity;

	return status;
}


/*
 *  reads accel and populates the passed struct with values
 *
 *  returns:
 *  	status of HAL i2c mem read. in case of failure, returns before writing into struct.
 *  	to check if mem read succeeded, check if function returned HAL_OK
 */
HAL_StatusTypeDef mpu_read_accel(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct) {
	uint8_t data[6];

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, MPU_ADDR, ACC_XOUT_H_REG, 1, data, 6, i2c_timeout);  //acc xyz h/l = 6 bytes of data

	if (status != HAL_OK) return status;

	mpu_struct->Ax_raw = (int16_t) (data[0] << 8 | data[1]);   //shift High into 8-15, OR to get Low into 0-7
	mpu_struct->Ay_raw = (int16_t) (data[2] << 8 | data[3]);
	mpu_struct->Az_raw = (int16_t) (data[4] << 8 | data[5]);

	mpu_struct->Ax = mpu_struct->Ax_raw / mpu_struct->accel_sensitivity;   //range*sens = int16 max bits  ->   raw/sens = val in units inside range     sensitivity: LSB/g  LSB/(LSB/g) = g
	mpu_struct->Ay = mpu_struct->Ay_raw / mpu_struct->accel_sensitivity;
	mpu_struct->Az = mpu_struct->Az_raw/ mpu_struct->accel_sensitivity;

	return status;
}


/*
 *  reads gyro and populates the passed struct with values
 *
 *  returns:
 *  	status of HAL i2c mem read. in case of failure, returns before writing into struct.
 *  	to check if mem read succeeded, check if function returned HAL_OK
 */
HAL_StatusTypeDef mpu_read_gyro(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct) {
	uint8_t data[6];

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, MPU_ADDR, GYRO_XOUT_H_REG, 1, data, 6, i2c_timeout);  //gyro xyz h/l = 6 bytes of data

	if (status != HAL_OK) return status;

	mpu_struct->Gx_raw = (int16_t) (data[0] << 8 | data[1]);   //shift High into 8-15, OR to get Low into 0-7
	mpu_struct->Gy_raw = (int16_t) (data[2] << 8 | data[3]);
	mpu_struct->Gz_raw = (int16_t) (data[4] << 8 | data[5]);

	mpu_struct->Gx = mpu_struct->Gx_raw / mpu_struct->gyro_sensitivity;   //range*sens = int16 max bits  ->   raw/sens = val in units inside range     sensitivity: LSB/g  LSB/(LSB/g) = g
	mpu_struct->Gy = mpu_struct->Gy_raw / mpu_struct->gyro_sensitivity;
	mpu_struct->Gz = mpu_struct->Gz_raw/ mpu_struct->gyro_sensitivity;

	return status;
}


/*
 *  reads temp (of mpu6050 chip) and populates the passed struct with values
 *
 *  returns:
 *  	status of HAL i2c mem read. in case of failure, returns before writing into struct.
 *  	to check if read mem read succeeded, check if function returned HAL_OK
 */
HAL_StatusTypeDef mpu_read_temp(I2C_HandleTypeDef* i2c, MPU_t* mpu_struct) {
	uint8_t data[2];

	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(i2c, MPU_ADDR, TEMP_OUT_H_REG, 1, data, 2, i2c_timeout);  //temp h/l = 2 bytes

	if (status != HAL_OK) return status;

	mpu_struct->Temp_raw = (int16_t) (data[0] << 8 | data[1]);
	mpu_struct->Temp_celsius = (float)(mpu_struct->Temp_raw / 340.0f) + 36.53f;

	return status;
}


/*
 *  enables the DATA_RDY hardware interrupt. with this enabled, mpu6050 generates and fires a hardware interrupt whenever data is ready, which allows for precise sampling.
 *  the interrupt can be caught with HAL_GPIO_EXTI_Callback.
 *  !IF CATCHING THIS INTERRUPT WITH AN INTERRUPT SERVICE ROUTINE, BEWARE THAT ALL READ FUNCTIONS HERE USE BLOCKING HAL_I2C_MEM_READ AND AS SUCH ARE NOT ISR SAFE.
 *   DO NOT CALL ANY READS (OR OTHER FUNCTIONS FROM THIS LIBRARY) DIRECTLY IN ISR CONTEXT!
 *
 *	returns:
 *		0: okay
 *		1: hal i2c mem write to INT PIN CFG failed
 *		2: hal i2c mem write to INT EN REG failed
 */
uint8_t mpu_enable_hardware_interrupts(I2C_HandleTypeDef* i2c) {
	uint8_t reg = 0x00;
	HAL_StatusTypeDef status = HAL_I2C_Mem_Write(i2c, MPU_ADDR, INT_PIN_CFG, 1, &reg, 1, i2c_timeout);   //register to 0: push-pull, active high (-> gpio_mode_it_rising on stm side)
	if (status != HAL_OK) return 1;

	reg = 0x01;                 //bit 0 to 1
	status = HAL_I2C_Mem_Write(i2c, MPU_ADDR, INT_EN_REG, 1, &reg, 1, i2c_timeout);  //DATA_RDY_EN
	if (status != HAL_OK) return 2;

	return 0;
}


/*
*  sets the INT_EN_REG to 0, which disables DATA RDY and other interrupts.
*
*  returns:
*  		status of HAL i2c mem read. to check if mem read succeeded, check if function returned HAL_OK
*/
HAL_StatusTypeDef mpu_disable_hardware_interrupts(I2C_HandleTypeDef* i2c) {
	uint8_t reg = 0x00;
	return HAL_I2C_Mem_Write(i2c, MPU_ADDR, INT_EN_REG, 1, &reg, 1, i2c_timeout);  //back to defaults (no interrupts)
}
