//Test code for the nintendo ds touchscreen with msp430 launchapad.

#include  "msp430g2553.h"
#include  "stdint.h"
#include  "math.h"

#define X1 BIT3
#define Y2 BIT4
#define X2 BIT5
#define Y1 BIT6
#define chan_X1 INCH_3
#define chan_Y2 INCH_4
#define chan_X2 INCH_5
#define chan_Y1 INCH_6

#define Speaker BIT7

char recv;

void delay_ms(unsigned int ms ) {
    while (ms) {
		__delay_cycles(1000); 		// set it to 1000 for 1 Mhz
		ms = ms-1;
	}
}
 
void delay_us(unsigned int us ) {
    unsigned int i;
    for (i = 0; i<= us/2; i++) 
       __delay_cycles(1);
}

int get_x(void) {
  int x_adc;
  ADC10CTL0 &= ~SREF1;				// ADC10ON
  ADC10CTL1 = chan_Y1;				// Channel 4 to be used for x reading BOTTOM
  ADC10AE0 = Y1;					// Set P1.4 (A4) as adc pin
  P1DIR |= X1 + X2;					// Set P1.3 and P1.5 as output
  P1OUT &= ~X1;  					// Set left to 0
  P1OUT |= X2;   					// Set right to VCC (or at least to logical 1)
  P1DIR &= ~Y2;						// Disc Y1 (throwing off #s)
  delay_ms(10);					    // delay
  ADC10CTL0 |= ADC10SC + ENC;		
  ADC10CTL0 &= ~ADC10SC;
  while((ADC10CTL1 & ADC10BUSY) == 1);
  // Disable conversion
  ADC10CTL0 &= ~ENC;
  ADC10CTL0 |= SREF1;
  x_adc = ADC10MEM;
  return x_adc;
}

int get_y(void) {
  int y_adc;

  ADC10CTL1 = chan_X1;				// Channel 3 to be used for y reading LEFT
  ADC10AE0 = X1;					// Set P1.3 (A3) as adc pin
  P1DIR |= Y1 + Y2;					// Set P1.6 and P1.4 as output
  P1OUT &= ~Y1;  					// Set TOP to 0
  P1OUT |= Y2;   					// Set BOTTOM to VCC (or at least to logical 1)
  P1DIR &= ~X2;						// Disc X2 (throwing off #s)
  delay_ms(10);					    // delay
  ADC10CTL0 |= ADC10SC + ENC;
  ADC10CTL0 &= ~ADC10SC;
  while((ADC10CTL1 & ADC10BUSY) == 1);
  // Disable conversion
  ADC10CTL0 &= ~ENC;
  y_adc = ADC10MEM;
  return y_adc;
}

void beep(unsigned int note, unsigned int duration) {	
    int i;  
    long delay = (long)(10000/note);  				// This is the semiperiod of each note. 
    long time = (long)((duration*100)/(delay*2));  	// This is how much time to spend on the note.
    for (i=0;i<time;i++) {    
        P1OUT |= BIT7;     							// Set P1.7 as output pwn hight
        delay_us(delay);  							// keep pwm hight
        P1OUT &= ~BIT7;    							// pwm low
        delay_us(delay);   							// keep low  (make a square wave)
    }  
    delay_ms(20); 									// delay to separate the single notes
}
 
void main(void) {
  unsigned int freq = 466;
  unsigned int dur = 400;
  unsigned int x_val;
  unsigned int y_val;
  
//SETUP	
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  ADC10CTL0 = ADC10ON + REFOUT + SREF1 + ADC10SHT_2;		// ADC10ON
  P1DIR |= Speaker;
//Main loop
  while (1) {
  	//Update values
	x_val = get_x();
	delay_ms(10);
	y_val = get_y();
	delay_ms(10);
	
	if ((x_val > 30) && (y_val > 70)) { 	// Acount for no touch
		freq = x_val;
		dur = y_val;
	}
	beep(freq, 440);						// Default beat duration set to 440
	delay_ms(dur);
  } 
}
