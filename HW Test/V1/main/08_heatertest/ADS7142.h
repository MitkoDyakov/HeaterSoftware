#ifndef __ADS7142__
#define __ADS7142__

#include <stdint.h>

#define ADS7142_I2C_ADDRESS                                         0x18

//***************************************************************************************************************************************************************************************************************************************
//
//  Command Opcodes
//
//***************************************************************************************************************************************************************************************************************************************

#define SINGLE_REG_READ                                             0x10        //Use for Single Register Read
#define SINGLE_REG_WRITE                                            0x08        //Use for Single Register Write
#define SET_BIT                                                     0x18        //Use for Single Bit Set
#define CLEAR_BIT                                                   0x20        //Use for Single Bit Clear
#define BLOCK_READ                                                  0x30        //Use for Block Register Read
#define BLOCK_WRITE                                                 0x28        //Use for Block Register Write

//***************************************************************************************************************************************************************************************************************************************
//
//  Register Map
//
//***************************************************************************************************************************************************************************************************************************************

/* Reset Registers */

#define ADS7142_REG_WKEY                                            0x17        //Write Key for writing into DEVICE_RESET Register
#define ADS7142_VAL_WKEY_RESET                                      0x00        //Write 00h to WKEY after device reset to prevent erroneous reset
#define ADS7142_VAL_WKEY_DEVICE_RESET                               0x0A        //Write 0Ah for write access to DEVICE_RESET

#define ADS7142_REG_DEVICE_RESET                                    0x14        //DEVICE_RESET Register
#define ADS7142_VAL_DEVICE_RESET_RESET                              0x01        //Write 01h to reset device

//***************************************************************************************************************************************************************************************************************************************

/* Functional Mode Select Registers */

#define ADS7142_REG_OFFSET_CAL                                      0x15        //Write to this register initiates internal offset calibration cycle, perform offset calibration periodically to nullify offset error
                                                                                //Write 01h to OFFSET_CAL to begin offset calibration

#define ADS7142_VAL_TRIG_OFFCAL                                     0x01        //Writing this mask to the OFFSET_CAL register triggers internal offset calibration

#define ADS7142_REG_OPMODE_SEL                                      0x1C        //Write to OPMODE_SEL to set the functional mode of the device

#define ADS7142_VAL_OPMODE_SEL_I2C_CMD_MODE_W_CHANNEL_0             0x00        //00h - Manual Mode with Channel 0 only
#define ADS7142_VAL_OPMODE_SEL_I2C_CMD_MODE_W_AUTO_SEQ_EN           0x04        //04h - Manual Mode with AUTO Sequencing enabled
#define ADS7142_VAL_OPMODE_SEL_AUTONOMOUS_MONITORING_MODE           0x06        //06h - Autonomous Monitoring Mode with AUTO Sequencing enabled
#define ADS7142_VAL_OPMODE_SEL_HIGH_PRECISION_MODE                  0x07        //07h - High Precision Mode with AUTO Sequencing enabled


#define ADS7142_REG_OPMODE_I2CMODE_STATUS                           0x00        //Read only register that provides the present operation mode and I2C mode configuration
                                                                                //00h - Device is not in High Speed I2C mode and is in Manual Mode
                                                                                //01h - Device is not in High Speed I2C mode and is in Autonomous Monitoring Mode
                                                                                //03h - Device is not in High Speed I2C mode and is in High Precision Mode (16-bit Resolution)
                                                                                //04h - Device is in High Speed I2C mode & Manual Mode
                                                                                //06h - Device is in High Speed I2C mode & Autonomous Monitoring Mode
                                                                                //07h - Device is in High Speed I2C mode & High Precision Mode (16-bit Resolution)

#define ADS7142_VAL_OPMODE_I2CMODE_HS_1                              0x08       //I2C High Speed Master code for high speed configuration (1.7MHz)
#define ADS7142_VAL_OPMODE_I2CMODE_HS_2                              0x0F       //I2C High Speed Master code for high speed configuration (3.4MHz)

//***************************************************************************************************************************************************************************************************************************************

/* Input Configuration Registers */

#define ADS7142_REG_CHANNEL_INPUT_CFG                               0x24        //Configures the analog input pins of the device

#define ADS7142_VAL_CHANNEL_INPUT_CFG_2_CHANNEL_SINGLE_ENDED        0x03        //Two-Channel, Single-Ended Configuration
#define ADS7142_VAL_CHANNEL_INPUT_CFG_1_CHANNEL_SINGLE_ENDED        0x01        //Single-Channel, Single-Ended Configuration
#define ADS7142_VAL_CHANNEL_INPUT_CFG_1_CHANNEL_PSEUDO_DIFF         0x02        //Single-Channel, Pseudo-Differential Configuration

//***************************************************************************************************************************************************************************************************************************************

/* Analog MUX and Sequencer Registers */

#define ADS7142_REG_AUTO_SEQ_CHEN                                   0x20        //This register selects the channels that are scanned when auto-sequencing is enabled
                                                                                //By default, both channels are selected at power up

