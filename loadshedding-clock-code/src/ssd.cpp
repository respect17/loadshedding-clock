#include "ssd.h"

// common cathode seven-segment display pins
const int PIN_A = 4;
const int PIN_B = 13;
const int PIN_C = 14;
const int PIN_D = 0;
const int PIN_E = 26;
const int PIN_F = 25;
const int PIN_G = 12;
const int DIGIT_0_CATHODE = 23; // leftmost coundown digit
const int DIGIT_1_CATHODE = 19;
const int DIGIT_2_CATHODE = 16;
const int DIGIT_3_CATHODE = 17; // rightmost countdown digit
const int STAGE_DIGIT_CATHODE = 18;

// segement pins in GFEDCBA order
const int NUM_SEGMENTS = 7;
const int SSD_PINS[] = {PIN_G, PIN_F, PIN_E, PIN_D, PIN_C, PIN_B, PIN_A};

// digit pins in order
const int NUM_DIGITS = 5;
const int SSD_DIGITS[] = {DIGIT_0_CATHODE, DIGIT_1_CATHODE, DIGIT_2_CATHODE, DIGIT_3_CATHODE, STAGE_DIGIT_CATHODE};

// values to display on each digit
int currentDigitValues[] = {0, 0, 0, 0, 0}; // 0 is the leftmost digit. 3 is the rightmost digit. 4 is the stage digit.

// digitCodeMap to indicate which segments must be illuminated to display each number
// from https://github.com/DeanIsMe/SevSeg
static const uint8_t digitCodeMap[] = {
    // GFEDCBA  Segments      7-segment map:
    0b00111111, // 0   "0"          AAA
    0b00000110, // 1   "1"         F   B
    0b01011011, // 2   "2"         F   B
    0b01001111, // 3   "3"          GGG
    0b01100110, // 4   "4"         E   C
    0b01101101, // 5   "5"         E   C
    0b01111101, // 6   "6"          DDD
    0b00000111, // 7   "7"
    0b01111111, // 8   "8"
    0b01101111, // 9   "9"
    0b01110111, // 65  'A'
    0b01111100, // 66  'b'
    0b00111001, // 67  'C'
    0b01011110, // 68  'd'
    0b01111001, // 69  'E'
    0b01110001, // 70  'F'
    0b00111101, // 71  'G'
    0b01110110, // 72  'H'
    0b00110000, // 73  'I'
    0b00001110, // 74  'J'
    0b01110110, // 75  'K'  Same as 'H'
    0b00111000, // 76  'L'
    0b00000000, // 77  'M'  NO DISPLAY
    0b01010100, // 78  'n'
    0b00111111, // 79  'O'
    0b01110011, // 80  'P'
    0b01100111, // 81  'q'
    0b01010000, // 82  'r'
    0b01101101, // 83  'S'
    0b01111000, // 84  't'
    0b00111110, // 85  'U'
    0b00111110, // 86  'V'  Same as 'U'
    0b00000000, // 87  'W'  NO DISPLAY
    0b01110110, // 88  'X'  Same as 'H'
    0b01101110, // 89  'y'
    0b01011011, // 90  'Z'  Same as '2'
    0b00000000, // 32  ' '  BLANK
    0b01000000, // 45  '-'  DASH
    0b10000000, // 46  '.'  PERIOD
    0b01100011, // 42 '*'  DEGREE ..
    0b00001000, // 95 '_'  UNDERSCORE
};

/**
 * Clears all seven-segment displays.
 */
void clearSSDs()
{
    // loop through digits and disable all
    for (int i = 0; i < NUM_DIGITS; i++)
    {
        // digit disabled: digit cathode pin = HIGH-IMPEDANCE (INPUT)
        pinMode(SSD_DIGITS[i], INPUT);
    }

    // loop through segments and disable all
    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        // segment disabled: segment pin = HIGH-IMPEDANCE (INPUT)
        pinMode(SSD_PINS[i], INPUT);
    }
}

/**
 * Displays a test pattern on the seven-segment displays.
 */
void testSSDs()
{
    clearSSDs();

    // test ssds
    for (int i = 1; i < 10; i++)
    {
        for (int j = 0; j < NUM_DIGITS; j++)
        {
            currentDigitValues[j] = i;
        }
        delay(250);
    }

    clearSSDs();
}

/**
 * Displays a digit on the given seven-segment display.
 *
 * @param digit The digit on which to display the value.
 * 0 is the leftmost digit.
 * 3 is the rightmost digit.
 * 4 is the stage digit.
 * @param value The value to display on the digit.
 */
void displayOnSSD(int digit, int value)
{
    // disable all digits and segments
    clearSSDs();

    // enable digit: digit cathode pin = LOW
    pinMode(SSD_DIGITS[digit], OUTPUT);
    digitalWrite(SSD_DIGITS[digit], LOW);

    // loop through segments and enable segments to display value
    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        if (digitCodeMap[value] & 0b01000000 >> i)
        {
            // segment enabled: segment pin = LOW
            pinMode(SSD_PINS[i], OUTPUT);
            digitalWrite(SSD_PINS[i], LOW);
        }
    }
}

/**
 * Updates all the current values of the seven-segment displays.
 *
 * @param secondsUntilNextTransition The number of seconds until the next transition.
 * @param stage The current stage of loadshedding.
 */
void updateSSDs(int secondsUntilNextTransition, int stage)
{
    // convert seconds to hour value and minute value
    int hoursUntilNextEvent = secondsUntilNextTransition / 3600;
    int minutesUntilNextEvent = (secondsUntilNextTransition % 3600) / 60;

    // extract digits
    int firstDigitOfHourValue = hoursUntilNextEvent / 10;
    int secondDigitOfHourValue = hoursUntilNextEvent % 10;
    int firstDigitOfMinuteValue = minutesUntilNextEvent / 10;
    int secondDigitOfMinuteValue = minutesUntilNextEvent % 10;

    // update current digit values
    currentDigitValues[0] = firstDigitOfHourValue;
    currentDigitValues[1] = secondDigitOfHourValue;
    currentDigitValues[2] = firstDigitOfMinuteValue;
    currentDigitValues[3] = secondDigitOfMinuteValue;
    currentDigitValues[4] = stage;
}

int currentDigit = 0;

/**
 * Refreshes all seven-segment displays.
 * This function must be called repeatedly to keep the displays visible.
 */
void refreshSSDs()
{
    // increment current digit
    currentDigit++;
    currentDigit = currentDigit % NUM_DIGITS;
    displayOnSSD(currentDigit, currentDigitValues[currentDigit]);
}
