#include <msp430g2231.h>

// SRCLK P1.0
// SER1 P1.1
// SER2 P1.2
// SER3 P1.3
// SER4 P1.4
// RCLK P1.5

// Led 1-7 of seven segment display correspond to outputs A-G of bit shift register

// index i of array digitx denotes whether led i will be on or off for the digitx
const char digit0[] = {1, 1, 1, 1, 1, 1, 0};
const char digit1[] = {0, 1, 1, 0, 0, 0, 0};
const char digit2[] = {1, 1, 0, 1, 1, 0, 1};
const char digit3[] = {1, 1, 1, 1, 0, 0, 1};
const char digit4[] = {0, 1, 1, 0, 0, 1, 1};
const char digit5[] = {1, 0, 1, 1, 0, 1, 1};
const char digit6[] = {1, 0, 1, 1, 1, 1, 1};
const char digit7[] = {1, 1, 1, 0, 0, 0, 0};
const char digit8[] = {1, 1, 1, 1, 1, 1, 1};
const char digit9[] = {1, 1, 1, 1, 0, 1, 1};

const char* digits[10];

char secondsUnitsDigitIndex = 0;
char secondsTensDigitIndex = 0;
char minutesUnitsDigitIndex = 0;
char minutesTensDigitIndex = 0;

char timeCounter = 0;

int main(void){

    // TODO: update watch dog timer to make clock display go back to 0000 when the day is over
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    P2OUT = 0;
    P2DIR = 0;

    P1OUT = 0;  // setting all gpio to zero initially
    P1DIR = 1 + 2 + 4 + 8 + 16 + 32; //  P1.0 - P1.5 are outputs, P1.6 and P1.7 are not being used

    digits[0] = digit0;
    digits[1] = digit1;
    digits[2] = digit2;
    digits[3] = digit3;
    digits[4] = digit4;
    digits[5] = digit5;
    digits[6] = digit6;
    digits[7] = digit7;
    digits[8] = digit8;
    digits[9] = digit9;

    CCTL0 = CCIE;
    CCR0 = 49999;  //FIXME: CCR0 cannot take numbers bigger than 999980?
    TACTL = TASSEL_2 + MC_1;
    __bis_SR_register(GIE+LPM0_bits);
    // wait for timer for completion of 0.99998 seconds, i.e., reach CCR0 value of 49999 20 times
}

#pragma vector = TIMERA0_VECTOR
__interrupt void Timer_A (void) {

    timeCounter++;

    if (timeCounter == 20) {
        // update shift registers with values
        signed char led;
        for (led = 6; led >= 0; led--) {
            P1OUT = digits[secondsUnitsDigitIndex][led]*2 + digits[secondsTensDigitIndex][led]*4
                    + digits[minutesUnitsDigitIndex][led]*8 + digits[minutesTensDigitIndex][led]*16;   // set ser1 to thecurrent bit for the current digit
            _delay_cycles(1);   // time delay between ser change and srclk high
            P1OUT |= BIT0;  // srclk high
            _delay_cycles(1);   // pulse duration for srclk high
            P1OUT &= ~(BIT0);   // srclk low
        }

        // set output to new values in shift registers
        _delay_cycles(1);   // time delay between srclk pulse and rclk high
        P1OUT |= BIT5;  // rclk high
        _delay_cycles(1);   // pulse duration for rclk high
        P1OUT &= ~(BIT5);   // rclk low

        // update seconds units digit display index every second,
        // update seconds tens digit display index when seconds units digit display index passes 9
        // update minutes units digit display index when seconds tens digit display index passes 5
        // update minutes tens digit display index when minutes units digit display index passes 9
        // reset all indices when 59:59 is reached
        secondsUnitsDigitIndex++;
        if (secondsUnitsDigitIndex == 10) {
            secondsUnitsDigitIndex = 0;
            secondsTensDigitIndex++;
            if (secondsTensDigitIndex == 6) {
                secondsTensDigitIndex = 0;
                minutesUnitsDigitIndex++;
                if (minutesUnitsDigitIndex == 10) {
                    minutesUnitsDigitIndex = 0;
                    minutesTensDigitIndex++;
                    if (minutesTensDigitIndex == 6) {
                        minutesTensDigitIndex = 0;
                        secondsUnitsDigitIndex = 0;
                        secondsTensDigitIndex = 0;
                        minutesUnitsDigitIndex = 0;
                    }
                }
            }
        }

        timeCounter = 0;
    }
}
