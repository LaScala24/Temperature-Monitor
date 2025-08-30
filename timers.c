#include "timers.h"
#include "project.h"

void timer1_init(void)
{
    TIMSK1 |= (1 << TOIE1) | (1 << OCIE1B); //in data sheet for timer interupts

    TCCR1A |= (1 << WGM11) | (1 << WGM10); //set all values to 1 as disclosed in document to set to fast mode
    TCCR1B |= (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10); //sets the 13 and 12 bits to 1. And also gives me the prescalar value for 64

    OCR1A = 2500; // got by 16million / 64 * 10ms period = 2500
}

void timer2_init(void) //timer 2 from lab7
{
    TCCR2A |= (0b11 << WGM20);   //Fast PWM mode
    TCCR2A |= (0b10 << COM2A0);  //toggles short pulses
    OCR2A = MINIMUM_PWM; //12
    TCCR2B |= (0b111 << CS20);   //sets prescalar to 1024

    TIMSK2 |= (1 << TOIE2);     //calls isr when overlow
}


ISR(TIMER1_OVF_vect) //just turns on green LED
{
    // Turn the PWM bit on
    PORTC |= (1 << PC3);
}

ISR(TIMER1_COMPB_vect) //just turns off green LED
{
    // Turn the PWM bit off
    PORTC &= ~(1 << PC3);
}

ISR(TIMER2_OVF_vect)
{
    if(servo_move == 0)
    {
        return; //returns nothing if either motor not activated, therefore count is progressed
    }

    T2_count++;

    if(T2_count >= 27) //we have 10/23 seconds/steps = 0.4348 so 434/16.4 which gives around 26.5 but rounds up to get full 10seconds 
    {
        servo_pwm++; //increase the pwm signal by 1 - already at 12

        T2_count= 0; //resets the count back to 0

        if (servo_pwm >= MAXIMUM_PWM) //wants goes over 35 so from 12 to 35 then set it to 35 for the rest of time
        {
            servo_pwm = MAXIMUM_PWM; //stays at max pwm
        }

        OCR2A = servo_pwm; //set the OCR2A as the current servopwm
    }
}