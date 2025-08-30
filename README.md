# Temperature-Monitor
Temperature Monitor Project Device

Introduction:

I built a room temperature monitor using the ATmega328P that reads a DS18B20 sensor, displays °F with 0.1 precision on the LCD, and utilizes RGB LEDs, a rotary encoder, EEPROM, a servo countdown dial, and an RS-232 link to a second unit. Everything is done with fixed-point integer math. No floats.

Overview:

The device samples the DS18B20 over a 1-Wire bus on PC1, converts the 12-bit value to Fahrenheit, and displays the temperature, as well as the current high and low thresholds, on the LCD at all times. A HI LO button selects which threshold the encoder adjusts. Green indicates in range, red means too hot, and blue means too cold. Green brightness is PWM-controlled and scales linearly between the thresholds. A servo runs a ten-second sweep whenever the temperature goes outside the range and pauses if it returns to normal. Thresholds can be sent and received over a 9600 baud serial link, and the user can choose to use local or remote thresholds.

Getting Started:

I created a dedicated project folder, pulled in code from earlier labs for LCD, encoder, timers, and then added ds18b20.c and ds18b20.h. I trimmed unused lab code to keep debugging focused. I read through the entire spec first, then implemented it in checkpoints.

Which Port Bits I used:

PB3 drives the servo PWM. PB4 enables the 74HCT125 tri-state buffer. PB5 is the HI LO button. PC1 is the DS18B20 data line with a 4.7 kΩ pull-up. PC2, PC3, and PC4 are blue, green, and red for the RGB LED. PC5 is the LOCAL REMOTE button. PD0 RX and PD1 TX handle serial. PD2 and PD3 are the encoder inputs. The LCD RIGHT button on PC0 is the SEND button, read as a digital input.

Checkpoint 1:

I verified the LCD by printing a splash screen with my name on it. I wired the DS18B20 with the required pull-up on PC1 and ran the provided sensor test to confirm clean readings. I finished the low-level 1-Wire routines in ds18b20.c and used the API calls ds_init, ds_convert, and ds_temp to acquire samples continuously. I combined the two bytes into a signed 16-bit value, converted it to Fahrenheit in fixed-point notation, and displayed one decimal place. I added the HI-LO button on PB5 with a pin change interrupt and proper debouncing, and I displayed which threshold is active on the LCD. I connected the encoder on PD2 and PD3 and adjusted the selected threshold between 50 and 90 while enforcing low ≤ high.

Checkpoint 2:

I added EEPROM persistence using the eeprom_read_byte and eeprom_update_byte functions. Upon boot, I validate the stored values and fall back to defaults if they are invalid. I wired the common-anode RGB LED with individual resistors and drove blue when below low, red when above high, and green when in range. I configured TIMER1 in fast PWM mode 15 with OCR1A as top and used the TIMER1_OVF_vect and TIMER1_COMPB_vect ISRs to bit-bang PWM on the chosen pin for the green LED. I mapped temperature to OCR1B so that low values map to dim and high values map to full brightness, clamping and handling the equal-threshold case. I added the servo on PB3 using TIMER2 overflow interrupts, computed the counts for a ten-second sweep, and advanced the pulse width from the minimum to the maximum across the interval. The sweep pauses if the temperature reenters the range.

Checkpoint 3:

I inserted the 74HCT125 on the RX path and controlled enable with PB4, so programming is never blocked. I configured USART0 for 9600 baud, 8N1, asynchronous. I implemented the SEND button on PC0 to transmit a packet with the format <LLHH>. I wrote a receiver using the RX complete interrupt (USART_RX_vect) that collects characters into a small buffer, watches for start and end markers, validates the digit count, rejects errors, and exposes a flag when a complete set is received. I added the LOCAL REMOTE button on PC5 with a pin change interrupt to toggle which thresholds are in use and updated LEDs and servo behavior immediately when the selection changes. I verified loopback and timing on the scope.

Software Organization:

I split the code across multiple modules. lcd.c lcd.h handle the display. ds18b20.c ds18b20.h handle the sensor. encoder.c encoder.h handle the rotary logic and debouncing. timers.c timers.h handle TIMER1 PWM and TIMER2 servo timing. serial.c serial.h handle TX polling and RX ISR buffering with protocol parsing. project.c project.h hold globals, state, and the main loop. The Makefile lists all object dependencies, so editing a header rebuilds the right units.

Build and Flash:

make builds the project and makes flash programs for the board. Artifacts like .hex and .o are not required in source control. I keep them ignored or in a build folder.

-
-
- (Hardware Instructions)
-
-
'''
Pin Map (if needed):

Port.Bit  Arduino  Dir  Function                     Notes
PC1       A1       I/O  DS18B20 DQ                   4.7k pull up to 5V (One Wire)
PB5       D13      In   HI/LO button                 internal pull up, pin change ISR
PD2       D2       In   Encoder A                    pull up, INT0 or pin change ISR
PD3       D3       In   Encoder B                    pull up, INT1 or pin change ISR
PC5       A5       In   LOCAL/REMOTE button          internal pull up, pin change ISR
PC0       A0       In   SEND button (LCD RIGHT)      read as digital, LCD drives 0V when pressed
PD0       D0       In   UART RX                      9600 8N1, through 74HCT125 buffer
PD1       D1       Out  UART TX                      9600 8N1, TTL levels
PB4       D12      Out  74HCT125 enable              active low enable, external pull up keeps disabled during programming
PB3       D11      Out  Servo PWM                    Timer2 OC2A, ~16.4 ms period, 0.75..2.25 ms pulse
PC2       A2       Out  RGB LED Blue                 common anode, 240 ohm series resistor
PC3       A3       Out  RGB LED Green                PWM via Timer1 ISRs, 1.5k series resistor
PC4       A4       Out  RGB LED Red                  240 ohm series resistor

'''

Power and ground
Use one common ground for Arduino, breadboard rails, DS18B20, RGB LED, servo, and 74HCT125
Keep the One Wire lead short and tidy

Timers and interrupts
Timer1 fast PWM mode 15 using OVF and COMPB ISRs to toggle the green LED pin
Timer2 overflow ISR steps the servo pulse width across 10 s
Pin change ISRs on PB5 (HI/LO), PC5 (LOCAL/REMOTE), PC0 (SEND); encoder uses INT0/INT1 or pin change ISR

-
-
- (End)
-
-

Operation Summary:

The temperature always shows on the LCD with one decimal. The encoder adjusts only the selected threshold, and all limits are enforced. Green brightness ramps between low and high. Red and blue indicate out of range. The servo runs a ten-second sweep on any out-of-range condition and pauses when back in range. Pressing SEND transmits thresholds as ASCII. Received thresholds display on the LCD only if valid. LOCAL REMOTE selects which set the device uses.

Notes:

All math related to temperature and thresholds is based on integers. No floating point #'s. Wiring follows the required port map, allowing the program to run on any instructor board.

(USC > UCLA) ---> :)
