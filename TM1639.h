#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif

#define MAX_INTENSITY 7
#define AUTOMATIC_ADDRESS_MODE 0x40
#define DIRECT_ADDRESS_MODE 0x44
#define DISPLAY_OFF_BRIGHTNESS 0x88
#define START_ADDRESS 0xC0

const byte DIGIT_SWITCH_OFF_DATA[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
const byte TIME_SEMICOLON_DATA[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6 };
const byte DEGREE_DATA[] = { 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0 };
const byte HUMIDITY_DATA[] = { 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0 };

const byte DIGIT0[][16] = {
                      { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0 },//1
                      { 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0 },//2
                      { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },//3
                      { 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0 },//4
                      { 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0 },//5
                      { 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0 },//6
                      { 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 },//7
                      { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 },//8
                      { 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0 } //9
                      
                    };
const byte DIGIT1[][16] = {
                      { 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0, 0 },//1
                      { 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 0, 0 },//2
                      { 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0 },//3
                      { 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0 },//4
                      { 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0 },//5
                      { 2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0 },//6
                      { 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0, 0 },//7
                      { 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0 },//8
                      { 2, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0 } //9
                    };                    
const byte DIGIT2[][16] = {
                      { 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0 },//0
                      { 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//1
                      { 4, 0, 4, 0, 0, 0, 4, 0, 4, 0, 0, 0, 4, 0, 0, 0 },//2
                      { 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 4, 0, 0, 0 },//3
                      { 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 4, 0, 4, 0, 0, 0 },//4
                      { 4, 0, 0, 0, 4, 0, 4, 0, 0, 0, 4, 0, 4, 0, 0, 0 },//5
                      { 4, 0, 0, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0 },//6
                      { 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//7
                      { 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0 },//8
                      { 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 4, 0, 4, 0, 0, 0 } //9
                    };                    
const byte DIGIT3[][16] = {
                      { 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0, 0 },//1
                      { 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 0, 0 },//2
                      { 8, 0, 0, 0, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0 },//3
                      { 0, 0, 0, 0, 8, 0, 0, 0, 8, 0, 8, 0, 8, 0, 0, 0 },//4
                      { 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0 },//5
                      { 8, 0, 8, 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0 },//6
                      { 0, 0, 0, 0, 0, 0, 8, 0, 8, 0, 8, 0, 0, 0, 0, 0 },//7
                      { 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0 },//8
                      { 8, 0, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0 } //9
                    };                    
const byte DIGIT4[][16] = {
                      { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0 },//1
                      { 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0 },//2
                      { 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0 },//3
                      { 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0 },//4
                      { 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0 },//5
                      { 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0 },//6
                      { 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0 },//7
                      { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0 },//8
                      { 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0 } //9
                    };                    
const byte DIGIT5[][16] = {
                      { 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 0, 0, 0 },//1
                      { 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 0 },//2
                      { 0, 2, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0 },//3
                      { 0, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0 },//4
                      { 0, 2, 0, 0, 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0 },//5
                      { 0, 2, 0, 2, 0, 2, 0, 2, 0, 0, 0, 2, 0, 2, 0, 0 },//6
                      { 0, 0, 0, 0, 0, 0, 0, 2, 0, 2, 0, 2, 0, 0, 0, 0 },//7
                      { 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0 },//8
                      { 0, 2, 0, 0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 0 } //9
                    };                    
const byte DIGIT6[][16] = {
                      { 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//1
                      { 0, 4, 0, 4, 0, 0, 0, 4, 0, 4, 0, 0, 0, 4, 0, 0 },//2
                      { 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 4, 0, 0 },//3
                      { 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 0, 4, 0, 4, 0, 0 },//4
                      { 0, 4, 0, 0, 0, 4, 0, 4, 0, 0, 0, 4, 0, 4, 0, 0 },//5
                      { 0, 4, 0, 0, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0 },//6
                      { 0, 4, 0, 4, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//7
                      { 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 0 },//8
                      { 0, 4, 0, 4, 0, 4, 0, 4, 0, 0, 0, 4, 0, 4, 0, 0 } //9
                    };                    
//                      0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15  
const byte DIGIT7[][16] = {
                      { 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0, 0 },//0
                      { 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0 },//1
                      { 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 0 },//2
                      { 0, 8, 0, 0, 0, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0 },//3
                      { 0, 0, 0, 0, 0, 8, 0, 0, 0, 8, 0, 8, 0, 8, 0, 0 },//4
                      { 0, 8, 0, 0, 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0 },//5
                      { 0, 8, 0, 8, 0, 8, 0, 8, 0, 0, 0, 8, 0, 8, 0, 0 },//6
                      { 0, 0, 0, 0, 0, 0, 0, 8, 0, 8, 0, 8, 0, 0, 0, 0 },//7
                      { 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0 },//8
                      { 0, 8, 0, 0, 0, 8, 0, 8, 0, 8, 0, 8, 0, 8, 0, 0 } //9
                    };  

class TM1639 {
    private :
        /* The array for shifting the data to the devices */
        byte ledData[16];
        
        /* Send out a single command to the device */
        void sendCommand(byte data);

        void display(byte addr, byte data);

        void setData(byte data[16]);

        void sendData(byte address, byte data);

        void writeByte(byte data);

        /* Data is shifted out of this pin*/
        byte DIN_PIN;
        
        /* The clock is signaled on this pin */
        byte CLK_PIN;
        
        /* This strob is signaled on this pin  */
        byte STB_PIN;

    public:
        /* 
         * Create a new controler 
         * Params :
         * dataPin    pin on the Arduino where data gets shifted out
         * clockPin   pin for the clock
         * stbPin     pin for the strob
         */
        TM1639(byte dataPin, byte clkPin, byte stbPin);

        /* 
         * Set the brightness of the display.
         * Params:
         * intensity  the brightness of the display. (0..7)
         */
        void setIntensity(byte intensity);

        /* 
         * Switch all Leds on the display off. 
         */
        void clearDisplay();


        /* 
         * Display a hexadecimal digit on a 7-Segment Display
         * Params:
         * digit  the position of the digit on the display (0..7)
         * value  the value to be displayed. (0x00..0x0F)
         */
        void setDigit(byte digit, byte value);

        /* 
         * Switch off digit on a 7-Segment Display
         * Params:
         * digit  the position of the digit on the display (0..7)
         */
        void switchOffDigit(byte digit);

        /* 
         * Switch on/off semicolon on a 7-Segment Display
         * Params:
         * isShown state
         */
        void showTimeTick(boolean isShown);

        /* 
         * Show Celcius degree sign on a 7-Segment Display
         */
        void showDegreeSign();

        /* 
         * Show humidity sign on a 7-Segment Display
         */
        void showHumiditySign();

        /* 
         * Refreshes data on a 7-Segment Display
         */
        void updateDisplay();
};
