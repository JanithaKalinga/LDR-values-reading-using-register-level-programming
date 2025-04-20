#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

// Definitions for pins
#define LDR1_PIN 0   // ADC0 (PC0)
#define LDR2_PIN 1   // ADC1 (PC1)
#define PB1_PIN 2    // INT0 (PD2)
#define PB2_PIN 3    // INT1 (PD3)

volatile uint16_t interval = 5000;  // Default interval in ms
const uint16_t minInterval = 1000;  // Minimum interval in ms
const uint16_t maxInterval = 10000; // Maximum interval in ms
volatile uint16_t elapsedTime = 0;

void ADC_init();
uint16_t ADC_read(uint8_t channel);
void Timer1_init();
void UART_init();
void UART_send(char data);
void UART_sendString(const char* str);
void ExternalInterrupts_init();
void UART_sendFloat(float value);



void ADC_init() {
    // Enable ADC, set prescaler to 128, reference voltage to AVCC
    ADMUX = (1 << REFS0);  // AVCC reference voltage
    DIDR0 |= (1<<ADC0D) | (1<<ADC1D); // Disable digital inputs in ADC0 and ADC1 pins
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // Enable ADC and set prescaler to 128
}

uint16_t ADC_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);  // Select ADC channel
    ADCSRA |= (1 << ADSC);  // Start conversion
    while (ADCSRA & (1 << ADSC));  // Wait for conversion to complete
    return ADC;
}


void Timer1_init() {
    // Configure Timer1 for CTC mode, prescaler 1024
    TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);  // CTC mode with 1024 prescaler
    OCR1A = 15624;  // 1 second interrupt with 16MHz clock and 1024 prescaler
    TIMSK1 |= (1 << OCIE1A);  // Enable Timer1 compare interrupt
}

void UART_init() {
    // Set baud rate to 9600
    UBRR0 = 103;
    UCSR0B = (1 << TXEN0);  // Enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8-bit data
}

void UART_send(char data) {
    while (!(UCSR0A & (1 << UDRE0)));  // Wait for the transmit buffer to be empty
    UDR0 = data;  // Send data
}

void UART_sendString(const char* str) {
    while (*str) {
        UART_send(*str++);
    }
}


void ExternalInterrupts_init() {
    // Configure INT0 and INT1 for falling edge
    EICRA |= (1 << ISC01) | (1 << ISC11);
    EIMSK |= (1 << INT0) | (1 << INT1);  // Enable external interrupts INT0 and INT1
}

void UART_sendFloat(float value) {
    char buffer[10];
    int integerPart = (int)value;  // Extract integer part
    int decimalPart = (int)((value - integerPart) * 100);  // Extract decimal part (2 decimal places)
    
    snprintf(buffer, sizeof(buffer), "%d.%02d", integerPart, decimalPart);
    UART_sendString(buffer);
}

ISR(TIMER1_COMPA_vect) {
    elapsedTime += 1000;  // Increment elapsed time by 1 second
    if (elapsedTime >= interval) {
        elapsedTime = 0;

        // Read LDR values
        uint16_t ldr1Value = ADC_read(LDR1_PIN);
        uint16_t ldr2Value = ADC_read(LDR2_PIN);

        // Map ADC values to 0-100 (scaled from 0 to 1 for 0-1023)
        float ldr1Mapped = (ldr1Value * 100.0) / 1023;
        float ldr2Mapped = (ldr2Value * 100.0) / 1023;

        // Compare and send the higher value over UART
        if (ldr1Mapped >= ldr2Mapped) {
            UART_sendString("LDR1: ");
            UART_sendFloat(ldr1Mapped / 100);
            UART_sendString("\r\n");
        } else {
            UART_sendString("LDR2: ");
            UART_sendFloat(ldr2Mapped / 100);
            UART_sendString("\r\n");
        }
    }
}

// Interrupt service routine for PB1 (INT0)
ISR(INT0_vect) {
    if (interval < maxInterval) {
        interval += 1000;  // Increase interval by 1 second
    }
}

// Interrupt service routine for PB2 (INT1)
ISR(INT1_vect) {
    if (interval > minInterval) {
        interval -= 1000;  // Decrease interval by 1 second
    }
}

int main(void) {
    ADC_init();           // Initialize ADC
    Timer1_init();        // Initialize Timer1
    UART_init();          // Initialize UART for serial communication
    ExternalInterrupts_init();  // Initialize external interrupts

    sei();  // Enable global interrupts

    while (1) {
        // Main loop does nothing, everything is handled by interrupts
    }
}