#define ADS7142_VAL_AUTO_SEQ_CHENAUTO_SEQ_CH0                       0x01        //Scan Ch0 only when auto-sequencing is enabled

#define ADS7142_VAL_AUTO_SEQ_CHENAUTO_SEQ_CH1                       0x02        //Scan Ch1 only when auto-sequencing is enabled

#define ADS7142_VAL_AUTO_SEQ_CHENAUTO_SEQ_CH0_CH1                   0x03        //Scan both channels in auto-sequencing mode

#define ADS7142_REG_START_SEQUENCE                                  0x1E        //A write to this register starts the channel scanning sequence

#define ADS7142_VAL_START_SEQUENCE                                  0x01        //Sets the BUSY/~RDY pin high and starts the first conversion in the sequence

#define ADS7142_REG_ABORT_SEQUENCE                                  0x1F        //A write to this register aborts the channel scanning sequence
                                                                                /* Once sequence is aborted using this register, it is recommended to read the DATA_BUFFER_STATUS register to know the number of entries filled in the data buffer
                                                                                or ACCUMULATOR_STATUS register to know the number of accumulations finished before the abort */

#define ADS7142_VAL_ABORT_SEQUENCE                                  0x01        //Aborts the present sequence by driving the BUSY/~RDY pin low

#define ADS7142_REG_SEQUENCE_STATUS                                 0x04        //Read-only register that provides the status of sequence in the device
                                                                                //00h - Auto Sequencing disabled, no error
                                                                                //02h - Auto Sequencing enabled, no error
                                                                                //04h - Not Used
                                                                                //06h - Auto Sequencing enabled, device in error

//***************************************************************************************************************************************************************************************************************************************

/* Oscillator and Timing control Registers */

#define ADS7142_REG_OSC_SEL                                         0x18        //A write to this register selects the oscillator used for the conversion process
#define ADS7142_VAL_OSC_SEL_HSZ_LP                                  0x01        //A read or write of this value indicates use or enable of the Low Power Oscillator
#define ADS7142_VAL_OSC_SEL_HSZ_HSO                                 0x00        //A read or write of this value indicates use or enable of the High Speed Oscillator

#define ADS7142_REG_nCLK_SEL                                        0x19        //This register controls the cycle time for a single conversion by setting the nCLK parameter
                                                                                //nCLK is the number of clocks of the selected oscillator that the device uses for one conversion cycle
                                                                                /* When using the High Speed Oscillator: for a value x written into the nCLK register
                                                                                 * if x <= 21, nCLK is set to 21 (15h or 00010101b)
                                                                                 * if x > 21, nCLK is set to x
                                                                                 * When using the Low Power Oscillator: for a value x written into the nCLK register:
                                                                                 * if x <= 18, nCLK is set to 18 (12h or 00010010b)
                                                                                 * if x > 18, nCLK is set to x
                                                                                 */

//***************************************************************************************************************************************************************************************************************************************

/* Data Buffer Control Register */

#define ADS7142_REG_DATA_BUFFER_OPMODE                             0x2C         //Selects Data Buffer Operation Mode
#define ADS7142_VAL_DATA_BUFFER_STARTSTOP_CNTRL_STOPBURST          0x00         //Stop Burst Mode
#define ADS7142_VAL_DATA_BUFFER_STARTSTOP_CNTRL_STARTBURST         0x01         //Start Burst Mode, default
#define ADS7142_VAL_DATA_BUFFER_STARTSTOP_CNTRL_PREALERT           0x04         //Pre-Alert Data Mode
#define ADS7142_VAL_DATA_BUFFER_STARTSTOP_CNTRL_POSTALERT          0x06         //Post Alert Data Mode

/*Output Data Format Configuration*/

#define ADS7142_REG_DOUT_FORMAT_CFG                                 0x28         //This read/write register configures the data output format for the data buffer
#define ADS7142_VAL_DOUT_FORMAT_CFG_DOUT_FORMAT0                    0x00         //12-bit conversion result followed by 0000b or 0h
#define ADS7142_VAL_DOUT_FORMAT_CFG_DOUT_FORMAT1                    0x01         //12-bit conversion result followed by 3-bit Channel ID (000b for CH0, 001b for CH1)
#define ADS7142_VAL_DOUT_FORMAT_CFG_DOUT_FORMAT2                    0x02         //12-bit conversion result followed by 3-bit Channel ID (000b for CH0, 001b for CH1) followed by DATA_VALID bit
#define ADS7142_VAL_DOUT_FORMAT_CFG_DOUT_FORMAT3                    0x03         //12-bit conversion result followed by 0000b or 0h
#define ADS7142_REG_DATA_BUFFER_STATUS                              0x01         //Read-Only register that provides the number of entries filled in the data buffer until the last conversion
                                                                                 //Read will vary from 00h to 10h
                                                                                 //Bits 4-0 of this 8-bit register represent DATA_WORDCOUNT [00000] to [10000] = Number of entries filled in data buffer (0 to 16)

//***************************************************************************************************************************************************************************************************************************************

/*Accumulator Control Registers */

