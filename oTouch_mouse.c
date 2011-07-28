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

//Definition of the notes' frequecies in Hertz.
#define c 261
#define d 294
#define e 329
#define f 349
#define g 391
#define gS 415
#define a 440
#define aS 455
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880

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

  ADC10CTL1 = INCH_3;				// Channel 3 to be used for y reading LEFT
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

 
void main(void) {
  unsigned int x_val;
  unsigned int y_val;
  
//SETUP	
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  //Setup SCI
  P1SEL |= BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
  P1SEL2 |= BIT1 + BIT2 ;                   // P1.1 = RXD, P1.2=TXD
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK
  UCA0BR0 = 104;                            // 1MHz 9600
  UCA0BR1 = 0;                              // 1MHz 9600
  UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  _EINT();
  //END SETUP SCI
  
  ADC10CTL0 = ADC10ON + REFOUT + SREF1 + ADC10SHT_2;		// ADC10ON
//Main loop
  while (1) {
  	//Update values
	x_val = get_x();
	delay_ms(10);
	y_val = get_y();
	delay_ms(10);
	
	//recv = UCA0RXBUF;
	
   	//Send values through serial
  	if(recv == 'x') {
		while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  		UCA0TXBUF = 'x';		                  // TX 'x'
		while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  		UCA0TXBUF = (x_val >> 8);                 // TX x top byte
  		while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
  		UCA0TXBUF = (x_val & 0x00FF);             // TX x bottom byte
  		recv = 0x00;							  // Reset recv to stop sending
  	}
	if(recv == 'y') {
	  	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	  	UCA0TXBUF = 'y';                 		  // TX 'y'
	  	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	  	UCA0TXBUF = (y_val >> 8);                 // TX y top byte
	  	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	  	UCA0TXBUF = (y_val & 0x00FF);             // TX y bottom byte
	  	recv = 0x00;
	}
  } 
}


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){ 		// RX interrupt
  	recv = UCA0RXBUF;                   		// Read RX
}
