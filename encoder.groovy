#include <avr/io.h>
#include "project.h"

ISR(PCINT0_vect)
     {
        if((PINB & (1 << PB5)) == 0) //High/Low button is pressed
        {
            if(prev_button_press == 0)
            {
                debounce(PB5); //debounce the HI/LO button
                high_set ^= 1; //flips high_set 
                prev_button_press = 1; //sets the previous to 1
            }
        }
        else
        {
            prev_button_press = 0; //checks to ensure only 1 button flip and sets back to 0
        }
     }

     ISR(PCINT1_vect)
     {
        if((PINC & (1 << PC5)) == 0) //Local/remote button pressed
        {
            if (prev_RLbutton_press == 0)
            {
                debounce(PC5);  //debounce the remote/local button
                local_set ^= 1; //switches between localand remote mode
                prev_RLbutton_press = 1; //sets the previous to 1
            }
        }
        else
        {
            prev_RLbutton_press = 0;
        }

        if((PINC & (1 << PC0)) == 0)
        {
            if(prev_right_press == 0)
            {
                debounce(PC0); //debounce PC0 - ask if needed not sure
                prev_right_press = 1; //set the previous right to 1 
                start_send = 1;  //set start_send to 1 to then call the send function 
            }
        }
     }

     ISR(INT0_vect)
     {
        //same as above but new in ISR-reintroduced to continue not initalized
        uint8_t x = PIND;
        a = (x >> PD2) & 1;
        b = (x >> PD3) & 1;

        if (old_state == 0) 
        {
        if (!b && a)
        {
            new_state = 1;
        } 
        else if (b && !a) 
        {
            new_state = 2;
        }
        } 
        else if (old_state == 1) 
        {
        if (b && a)
        {
            new_state = 3;
        } 
        else if (!b && !a) 
        {
            new_state = 0;
        }
        } 
        else if (old_state == 2) 
        {
        if (!b && !a) 
        {
            new_state = 0;
        }
        else if (b && a) 
        {
            new_state = 3;
        }
        } 
        else if (old_state == 3) 
        {
        if (b && !a) 
        {
            new_state = 2;
        }
        else if (!b && a) 
        {
            new_state = 1;
        }
    }

    if (new_state != old_state) //makes sure it has changed values
    {
        //the clockwise functions for the rotary
        if ((old_state == 0 && new_state == 1) || (old_state == 1 && new_state == 3) || (old_state == 3 && new_state == 2) || (old_state == 2 && new_state == 0))
        {
            if (high_set) //if high value is changing
            {
                if (highvalue < 90) //max is 90
                    highvalue++;
            }
            else
            {
                if (lowvalue < 90 && lowvalue < highvalue) //if low value is changing - must be less than high value and less than the max is 90
                    lowvalue++;
            }
        }
        else
        {
            //the counter clockwise functions for the rotary
            if (high_set) //high value is changing
            {
                if (highvalue > lowvalue) //cannot be lower than the low valeue
                    highvalue--;
            }
            else
            {
                if (lowvalue > 50) //low value cannot go under 50
                    lowvalue--;
            }
        }

        if(high_set == 0) //if high set is a - or we are changing the low thresh
        {
            eeprom_update_byte((void *) 200, lowvalue);
        }
        else //if we are changing the high thresh
        {
            eeprom_update_byte((void *) 201, highvalue);
        }

        old_state = new_state; //set the old state equal to the state that just went through
    }
 }