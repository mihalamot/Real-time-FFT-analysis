# MPU6050 Interrupt-Driven Driver — STM32F446RE

Register-level MPU-6050 accelerometer/gyroscope driver for STM32, built around the sensor's INT pin instead of software polling. The MPU-6050 fires a hardware interrupt on the STM32 the instant new data is ready, giving precise, jitter-free sample timing — built for feeding downstream signal processing (e.g. FFT) where consistent sample rate matters.

## Hardware
- STM32 Nucleo F446RE
- MPU-6050 accelerometer/gyroscope (INT pin wired to an EXTI-capable GPIO)

## Concepts
- I2C communication (MPU-6050 register read/write)
- EXTI / hardware interrupt-driven sampling (data-ready on INT pin, no polling)
- FreeRTOS semaphore-based ISR-to-task signaling
- Bitfield register configuration (DLPF, full-scale range, sample rate divider)

## Third party
- ST HAL drivers (ST License)
- FreeRTOS (CMSIS-RTOS2 API)

## Features
- Configurable sample rate, DLPF, accel/gyro full-scale range (or use built-in defaults)
- Full read (`readall`) or targeted reads (accel-only, gyro-only, temp-only)
- Hardware interrupt enable/disable, decoupled from init — pair with an EXTI callback + semaphore for interrupt-driven sampling
- Per-function status codes (HAL passthrough or custom, documented per function) — no silently ignored I2C failures

## Known limitations
- Severe physical shock/vibration can occasionally cause an I2C bus lockup (SDA/SCL held low by the sensor). MCU reset alone will not clear it — this is a known I2C failure mode, not specific to this driver. Bus recovery (GPIO bit-bang SCL pulse sequence) is not yet implemented; a full power cycle currently clears it.
- Read functions use blocking `HAL_I2C_Mem_Read` — do not call them directly from ISR context. Use the interrupt-enable function together with a semaphore released from your `HAL_GPIO_EXTI_Callback`, and do the actual read in a task/thread.

## Usage examples

**Basic — init with defaults, poll-read in a loop:**
```c
MPU_t mpu;
mpu_init(&hi2c1, &mpu);          // WHO_AM_I check + default config (1kHz, DLPF off, ±2g, ±250dps)

while (1) {
    if (mpu_readall(&hi2c1, &mpu) == HAL_OK) {
        // mpu.Ax, mpu.Ay, mpu.Az, mpu.Gx, mpu.Gy, mpu.Gz, mpu.Temp_celsius
    }
}
```

**Custom config:**
```c
MPU_t mpu;
mpu_get_dflt_cnfg(&mpu);         // populate struct with defaults
mpu.accel_fs_sel = 2;            // override just what you need (here: ±8g)
mpu.sample_rate_div = 3;         // 8kHz / (1+3) = 2kHz

mpu_init_cnfg(&hi2c1, &mpu);
```

**Interrupt-driven sampling (recommended for consistent sample timing):**
```c
// after mpu_init(...):
mpu_enable_hardware_interrupts(&hi2c1);

// in HAL_GPIO_EXTI_Callback, ISR-safe only:
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == MPU_INT_Pin) {
        osSemaphoreRelease(dataReadySemHandle);
    }
}

// in a FreeRTOS task:
for (;;) {
    osSemaphoreAcquire(dataReadySemHandle, osWaitForever);
    mpu_read_accel(&hi2c1, &mpu);   // do the actual I2C read here, not in the ISR
}
```

**Targeted reads (lighter than full `readall`):**
```c
mpu_read_accel(&hi2c1, &mpu);   // accel only
mpu_read_gyro(&hi2c1, &mpu);    // gyro only
mpu_read_temp(&hi2c1, &mpu);    // chip temp only
```
