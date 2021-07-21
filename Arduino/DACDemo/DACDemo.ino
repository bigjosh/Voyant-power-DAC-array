#include <SPI.h>

#define SERIAL_BAUD 1000000 // Speed of connection to host

#define FULL_VOLTS 10.0   // NUmber of volts at full + scale of DAC


//Method to write to the DAC,using  digitalWrite for slave select
void writeMCP492x(const uint16_t data, const uint8_t slave_select) {
  // Take the top 4 bits of config and the top 4 valid bits (data is actually a 12 bit number) 
  //and OR them together
  const uint8_t top_msg = (0x30 & 0xF0) | (0x0F & (data >> 8));

  // Take the bottom octet of data
  const uint8_t lower_msg = (data & 0x00FF);

  // Select our DAC, Active LOW
  digitalWrite(slave_select, LOW);

  // Send first 8 bits
  SPI.transfer(top_msg);
  // Send second 8 bits
  SPI.transfer(lower_msg);

  //Deselect DAC
  digitalWrite(slave_select, HIGH);
}

static const uint16_t dac_full_val = 4095;   // Max 12 bit number, (1<<12)-1   

struct powerboard_t {

  const char tag;      // index we use to refer to it in protocol

  const uint8_t pin;        // CS pin
  const uint16_t dac_zero;  // DAC value to 0V

  void setup() const {
    // set the slaveSelectPins as an output:
    pinMode (pin , OUTPUT);
  }

  // Set in DAC steps
  void setSteps( const uint16_t s ) const {
    writeMCP492x( s , pin );    
  }

  // Set to voltage
  void setVolts( const float v ) const {
    uint16_t s = ( ( ( dac_full_val - dac_zero )  / FULL_VOLTS ) * v) + dac_zero; 
    setSteps( s );
  }

  
};

// The 1st number is the single char tag that is used to refer to this board in the protocol
// The 2nd number is the digital pin that is connected to the board's ~CS line
// The 3rd number is the DAC setting to results in a 0 volt output. This is an emperically measured number for each board.

powerboard_t powerboards[] = {

  {'1' , 10 , 2056 } ,
  {'2' ,  9 , 2056 } ,
  {'3' ,  8 , 2056 } ,
  {'4' ,  7 , 2056 } ,
  {'5' ,  6 , 2056 } ,
  {'6' ,  5 , 2056 } 
      
};


void setup() {

  // initialize SPI: 
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);   
  
  // set the slaveSelectPins as an output:
  for( const auto& powerboard : powerboards) {
    powerboard.setup();
    powerboard.setVolts(0.0);
  }

  Serial.begin( SERIAL_BAUD );
  Serial.println( "Command? Ex 'V24.5'=set board #2 to 4.5 volts (0.00-10.00), 'S54095'=Set board #5 to DAC step 4095 (0-4095), 'D500'=delay 500ms");

  
}

// It bugs me as much as it bug you to see this redundant code here, but somehow Arduino builder
// manages to mangle a functor I tried to put here. So after an hour wasted, I am giving in. 

// Set the direct DAC value. Returns 0=Success, 1=Tag not found

uint8_t setSteps( char tag , uint16_t steps  ) {

  for( const auto& powerboard : powerboards) {

    if ( powerboard.tag == tag ) {

      writeMCP492x( steps , powerboard.pin );
      return(0);      
      
    }
  }

  return(1);
  
}


uint16_t mapf2u16(float x, float in_min, float in_max, uint16_t out_min, uint16_t out_max) {
  return round(  (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min );
}


// Set the nearest voltage. Returns 0=Success, 1=Tag not found

uint8_t setVolts( char tag , float volts ) {

  for( const auto& powerboard : powerboards) {

    if ( powerboard.tag == tag) {

      uint16_t steps = mapf2u16( volts , 0.00 , 10.00 ,  powerboard.dac_zero , dac_full_val );

      //Serial.println( steps );

      writeMCP492x( steps , powerboard.pin );
      return(0);
            
    }
    
  }

  return(1);

}


void processLine( const char *s ) {

  const char command = s[0];

  switch (command) {
    
    case 'V': {   // Volts

       if (strlen(s )< 3) {
         Serial.println("1-ERR_SHORT");
         return;
       }
        
       char tag = s[1]; 
       float volts = atof( s+2 );  

       //Serial.println(volts);

       if (!setVolts( tag , volts )) {           
         Serial.println("0-SUCCESS");
       } else {
         Serial.println("2-ERR_TAG");          
       }
       break;
    }
  
    case 'S': {   // Steps

       if (strlen(s )< 3) {
         Serial.println("1-ERR_SHORT");
         return;
       }
        
       char tag = s[1]; 
       uint16_t steps = atoi( s+2 );  

       Serial.println(steps);

       if (!setSteps( tag , steps )) {           
         Serial.println("0-SUCCESS");
       } else {
         Serial.println("2-ERR_TAG");          
       }
       break;
    }
    
    case 'D': {   // Delay
      
       if (strlen(s )< 2) {
         Serial.println("1-ERR_SHORT");
         return;
       }

       uint16_t ms = atoi( s+1 );  
       Serial.println("0-SUCCESS");       
       delay(ms); 
       break;  
    }
    
    default: {
      Serial.println("3-ERR_CMD");      
      break;
    }
    
  }

}

#define MAX_INPUT_LINE_LEN 10
char inputLine[MAX_INPUT_LINE_LEN+1]; // Leave room for terminating null
uint8_t inputLineLen=0;

void loop() {

  if (Serial.available()) {

    char c = Serial.read(); 

    if (c=='\n' || c=='\r') {   // DOn't pass emptry string. This lets us be \n \r agnostic and always ignore the 2nd one. 

      if (inputLineLen>0) {
      
        inputLine[inputLineLen]=0x00;
        processLine( inputLine );
        inputLineLen=0;
        
      }
      
    } else if (inputLineLen<MAX_INPUT_LINE_LEN) {
      
      inputLine[inputLineLen++]=c;      
      
    }
    
  }
}
  
  