#define ADS7142_REG_ACC_EN                                          0x30         //Write to this register to enable the Accumulator
#define ADS7142_VAL_ACC_EN                                          0x0F         //Turns on the accumulator
#define ADS7142_REG_ACC_EN_OFF                                      0x00         //Turns off the accumulator
#define ADS7142_REG_ACC_CH0_LSB                                     0x08         //Provides the LSB of accumulated data for CH0
#define ADS7142_REG_ACC_CH0_MSB                                     0x09         //Provides the MSB of accumulated data for CH0
#define ADS7142_REG_ACC_CH1_LSB                                     0x0A         //Provides the LSB of accumulated data for CH1
#define ADS7142_REG_ACC_CH1_MSB                                     0x0B         //Provides the MSB of accumulated data for CH1
#define ADS7142_REG_ACCUMULATOR_STATUS                              0x02         /*Provides the present status of Accumulator (Read only)  Bits 3-0 of this register yield the number of accumulations completed until the last finished conversion*/

//***************************************************************************************************************************************************************************************************************************************

/*Digital Window Comparator Registers */

#define ADS7142_REG_ALERT_DWC_EN                                    0x37         //Write to this register enables the Alert and Digital Window Comparator block
#define ADS7142_VAL_ALERT_DWC_BLOCK_ENABLE                          0x01         //Enable the Digital Window Comparator
#define ADS7142_VAL_ALERT_DWC_BLOCK_DISABLE                         0x00         //Disable the Digital Window Comparator
#define ADS7142_REG_ALERT_CHEN                                      0x34         //A write to this register enables the alert functionality for individual channels
#define ADS7142_VAL_ALERT_EN_CH0                                    0x01         //Enables alerts on CH0
#define ADS7142_VAL_ALERT_EN_CH1                                    0x02         //Enables alerts on CH1
#define ADS7142_VAL_ALERT_EN_CH0_CH1                                0x03         //Enables alerts on CH0 and CH1
#define ADS7142_REG_DWC_HTH_CH0_MSB                                 0x39         //Sets the four most significant bits of high threshold for CH0
#define ADS7142_REG_DWC_HTH_CH0_LSB                                 0x38         //Sets the eight least significant bits of high threshold for CH0
#define ADS7142_REG_DWC_LTH_CH0_MSB                                 0x3B         //Sets the four most significant bits of low threshold for CH0
#define ADS7142_REG_DWC_LTH_CH0_LSB                                 0x3A         //Sets the eight least significant bits of high threshold for CH0
#define ADS7142_REG_DWC_HYS_CH0                                     0x40         //Sets the hysteresis for both comparators on CH0
#define ADS7142_REG_DWC_HTH_CH1_MSB                                 0x3D         //Sets the four most significant bits of high threshold for CH1
#define ADS7142_REG_DWC_HTH_CH1_LSB                                 0x3C         //Sets the eight least significant bits of high threshold for CH1
#define ADS7142_REG_DWC_LTH_CH1_MSB                                 0x3F         //Sets the four most significant bits of low threshold for CH1
#define ADS7142_REG_DWC_LTH_CH1_LSB                                 0x3E         //Sets the eight least significant bits of low threshold for CH1
#define ADS7142_REG_DWC_HYS_CH1                                     0x41         //Sets the hysteresis for both comparators for CH1
#define ADS7142_REG_PRE_ALERT_EVENT_COUNT                           0x36         //Sets the Pre-Alert Event Count for both high and low comparators, for both channels
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT1                          0x00         //Sets the Pre-Alert Event Count to 1
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT2                          0x10         //Sets the Pre-Alert Event Count to 2
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT3                          0x20         //Sets the Pre-Alert Event Count to 3
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT4                          0x30         //Sets the Pre-Alert Event Count to 4
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT5                          0x40         //Sets the Pre-Alert Event Count to 5
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT6                          0x50         //Sets the Pre-Alert Event Count to 6
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT7                          0x60         //Sets the Pre-Alert Event Count to 7
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT8                          0x70         //Sets the Pre-Alert Event Count to 8
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT9                          0x80         //Sets the Pre-Alert Event Count to 9
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT10                         0x90         //Sets the Pre-Alert Event Count to 10
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT11                         0xA0         //Sets the Pre-Alert Event Count to 11
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT12                         0xB0         //Sets the Pre-Alert Event Count to 12
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT13                         0xC0         //Sets the Pre-Alert Event Count to 13
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT14                         0xD0         //Sets the Pre-Alert Event Count to 14
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT15                         0xE0         //Sets the Pre-Alert Event Count to 15
#define ADS7142_VAL_PRE_ALERT_EVENT_COUNT16                         0xF0         //Sets the Pre-Alert Event Count to 16
#define ADS7142_REG_ALERT_TRIG_CH_ID                                0x03         //Provides the channel ID of the channel which was first to set the alert output (Read only)
                                                                                 //00h ---> Channel 0
                                                                                 //10h ---> Channel 1
#define ADS7142_REG_ALERT_LOW_FLAGS                                 0x0C         //Provides the read/write status of latched flags for low alert on both channels
#define ADS7142_REG_ALERT_HIGH_FLAGS                                0x0E         //Provides the read/write status of latched flags for high alert on both channels

#endif