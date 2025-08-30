#include <avr/io.h>
#include <avr/interrupt.h>
#include "project.h"

//one function for serial
void DataTransmit(void);  // Function to transmit temperature settings over serial
void RDfunct_convert(void); //converts to remote high and low values
