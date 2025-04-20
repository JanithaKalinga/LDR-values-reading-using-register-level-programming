# LDR values reading using register level programming
 This AVR-based program reads light intensity from two LDR sensors using the ADC of an ATmega microcontroller and compares their values periodically. The higher value is transmitted over UART. The sampling interval can be dynamically adjusted between 1 and 10 seconds using two push buttons:

PB1 increases the interval

PB2 decreases the interval

Key Features:

ADC-based LDR sensing

UART communication for serial output

Timer-based periodic sampling (using Timer1 in CTC mode)

External interrupts for push-button control
