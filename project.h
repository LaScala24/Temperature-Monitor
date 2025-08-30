#define MINIMUM_PWM 12 //minimum pwm
#define MAXIMUM_PWM 35 //maximum pwm
#define RANGE_PWM 23  //range of pwm: max - min 
#define FOSC 16000000 //Clock Frequency
#define BAUD 9600 //Baud rate used
#define MYUBRR (FOSC/16/BAUD-1) //Value for UBRR0

//my global variables
extern volatile uint8_t Low_EEPROM; //lowest eeprom value possible
extern volatile uint8_t High_EEPROM; //highst eeprom value possible
extern volatile uint8_t old_state, new_state; //state variables (same as used in lab6)
extern volatile uint8_t a, b; // a and b variables to use as logic
extern volatile uint8_t high_set; //set the original temp change option at the high point, press button to see if 
extern volatile uint8_t prev_button_press;
extern volatile uint8_t lowvalue; //initial high/low values
extern volatile uint8_t highvalue; 
extern volatile uint16_t T2_count; //count variable
extern volatile uint8_t servo_pwm; //initial pwm
extern volatile uint8_t servo_move; //initialize the move to 0-dont move
extern volatile uint8_t local_set;         //initializes it in local mode
extern volatile uint8_t prev_RLbutton_press;  //tracks when button is released 
extern volatile uint8_t prev_right_press;  //tracks when right button is released helper variable
extern volatile uint8_t start_send;

//for checkpoint 3 in order of needed variable creations
extern volatile char RDBuffer[5]; //buffer with 5 bytes
extern volatile uint8_t RDStarted; //variable to check if data is starting
extern volatile uint8_t RDRecievedData;  //variable to tell how many data characters have been recieved
extern volatile uint8_t RDEnding; //acts as a data valid flag to check for > to basically see where it is ending
extern volatile uint8_t ActiveLow;  //helper for switching between romate and local low
extern volatile uint8_t ActiveHigh; //helper for switching between remote and local high
extern volatile uint8_t valRD_high; //remote data high val
extern volatile uint8_t valRD_low; //remote data low val
extern volatile uint8_t RemoteOn; //remote values are active 

void debounce(uint8_t bit); //debounce function
void LCDlocal(char *buf); //function for displaying local values
void LCDremote(char *buf); //function for displaying remote values