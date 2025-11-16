#ifndef PINOUT_H
#define PINOUT_H

// sound buzzer pin definition
 #define BUZZER    21

// I2C and USB-PD and ADC pin definitions
 #define I2C_SCL   46
 #define I2C_SDA   9
 #define PD_INT    3
 #define ADC_ALERT 5
 #define ADC_RDY   6 

// Display pin definitions
#define DISPLAY_BACKLIGHT 14
#define DISPLAY_RS        38
#define DISPLAY_RST       39
#define DISPLAY_CS        36
#define DISPLAY_SCK       12
#define DISPLAY_MOSI      11
#define DISPLAY_TILT      8
 
// Heater control pin definitions
#define HEATER_CHANNEL_1  7
#define HEATER_CHANNEL_2  15

// Fan control pin definitions
#define ENABLE_FAN_VDD       13
#define ENABLE_FAN_CHANNEL_1 4
#define ENABLE_FAN_CHANNEL_2 16

// User button pin definitions
#define NUM_BUTTONS            6

#define BUTTON_RIGHT_TOP       41
#define BUTTON_RIGHT_CENTER    42
#define BUTTON_RIGHT_BOTTOM    40
#define BUTTON_LEFT_TOP        48
#define BUTTON_LEFT_CENTER     47
#define BUTTON_LEFT_BOTTOM     35

#endif // PINOUT_H