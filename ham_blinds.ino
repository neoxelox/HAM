//  BlindController by @Neoxelox
//  Rev. 1 | Last modification: 12/08/2017

//  Libraries Section:

//  Sleep for ATtiny85
  #include <avr/sleep.h>
  #include <avr/interrupt.h>

//  IR Receiver
  #include "IRLremote.h"
  
//------------------\\

//  Define Section:

  #define irInterrupt 0
  #define bupPin 0
  #define bstPin 1
  #define bdwPin 4

//------------------\\

//  Variable Section:

//  IR Receiver
uint8_t IRProtocol = 0; //Variables to store IR information
uint16_t IRAddress = 0;
uint32_t IRCommand = 0;

//------------------\\

void setup() 
{
  
  //  Transistors 
  pinMode(bupPin, OUTPUT);  //Prepare Pin to control the transistors
  pinMode(bstPin, OUTPUT);
  pinMode(bdwPin, OUTPUT);
  digitalWrite(bupPin, LOW); //Puts all the transistor on low
  digitalWrite(bstPin, LOW);
  digitalWrite(bdwPin, LOW);
 
  //  IR Receiver
  IRLbegin<IR_USER>(irInterrupt); //Initialize the IR Receiver
  
}

void loop() 
{
  sleep();

  delay(500); //The CPU needs that delay to handle the IR signal

  if (IRLavailable()) 
  {
    
    IRProtocol = IRLgetProtocol();  //Store the IR data
    IRAddress = IRLgetAddress();
    IRCommand = IRLgetCommand();
    
    if (IRAddress == 18509)
    {
      
      switch (IRCommand)
      {
        case 9180:  //Sent Blinds UP
        
        digitalWrite(bupPin, HIGH);
        delay(100);
        digitalWrite(bupPin, LOW);
        
        case 1020:  //Sent Blinds DOWN
        
        digitalWrite(bdwPin, HIGH);
        delay(100);
        digitalWrite(bdwPin, LOW);
        
        case 8670:  //Sent Blinds STOP
        
        digitalWrite(bstPin, HIGH);
        delay(100);
        digitalWrite(bstPin, LOW);
        
      }
    
    }
       
    IRLreset(); // Resume reading to get new values
    
  }
  
  
}

void decodeIR(const uint32_t duration) 
{
  // Called when directly received and interrupt CHANGE
  // Do not use Serial inside, it can crash your Arduino!
  
  decodeNec(duration);  //Only decode NEC to save flash space
  
}

void sleep() 
{

    GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
    PCMSK |= _BV(PCINT2);                   // Use PB2 as interrupt pin
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

    sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
    sei();                                  // Enable interrupts
    sleep_cpu();                            // sleep

    //Wake up after an interrupt
    
    cli();                                  // Disable interrupts
    PCMSK &= ~_BV(PCINT2);                  // Turn off PB2 as interrupt pin
    sleep_disable();                        // Clear SE bit
    ADCSRA |= _BV(ADEN);                    // ADC on

    sei();                                  // Enable interrupts
    
}

ISR(PCINT0_vect){}  //It is required to wake up the cpu