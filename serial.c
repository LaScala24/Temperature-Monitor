#include <stdio.h>
#include "serial.h"
#include "project.h"

void DataTransmit(void)
{
    uint8_t UDR0character[7] = {}; // character I am going to send initialized

    UDR0character[0] = '<';

    UDR0character[1] = ((lowvalue / 10) + '0'); // enters the character values for the low two adds 0 to convert to ascii EX
    UDR0character[2] = ((lowvalue % 10) + '0'); 

    UDR0character[3] = ((highvalue / 10) + '0'); // enters the character values for the high two
    character[4] = ((highvalue % 10) + '0'); 

    UDR0character[5] = '>'; // this is what we check to end flag from writeup prompt 
    UDR0character[6] = '\0'; //end

    int i; // needed to declare this variable apparently

    for(i = 0; i < 6;) // for loop to loop through each
    {
        while ((UCSR0A & (1 << UDRE0)) == 0) // waits for next character - this is taken off of the writeup
        { 
            //wait yo
        }
        UDR0 = UDR0character[i++]; // Adds the character at i to the UDR0
    }
}

void RDfunct_convert(void)
{
    uint8_t checkremote_low; //needed to first check if in 50 90 thresthold
    uint8_t checkremote_high;
    
    RDBuffer[4] = '\0';

    sscanf(RDBuffer, "%2hhd%2hhd", &checkremote_low, &checkremote_high); //need the 2 infront of hhd because 2 numbers, also no space like it had on writeup because 5090 - no space
    
    
    if(checkremote_high >= checkremote_low) //first check if high is not less than low
    {
        //Then check values are in range-we can most likely assume but writeup told me to so
        if ((checkremote_high >= 50 && checkremote_high <= 90) && (checkremote_low >= 50 && checkremote_low <= 90)) 
        {
            RemoteOn = 1; //basically says that the remote option is available for main to allow it to pop up on prints
            valRD_low = checkremote_low;
            valRD_high = checkremote_high;
        }
    }
}

ISR(USART_RX_vect) //directions diretcly from writeup section just followed logic and worked :)
{
    char receivedUDR0 = UDR0;

    if(receivedUDR0 == '<') //cheks if starting
    {
        RDStarted = 1; //show that it started
        RDRecievedData = 0; //count to 0
        RDEnding = 0;
    }
    else if(RDStarted == 1)
    {
        if((receivedUDR0 <= '9' && receivedUDR0 >= '0'))
        {
            if(RDRecievedData < 4) //resets after 4th character
            {
                RDBuffer[RDRecievedData] = receivedUDR0;
                RDRecievedData++;
            }
            else //seen from if above
            {
                RDRecievedData = 0;
                RDStarted = 0;
            }
        }
        else if(receivedUDR0 == '>')
        {
            if(RDRecievedData > 0) //close the data given
            {
                RDEnding = 1; //set to end
            }
            RDRecievedData = 0; //resets at end of >  even if <> - just better check
            RDStarted = 0; 
        }
        else //not any of numbers or >
        {
            RDRecievedData = 0;
            RDStarted = 0;
        }
    }
}