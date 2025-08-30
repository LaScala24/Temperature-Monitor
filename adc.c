#include <avr/io.h>

#include "adc.h"


void adc_init(void)
{
    // Initialize the ADC 
    ADMUX = (1 << REFS0) | (1 << ADLAR); //Gives analog range between 0 and 5 volts - REFS1 is already 0 so dont need to set 

    ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //set prescalar to 128 by using the ADPS's

    ADMUX |= (1 << ADLAR); //set 1 to adlar in admux to use 8 bit readings

    ADCSRA |= (1 << ADEN); //Lastly turn initialize ADEN on/off switch
}

uint8_t adc_sample(uint8_t channel)
{
    // Set ADC input mux bits to 'channel' value
    
    ADMUX = ((ADMUX & 0xf0) | (channel & 0x0f)); //set ADMUX to channel A3

    // Convert an analog input and return the 8-bit result

    ADCSRA |= (1 << ADSC); //Sets ADSC bit

    while((ADCSRA & (1 << ADSC)) != 0)
    {
        //Continues to loop until ADSC turns back to 0
    }

    return ADCH; //returns the value from the ADCH

}
