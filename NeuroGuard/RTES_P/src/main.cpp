/* 
RTES FINAL PROJECT
TEAM MEMBERS:
Yugesh Panta (yp2651)
Rithvik Peeriga (rp4082)
Srujana Kanchisamudram Seshagiribabu (sk11115)
*/

#include <mbed.h>
#include "math.h"
#include "drivers/LCD_DISCO_F429ZI.h"
#include "iostream"
#include "stdio.h"
#include <cstdio>
#include <sstream>
#include <iomanip>

#define WINDOW_SIZE 15

// Define Gyroscope settings...
#define CTRL_REG1 0x20
#define CTRL_REG1_CONFIG 0b01'10'1'1'1'1
#define CTRL_REG4 0x23 // Second configure to set the DPS
#define CTRL_REG4_CONFIG 0b0'0'01'0'00'0
#define CTRL_REG3 0x22 
#define CTRL_REG3_CONFIG 0b0'0'0'0'1'000
#define OUT_X_L 0x28

#define SPI_FLAG 1
#define DATA_READY_FLAG 2
#define BUTTON_PRESSED_FLAG 4

#define SCALING_FACTOR (17.5f * 0.01745f / 1000.0f)

#define FILTER_COEFFICIENT 0.1f // Adjust this value as needed

Ticker timer;
int countSecs = 0;
int cycles = 0;

// EventFlags object declaration
EventFlags flags;

// DigitalIn button(PA_0);
InterruptIn button(BUTTON1);

// Serial Port
BufferedSerial serial_port(USBTX, USBRX, 9600);

// Override writing to console to write to serial port to handle writing to file
FileHandle *mbed::mbed_override_console(int fd) {
    return &serial_port;
}

void timer_cb() {
    countSecs++;
}

// spi callback function
void spi_cb(int event) {
    flags.set(SPI_FLAG);
}

// data ready callback function
void data_cb() {
    flags.set(DATA_READY_FLAG);
}

// button pushed callback function
void button_pushed_cb(){
    flags.set(BUTTON_PRESSED_FLAG);
}


