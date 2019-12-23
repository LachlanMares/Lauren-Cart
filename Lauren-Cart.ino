#include <Wire.h>
#include <Servo.h>

#define parameter_select            2
#define high_switch                 4           
#define low_switch                  5
#define reverse_switch              6           
#define neutral_switch              7
#define brake_switch                A0 
#define esc_pin                     9

#define set_0_minimum_uS            75
#define set_0_reverse_uS            1250
#define set_0_lowspeed_uS           1550
#define set_0_highspeed_uS          1600

#define set_1_minimum_uS            75
#define set_1_reverse_uS            1050
#define set_1_lowspeed_uS           1750
#define set_1_highspeed_uS          2000

#define neutral_uS                  1400
#define steps                       20

Servo esc;  

boolean newMode = false;
boolean parameterSet = false;
boolean paramsSwitch = false;
boolean brakeOn = true;
boolean highSwitch = false;
boolean lowSwitch = false;
boolean forward = false;
boolean reverse = false;
boolean neutral = false;

int state = 0; // 0=startup, 1=reverse, 2=neutral, 3=forward
int step_counter = 0;
int minimum_uS, reverse_uS, lowspeed_uS, highspeed_uS;
int current_uS, reverse_step_size, lowspeed_step_size, highspeed_step_size;

void setup() 
{ 
  Serial.begin(57600);
  esc.attach(esc_pin);  
  esc.writeMicroseconds(neutral_uS);  

  pinMode(parameter_select, INPUT_PULLUP);
  pinMode(high_switch, INPUT_PULLUP);
  pinMode(low_switch, INPUT_PULLUP);
  pinMode(reverse_switch, INPUT_PULLUP);
  pinMode(neutral_switch, INPUT_PULLUP);
  pinMode(brake_switch, INPUT_PULLUP); 
}

void loop() 
{
  paramsSwitch = !digitalRead(parameter_select);
  
  if(paramsSwitch != parameterSet)
  {
    if(paramsSwitch)
    {
      parameterSet = true;
      minimum_uS = set_0_minimum_uS;
      reverse_uS = set_0_reverse_uS;
      lowspeed_uS = set_0_lowspeed_uS;
      highspeed_uS =  set_0_highspeed_uS;
    } else
      {
        parameterSet = false;
        minimum_uS = set_1_minimum_uS;
        reverse_uS = set_1_reverse_uS;
        lowspeed_uS = set_1_lowspeed_uS;
        highspeed_uS =  set_1_highspeed_uS;
      }
    
    reverse_step_size = int((neutral_uS - (reverse_uS + minimum_uS)) / steps);
    lowspeed_step_size = int(((lowspeed_uS - minimum_uS) - neutral_uS) / steps);
    highspeed_step_size = int(((highspeed_uS - minimum_uS) - neutral_uS) / steps);
  }
  
  highSwitch = !digitalRead(high_switch);
  lowSwitch = !digitalRead(low_switch);
  reverse = !digitalRead(reverse_switch);
  forward = (highSwitch || lowSwitch);
  neutral = !digitalRead(neutral_switch);
  brakeOn = !digitalRead(brake_switch);
        
  switch(state)
  {
    case 0:   
      if(brakeOn)
      {
        state = reverse ? 1 : state;
        state = neutral ? 2 : state;
        state = forward ? 3 : state;
      } 
      break;

    case 1:
      if(reverse)
      {
        if(brakeOn)
        {
          current_uS = neutral_uS; 
          step_counter = 0;
        } else 
          {
            if (step_counter < steps)
            {
              if(step_counter == 0)
              {
                current_uS = neutral_uS - minimum_uS;
              } else
                {
                  current_uS -= reverse_step_size;
                }
              step_counter++;
            } else
              {
                current_uS = reverse_uS;
              }
          } 
      } else
        {
          state = 0;
          current_uS = neutral_uS;
          step_counter = 0;
        }
      break;
        
    case 2:     
      if(neutral)
      {
        current_uS = neutral_uS;    
      } else
        {
          state = 0;
          current_uS = neutral_uS;
          step_counter = 0;
        }
      break;

    case 3:     
      if(forward)
      {
        if(brakeOn)
        {
          current_uS = neutral_uS;
          step_counter = 0;    
        } else 
          {
            if(lowSwitch)
            {
              if (step_counter < steps)
              {
                if(step_counter == 0)
                {
                  current_uS = neutral_uS + minimum_uS;
                } else
                  {
                    current_uS += lowspeed_step_size;
                  }
                step_counter++;
              } else
                {
                  current_uS = lowspeed_uS;
                }
            } else if(highSwitch)
              {
                if (step_counter < steps)
                {
                  if(step_counter == 0)
                  {
                    current_uS = neutral_uS + minimum_uS;
                  } else
                    {
                      current_uS += highspeed_step_size;
                    }
                  step_counter++;
                } else
                  {
                    current_uS = highspeed_uS;
                  }    
              }
          } 
      } else
        {
          state = 0;
          current_uS = neutral_uS;
          step_counter = 0;
        }
      break;
  }
  esc.writeMicroseconds(current_uS);
  delay(100); 
}

