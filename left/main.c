#include "board.h"
#include "fsl_clock_manager.h"
#include "fsl_i2c_slave_driver.h"
#include "fsl_i2c_shared_function.h"
#include "i2c.h"
#include "main.h"

uint8_t isSw2Pressed;
uint8_t isSw3Pressed;
uint8_t buffer[I2C_DATA_LENGTH];

void I2C0_IRQHandler(void)
{
    I2C_DRV_IRQHandler(I2C0_IDX);
}

static void i2c_slave_callback(uint8_t instance, i2c_slave_event_t i2cEvent, void *param)
{
    i2c_slave_state_t *slaveState = I2C_DRV_SlaveGetHandler(instance);

    switch (i2cEvent) {
        case kI2CSlaveTxReq:
            slaveState->txSize = I2C_DATA_LENGTH;
            slaveState ->txBuff = buffer;
            slaveState ->isTxBusy = true;
            buffer[0] = isSw2Pressed;
            buffer[1] = isSw3Pressed;
            break;

        case kI2CSlaveTxEmpty:
            slaveState->isTxBusy = false;
            break;

        default:
            break;
    }
}

int main(void)
{
    // Initialize clock.

    CLOCK_SYS_EnablePortClock(PORTA_IDX);
    CLOCK_SYS_EnablePortClock(PORTB_IDX);

    BOARD_ClockInit();

    // Initialize GPIO.

    gpio_input_pin_user_config_t inputPin[] =
    {
        {
            .pinName                       = kGpioSW2,
            .config.isPullEnable           = true,
            .config.pullSelect             = kPortPullUp,
            .config.isPassiveFilterEnabled = false,
            .config.interrupt              = kPortIntDisabled,
        },
        {
            .pinName                       = kGpioSW3,
            .config.isPullEnable           = true,
            .config.pullSelect             = kPortPullUp,
            .config.isPassiveFilterEnabled = false,
            .config.interrupt              = kPortIntDisabled,
        },
        {
            .pinName = GPIO_PINS_OUT_OF_RANGE,
        }
    };

    gpio_output_pin_user_config_t outputPin[] =
    {
        {
            .pinName              = kGpioLED1,
            .config.outputLogic   = 0,
            .config.slewRate      = kPortFastSlewRate,
            .config.driveStrength = kPortHighDriveStrength,
        },
        {
            .pinName              = kGpioLED3,
            .config.outputLogic   = 0,
            .config.slewRate      = kPortFastSlewRate,
            .config.driveStrength = kPortHighDriveStrength,
        },
        {
            .pinName = GPIO_PINS_OUT_OF_RANGE,
        }
    };

    GPIO_DRV_Init(inputPin, outputPin);

    // Initialize I2C.

    i2c_slave_state_t slave;

    i2c_slave_user_config_t userConfig = {
        .address         = LEFT_KEYBOARD_HALF_I2C_ADDRESS_7BIT,
        .slaveCallback   = i2c_slave_callback,
        .callbackParam   = NULL,
        .slaveListening  = true,
        .startStopDetect = true,
    };

    configure_i2c_pins(0);
    I2C_DRV_SlaveInit(BOARD_I2C_INSTANCE, &userConfig, &slave);

    // Update switch states and toggle LEDs accordingly.

    while (1) {
        isSw2Pressed = GPIO_DRV_ReadPinInput(kGpioSW2);
        isSw3Pressed = GPIO_DRV_ReadPinInput(kGpioSW3);
        GPIO_DRV_WritePinOutput(kGpioLED1, isSw2Pressed);
        GPIO_DRV_WritePinOutput(kGpioLED3, isSw3Pressed);
    }
}