int main() {
        LCD_DISCO_F429ZI LCD;

    //spi initialization
    SPI spi(PF_9, PF_8, PF_7, PC_1, use_gpio_ssel);
    uint8_t write_buf[32], read_buf[32];

    //interrupt initialization
    InterruptIn int2(PA_2, PullDown);
    int2.rise(&data_cb);

    //button setup
    button.rise(&button_pushed_cb);
    
    //spi format and frequency
    spi.format(8, 3);
    spi.frequency(1'000'000);

    // Write to control registers --> spi transfer
    write_buf[0] = CTRL_REG1;
    write_buf[1] = CTRL_REG1_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG4;
    write_buf[1] = CTRL_REG4_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[0] = CTRL_REG3;
    write_buf[1] = CTRL_REG3_CONFIG;
    spi.transfer(write_buf, 2, read_buf, 2, spi_cb);
    flags.wait_all(SPI_FLAG);

    write_buf[1] = 0xFF;

    //(polling for\setting) data ready flag
    if (!(flags.get() & DATA_READY_FLAG) && (int2.read() == 1)) {
        flags.set(DATA_READY_FLAG);
    }
    DigitalOut led(LED1); // Define LED pin

    // Initialize buffers to store raw values

    led.write(0);
    int16_t raw_gx_values[WINDOW_SIZE] = {0};
    int16_t raw_gy_values[WINDOW_SIZE] = {0};
    int16_t raw_gz_values[WINDOW_SIZE] = {0};

    // Start showing on LCD Screen

    LCD.Clear(LCD_COLOR_ORANGE);
    LCD.SetBackColor(LCD_COLOR_ORANGE);
    LCD.SetTextColor(LCD_COLOR_BLACK);
    LCD.SetFont(&Font16);

    LCD.DisplayStringAt(0, LINE(2), (uint8_t *)"RTES Final Project", CENTER_MODE);
    LCD.DisplayStringAt(0, LINE(4), (uint8_t *)"By", CENTER_MODE);
    LCD.DisplayStringAt(0, LINE(6), (uint8_t *)"Yugesh", CENTER_MODE); 
    LCD.DisplayStringAt(0, LINE(7), (uint8_t *)"Rithvik", CENTER_MODE); 
    LCD.DisplayStringAt(0, LINE(8), (uint8_t *)"Srujana", CENTER_MODE); 

    ThisThread::sleep_for(5s);
    // Main loop
    while (1) {
        LCD.Clear(LCD_COLOR_BLACK);
        LCD.DisplayStringAt(0, LINE(5), (uint8_t *)"Timer starts", CENTER_MODE);
        // Capture gyro data for 20 seconds
        for (int i = 0; i < WINDOW_SIZE; ++i) {
            flags.wait_all(DATA_READY_FLAG);
            
            // Read gyro data
            uint8_t write_buf[7], read_buf[7];
            int16_t raw_gx, raw_gy, raw_gz;

            write_buf[0] = OUT_X_L | 0x80 | 0x40;
            spi.transfer(write_buf, 7, read_buf, 7, spi_cb);
            flags.wait_all(SPI_FLAG);

            // Process raw data
            raw_gx = (((uint16_t)read_buf[2]) << 8) | ((uint16_t)read_buf[1]);
            raw_gy = (((uint16_t)read_buf[4]) << 8) | ((uint16_t)read_buf[3]);
            raw_gz = (((uint16_t)read_buf[6]) << 8) | ((uint16_t)read_buf[5]);

            // Store raw gyro values in arrays
            raw_gx_values[i] = raw_gx;
            raw_gy_values[i] = raw_gy;
            raw_gz_values[i] = raw_gz;
            
            // Delay before reading next sample
            ThisThread::sleep_for(500ms);
        }

       
        // Initialize average gyro values
        int16_t avg_raw_gx = 0, avg_raw_gy = 0, avg_raw_gz = 0;

        for (int i = 0; i < WINDOW_SIZE; ++i) {
            avg_raw_gx += raw_gx_values[i];
            avg_raw_gy += raw_gy_values[i];
            avg_raw_gz += raw_gz_values[i];
        }
         // Calculate average of gyro values
        avg_raw_gx /= WINDOW_SIZE;
        avg_raw_gy /= WINDOW_SIZE;
        avg_raw_gz /= WINDOW_SIZE;
        // Convert degree per second to radians per second by multiplying it with pi/180 and 
        // get frequency from angular velocity by dividing angular velocity with 2*pi
        // After simplifying the equation, we have to divide angular velocity with 360
        int16_t freq_gx,freq_gy,freq_gz =0;
        freq_gx = abs(avg_raw_gx) / 360;
        freq_gy = abs(avg_raw_gy) / 360;
        freq_gz = abs(avg_raw_gz) / 360;


        // Display the Frequencies on LCD
        char buffer[32];
        LCD.Clear(LCD_COLOR_BLACK);
        sprintf(buffer, "Freq gx: %d", freq_gx);
        LCD.DisplayStringAt(0, LINE(3), (uint8_t *)buffer, CENTER_MODE);
        sprintf(buffer, "Freq gy: %d", freq_gy);
        LCD.DisplayStringAt(0, LINE(4), (uint8_t *)buffer, CENTER_MODE);
        sprintf(buffer, "Freq gz: %d", freq_gz);
        LCD.DisplayStringAt(0, LINE(5), (uint8_t *)buffer, CENTER_MODE);


        ThisThread::sleep_for(5s);

        // Check if frequency falls within the specified range (3 to 6 Hz)
        bool detected_gy = (freq_gy >= 3 && freq_gy <= 6);
        bool detected_gz = (freq_gz >= 3 && freq_gz <= 6);

        LCD.SetBackColor(LCD_COLOR_BLACK);
        LCD.SetTextColor(LCD_COLOR_WHITE);
        LCD.SetFont(&Font16);

        // Print Less or More Intensity based on Frequency value.
        //
        if (detected_gy || detected_gz){
            LCD.DisplayStringAt(0, LINE(8), (uint8_t *)"Parkinsons Detected", CENTER_MODE);
            led.write(1);
            if  ((freq_gy >=5 && detected_gy) ||(freq_gz >=5 && detected_gz)){
                LCD.DisplayStringAt(0, LINE(10), (uint8_t *)"More Intensity", CENTER_MODE);
            }
            else{
                LCD.DisplayStringAt(0, LINE(10), (uint8_t *)"Less Intensity", CENTER_MODE);
            }
            ThisThread::sleep_for(5s);
            break;
        } else {
            LCD.DisplayStringAt(0, LINE(8), (uint8_t *)"No Parkinsons", CENTER_MODE);
        }

        ThisThread::sleep_for(5s);

    
    }

 
}






