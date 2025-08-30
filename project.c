/********************************************
 *
 *  Name: Ollie LaScala
 *  Email: olascala@usc.edu
 *  Section: Friday 12:30 - 1:50
 *  Assignment: Final Project
 *
 ********************************************/

 #include <avr/io.h>
 #include <util/delay.h>
 #include <stdio.h>
 #include <avr/interrupt.h>
 #include <avr/eeprom.h>
  
 #include "lcd.h"
 #include "ds18b20.h"
 #include "encoder.h"
 #include "timers.h"
 #include "serial.h"
 #include "project.h"
 
 volatile uint8_t Low_EEPROM = 50;
 volatile uint8_t High_EEPROM = 90;
 
 volatile uint8_t old_state = 0, new_state = 0; //state variables (same as used in lab6)
 volatile uint8_t a = 0, b = 0; // a and b variables to use as logic
 
 volatile uint8_t high_set = 1; //set the original temp change option at the high point, press button to see if 
 volatile uint8_t prev_button_press = 0;
 
 volatile uint8_t lowvalue; //initial high/low values
 volatile uint8_t highvalue; 
 
 volatile uint16_t T2_count =0; //count variable
 volatile uint8_t servo_pwm = MINIMUM_PWM; //initial pwm (all of these variables were the same I used in past labs)
 volatile uint8_t servo_move = 0; //initialize the move to 0-dont move
 
 volatile uint8_t local_set = 1;         //initializes it in local mode
 volatile uint8_t prev_RLbutton_press = 0;  //tracks when button is released 
 volatile uint8_t prev_right_press = 0;  //tracks when right button is released helper variable
 volatile uint8_t start_send = 0;
 
 //for checkpoint 3 in order of needed variable creations
 volatile char RDBuffer[5]; //buffer with 5 bytes 
 volatile uint8_t RDStarted = 0; //variable to check if data is starting
 volatile uint8_t RDRecievedData =0;  //variable to tell how many data characters have been recieved
 volatile uint8_t RDEnding = 0; //acts as a data valid flag to check for > to basically see where it is ending
 
 //to help update remote values
 volatile uint8_t valRD_high = 0; //value of remote date high
 volatile uint8_t valRD_low = 0; //value of remote data low
 volatile uint8_t RemoteOn = 0; //remote is currently not on

 //for changing the values between the remote and local values by having a shared variable that can be set to each
 volatile uint8_t ActiveLow; 
 volatile uint8_t ActiveHigh;
 
 int main(void) 
 {
     lcd_init(); //initalize lcd
     sei(); //enable interrupts
     timer1_init(); //initalize timer1
     timer2_init(); //initalize timer2
    
     PORTB |= (1 << PB5); //pullup resistor for PB5
     PCICR |= (1 << PCIE0);    //Port B interupt
     PCMSK0 |= (1 << PCINT5);  //PB5 interupt (checked on sheet)
 
     PORTC |= (1 << PC5); //initialize pull up resistors for PC5 
     PCICR |= (1 << PCIE1);     //PORTC interupt
     PCMSK1 |= (1 << PCINT8) | (1 << PCINT13);   //PC5 Interupt and PC0
 
     PORTD |= (1 << PD2) | (1 << PD3); //interrupts for rotary encoder inputs
     EIMSK |= (1 << INT0);   //External interrupt for PD2 needed to be called
     EICRA |= (1 << ISC00);  //Pin chain interrupt enabled INT0
 
     DDRC |= (1 << PC2) | (1 << PC3) | (1 << PC4); //sets the LED to outputs
     PORTC |= (1 << PC2) | (1 << PC3) | (1 << PC4); //initialize all PORTC LED ouputs to off
 
     DDRB |= (1 << PB3) | (1 << PB4); //PB3 and PB4 outputs
     
 
     UBRR0 = MYUBRR; //from slides on which registers to active (the three below)
     UCSR0B |= (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0); 
     UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); 
 
     PORTC |= (1 << PC0); //the right button on pc0 for transmitting data
 
     //My name original splash screen
     lcd_writecommand(1);
     lcd_moveto(0,1);    
     lcd_stringout("Ollie LaScala");
     lcd_moveto(1,1);
     lcd_stringout("EE109-Project");
     _delay_ms(1000);
     lcd_writecommand(1);  //clear after splash
 
     Low_EEPROM = eeprom_read_byte((void *) 200);   //Read the low/high eeprom values at address 200 and 201        
     High_EEPROM = eeprom_read_byte((void *) 201);    
 
     if (Low_EEPROM >= 50 && Low_EEPROM <= 90) //if statements used to make sure EEPROm values to go outisde leading to makeflash trestarting with wrong L/H values
     {
        lowvalue = Low_EEPROM; //returns lowvalue if valid
     }
     else //goes back to default if outside mark
     {
         lowvalue = 50;
         eeprom_update_byte((void *) 200, lowvalue); //these are valid values in the range 200 and 201 for storing
     }
 
     if (High_EEPROM >= 50 && High_EEPROM <= 90) //returns the correct highvalue for EEPROM
     {
         highvalue = High_EEPROM;
     }
     else //goes back to high regular value if outside mark
     {
         highvalue = 90;
         eeprom_update_byte((void *) 201, highvalue);  
     }
 
     uint8_t x = PIND;
     a = (x >> PD2) & 1; //sets PD2 to the right, then masks it to only contain the value - same for both
     b = (x >> PD3) & 1; //same as PD2
 
     if (!b && !a) //rotary input state logic - was introduced in lab6
     {
        old_state = 0;
     } 
     else if (!b && a)
     {
        old_state = 1;
     } 
     else if (b && !a)
     {
        old_state = 2;
     } 
     else 
     {
        old_state = 3;
     } 
 
     new_state = old_state;
 
     unsigned char t[2];
 
     while (ds_init() == 0) //dont know if I really need to keep but left in
     {
        //if ds not working
        _delay_ms(250);
 
        lcd_moveto(0, 0);
        lcd_stringout("Not working!"); //tells me if in working - we stuck on this 
     }
 
     ds_convert();  // Start first temperature conversion
 
     while (1) 
     {
         if(RDEnding) //if values recieved are finished 
         {
            RDEnding = 0; //set ending process back to 0
            RDfunct_convert(); //call to convert values to real remote high/low values
         }
         if(start_send != 0)
         {
            start_send = 0; //set the function qualifer back off
            DataTransmit(); //call the data transmit function to send one at a time
         }
         else
         { 
             prev_right_press = 0; //checks to ensure only 1 button flip and sets back to 0
         }
        
 
         if (ds_temp(t)) // True if conversion complete
         {
             // Extract temperature value
 
             int16_t upper = (int16_t)t[1] << 8; //converts the 8bit first read value to 16
             int16_t lower = t[0];
 
             int16_t real_temp = upper | lower; //adds the automatically 8 bit value to the whole 16
 
             //Convert to tenths of Fahrenheit (no floats) - the formula works because you need to divide by 16th and multiply by 9/5 /(5*16) = 80, then mult by 10
             //to make it easier to get ones and dec place by just using / and %
             int16_t Ftemp = (((real_temp * 9) / (80)) * 10) + 320; //gives int value of F so I can split it up between decimal and whole numbers
 
             int Ftemp_one = (Ftemp / 10); //gets the ones/tens place of the Ftemp variable
             int Ftemp_dec = (Ftemp % 10);
            
 
             //print sprintf on the lcd
             char buf1[16];
             snprintf(buf1, 16, "%d.%d", Ftemp_one, Ftemp_dec);
             lcd_moveto(0, 12);
             lcd_stringout(buf1);

             if(RemoteOn) //remote doesnt show up unless on
             {
                if(!local_set) //if the button chooses the remote setting and the remote is on
                {
                   ActiveLow = valRD_low; //set remote high and lows to active high and lows
                   ActiveHigh = valRD_high;
                }
                else
                {
                   ActiveLow = lowvalue; //if button chooses local button make local variables the active low and high
                   ActiveHigh = highvalue;
                   
                }

                char buf2[16]; //call the functions to pring the results for lcd shield of remote/local variables, functions are below to explain
                LCDlocal(buf2);
                lcd_moveto(0, 0);
                lcd_stringout(buf2);

                char buf3[16]; //same as above
                LCDremote(buf3);
                lcd_moveto(1,0);
                lcd_stringout(buf3);
                
             }
             else //if remote is not on dont display any remote high low values - always set to local
             {
                ActiveLow = lowvalue; //if button chooses local button make local variables the active low and high
                ActiveHigh = highvalue;

                char buf2[16]; //same as above
                LCDlocal(buf2);
                lcd_moveto(0, 0);
                lcd_stringout(buf2);
             }

             if(Ftemp_one < (ActiveLow))
             {
                PORTC &= ~(1 << PC2); //turns on blue if lower than low values and turns off red and green
                PORTC |= (1 << PC4);
                PORTC |= (1 << PC3);                        
 
                OCR1B = 2500; //sets to 2500 turn off green
 
                if (servo_move == 0) //make sure its not already running
                {
                    servo_pwm = MINIMUM_PWM; //sets back to 0
                    servo_move = 1; //turns on the servo_move
    
                    OCR2A = servo_pwm; //sets OCR2A to pwm
                    T2_count = 0; //sets count back to zero just incase it goes back to in bounds
                }
             }
             else if(Ftemp_one > (ActiveHigh))
             {
                PORTC |= (1 << PC2); //turns on red if higher than high values and turns off blue and green
                PORTC &= ~(1 << PC4);
                PORTC |= (1 << PC3); 
 
                OCR1B = 2500; //again as above
 
                if (servo_move == 0) //make sure its not already running
                {
                    servo_pwm = MINIMUM_PWM; //sets back to 0
                    servo_move = 1; //turns on the servo_move
    
                    OCR2A = servo_pwm; //sets OCR2A to pwm
                    T2_count = 0; //same as above
                }
             }
             else
             {
                servo_move = 0; //when in range servo_move is 0
 
                TIMSK1 |= (1 << TOIE1) | (1 << OCIE1B);
 
                PORTC |= (1 << PC2); //turns on green if between high and low thresh and turns off red and blue
                PORTC |= (1 << PC4);
 
                if((ActiveHigh - ActiveLow) != 0)
                {
                    uint16_t value_above = Ftemp_one - ActiveLow; //gives value of temp currently sitting above the low value
                    uint16_t value_range = ActiveHigh - ActiveLow; //gives range of the values between high and low
                    
                    //ok so the first part take half of the OCR1A multiplied by the value that it currently sits above divided by the range then subtract OCR1A by that (aka light level)
                    //This will give you OCR1B value between 0 and 1250 which for some reason is the right answer because 0 is giving me off bright and 1250 is giving me full bright
                    
                    uint16_t Level_light = (((OCR1A / 2) * value_above) / value_range); //light level for better understanding variable, helps me read better
                    OCR1B = OCR1A - Level_light; //higher the lightlevel the higher the light will be 
                }
                else
                {   //shine fully if special case happens so doesnt give error when dividng by zero - makes sense because it is technically between the two values perfectly
                    OCR1B = (OCR1A * 3) / 4; //special case when the int value equals the high and low values (when High/Low are same) - 
                }
             }

             ds_convert();  // Start next conversion

         }
     }
 }
 
 void debounce(uint8_t bit) //introduce debounce delay as used in lab 5
 {
     _delay_ms(5);
     while ((PINB & (1 << bit)) == 0);  
     {
         //wait (lil delay)
     }
     _delay_ms(5);
 }

 void LCDlocal(char *buf)
 {
    if(local_set)
    {
        if(high_set == 0)
        {
            snprintf(buf, 16, ">L=%d H=%d<", lowvalue, highvalue); //prints the change statement when L button is pressed
        }
        else
        {
            snprintf(buf, 16, " L=%d>H=%d<", lowvalue, highvalue);  //prints the change statement when H button is pressed
        }
    }
    else
    {
        if(high_set == 0)
        {
            snprintf(buf, 16, ">L=%d H=%d ", lowvalue, highvalue); //prints the change statement when L button is pressed and remote is pressed
        }
        else
        {
            snprintf(buf, 16, " L=%d>H=%d ", lowvalue, highvalue); //prints the change statement when h button is pressed and remote is pressed
        }
    }
 }

 void LCDremote(char *buf)
 {
    if(local_set)
    {
        snprintf(buf, 16, " L=%d H=%d ", valRD_low, valRD_high); //prints statement for the remote values when local values are selecting
    }
    else
    {
        snprintf(buf, 16, " L=%d H=%d<", valRD_low, valRD_high); //prints statement for the remote values when remote values are selecting
    }
 }
 
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
        prev_button_press = 0; //checks to ensure only 1 button flip and sets back to 0 (taken from past lab)
    }
 }
 
 ISR(PCINT1_vect)
 {
    if((PINC & (1 << PC5)) == 0) //Local/remote button pressed
    {
        if (prev_RLbutton_press == 0)
        {
            debounce(PC5);  //debounce the remote/local button
            if(RemoteOn)
            {
                local_set ^= 1; //switches between localand remote mode
            }
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