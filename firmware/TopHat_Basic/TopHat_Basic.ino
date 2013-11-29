/*
 Started: 6-16-2013
 Spark Fun Electronics
 Nathan Seidle
 
 Light Up Top Hat with Accelerometer - when movement is detected the LEDs light up in a way that makes it
 look like the lights are moving. They slow down as the hat stops moving.
 
 This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Orignally designed and built by Diana Eng and Dave for my wedding.
 You two are amazing. I can't think you enough! I love this hat!
 
 Mainboard is a Arduino Pro Mini at 3.3V/8MHz. Accelerometer is a ADXL335, analog output.

*/

#include <SoftPWM.h>

const int chan0 = 2;
const int chan1 = 3;
const int chan2 = 4;
const int chan3 = 5;
const int chan4 = 6;
const int chan5 = 7;
const int chan6 = 8;
const int chan7 = 9;

const int accelX = A3;
const int accelY = A2;
const int accelZ = A1;

#define SINGLE 1
#define ALL_BUT 2

float aX, aY, aZ;

int brightLevel = 9; //5% is still very noticeable. 40% or more is basically full brightness

const int statLED = A0;

//Timers
//==============
long startTime, endTime;
long lightSwitch, checkDirection;
//==============

//Growth/decay control
//==============
long maxTimeBetween = 3500; //In ms. Max time of no movement before we stop rotation completely
float shortestDelay = 10; //In ms. Min amount of time = fastest spinning that looks good. Don't go below about 25ms.
float timeBetweenChannelChange = 0;

//Pick your growth rate. Found by spreadsheet
//float growthRate = 1.0015; //for 2000ms
float growthRate = 1.00086; //for 3500ms
//float growthRate = 1.0006; //for 5000ms
//==============

float currentAccelReading = 0; //Used to see if there is new movement
int lightUpMode = SINGLE; //Start hat up in single LED mode
float oldMag = 938.0;
int channelNum = 0;

//Define clock wise and counter clock wise directions
#define CW 0
#define CCW 1

int lightDirection = CW;

void setup()
{
  setupLEDs();
  
  Serial.begin(57600);
  Serial.println("Welcome TopHat!");

  pinMode(accelX, INPUT);
  pinMode(accelY, INPUT);
  pinMode(accelZ, INPUT);

  startTime = millis() - 50; //Start the clock!
  
  lightSwitch = millis(); //Start clock for lighting type single/all_but
  
  checkDirection = millis(); //Start clock for checking the direction override
  
  randomSeed(analogRead(A0)); //Feed the chaos
  
  //lightUpMode = ALL_BUT; //Start in all on but one, mode.

  //Decide to switch directions
  lightDirection = random(0, 2);
  Serial.print("direction:");
  Serial.println(lightDirection);
}

void loop()
{
  //Check to see if we should change the display type
  if(millis() - lightSwitch > 60000)
  {
    //Flop the variable
    if(lightUpMode == SINGLE) lightUpMode = ALL_BUT;
    else lightUpMode = SINGLE;

    //reverse the current lights
    reverseLights(lightUpMode);
    
    lightSwitch = millis(); //Reset timer
  }

  long currentTime = millis() - startTime;
  timeBetweenChannelChange = shortestDelay * pow(growthRate, currentTime); //Exponentially grow the delay between changes

  //Change channels
  if(timeBetweenChannelChange < maxTimeBetween)
  {
    if(lightUpMode == SINGLE)
    {
      turnOffChannel(channelNum);

      if(lightDirection == CW)
      {
        channelNum++;
        if(channelNum > 7) channelNum = 0; //Wrap variable
      }
      else
      {
        channelNum--;
        if(channelNum < 0) channelNum = 7; //Wrap variable
      }

      turnOnChannel(channelNum);
    }
    else if(lightUpMode == ALL_BUT)
    {
      
      //Light up 4 channels
      turnOnChannel(channelNum);
      /*for(int x = 0 ; x < 4 ; x++)
      {
        int chanToChange = channelNum + x;
        if(chanToChange > 7) chanToChange -= 8;
        
        turnOnChannel(chanToChange);
      }*/
      
      if(lightDirection == CW)
      {
        channelNum++;
        if(channelNum > 7) channelNum = 0; //Wrap variable
      }
      else
      {
        channelNum--;
        if(channelNum < 0) channelNum = 7; //Wrap variable
      }

      //Turn off 4 channels
      turnOffChannel(channelNum);
      /*for(int x = 0 ; x < 4 ; x++)
      {
        int chanToChange = channelNum + x;
        if(chanToChange > 7) chanToChange -= 8;
        
        turnOffChannel(chanToChange);
      }*/

    }

    if(timeBetweenChannelChange > 300)
    {
      //Serial.print(" Slow check ");
      //Once tBCC becomes large, we need to check the accel a lot while we wait
      
      int numberOfChecks = timeBetweenChannelChange / 25;
      //Serial.print(" checks:");
      //Serial.print(numberOfChecks);
      
      for(int x = 0 ; x < numberOfChecks ; x++)
      {
        currentAccelReading = getAccel();
        if(currentAccelReading > 0)
        {
          speedUpSpin(); //Speed things up!
          break;
        }

        delay(timeBetweenChannelChange / numberOfChecks);
      }
    }
    else
    {
      delay(timeBetweenChannelChange); //Do nothing while this tBCC is run through
    }

  }

  /*Serial.print("currentTime: ");
  Serial.print(currentTime);
  Serial.print(" timeBetween channel change: ");
  Serial.print(timeBetweenChannelChange, 4);
  Serial.println();*/

  currentAccelReading = getAccel(); //Check accel for new energy into the system

  //If the accel is reporting moving, decrease tBCC by decreasing currentTime some amount
  if(currentAccelReading > 0)
  {
    if(currentTime > maxTimeBetween)
    {
      speedUpSpin(); //Reset things entirely
      //Serial.println("Movement from static");
      
      //Decide to switch directions
      //lightDirection = random(0, 2);
    }
    else
    {
      speedUpSpin();
      //Serial.println("startTime increase of 100ms");
    }
  }

}

void speedUpSpin()
{
  Serial.println(" speed up ");
  
  //Check to see if we should override the reading and change the direction
  if(millis() - checkDirection < 10000)
  {
    startTime = millis(); //Speed things up!  
  }
  else //Don't speed up, and also don't change direction
  {
    Serial.print(" no speed ");
    
    //Wait for spin down to complete
    if(timeBetweenChannelChange > maxTimeBetween)
    {
      Serial.print(" direction change ");
      if(lightDirection == CW) lightDirection = CCW;
      else lightDirection = CW;
      
      checkDirection = millis(); //Reset timer
    }
  }

}

//Reads the acces and returns an 'energy' value
float getAccel()
{
  float avgMag = 0;
  for(int x = 0 ; x < 8 ; x++)
  {
    aX = analogRead(accelX);
    aY = analogRead(accelY);
    aZ = analogRead(accelZ);

    float magnitude = sqrt((aX * aX) + (aY * aY) + (aZ * aZ)); //Combine all vectors 
    avgMag += magnitude;
  }
  avgMag /= 8;

  //Serial.print(" avgMag:");
  //Serial.print(avgMag);

  float magDifference = abs(oldMag - avgMag);

  if(magDifference > 10) //10 works well. Increase to desensitize the hat.
  {
    oldMag = avgMag; //Remember this new mag
    return(magDifference);
  } 
  else
    return(0);
}

float floatMap(float x, float inMin, float inMax, float outMin, float outMax){
  return (x-inMin)*(outMax-outMin)/(inMax-inMin)+outMin;
}

//Rotates in a given direction (CW or CCW), with a given speed
void rotate(int dir, int spd)
{
  if(dir == CCW)
  {
    for(int x = 0 ; x < 8 ; x++)
    {
      turnOffChannel(x);
      delay(50);
      turnOnChannel(x);
    }
  }
  else
  {
    for(int x = 8 ; x > 0 ; x--)
    {
      //lightChannel(x - 1);//, spd);
    }
  }
}

//Light a given channel
void turnOnChannel(int channel)
{
  switch(channel)
  {
    case(0): 
    SoftPWMSetPercent(chan0, brightLevel); 
    break;
    case(1): 
    SoftPWMSetPercent(chan1, brightLevel); 
    break;
    case(2): 
    SoftPWMSetPercent(chan2, brightLevel); 
    break;
    case(3): 
    SoftPWMSetPercent(chan3, brightLevel); 
    break;
    case(4): 
    SoftPWMSetPercent(chan4, brightLevel); 
    break;
    case(5): 
    SoftPWMSetPercent(chan5, brightLevel); 
    break;
    case(6): 
    SoftPWMSetPercent(chan6, brightLevel); 
    break;
    case(7): 
    SoftPWMSetPercent(chan7, brightLevel); 
    break;
  }

}

//Turn off a given channel
void turnOffChannel(int channel)
{
  switch(channel)
  {
    case(0): 
    SoftPWMSetPercent(chan0, 0); 
    break;
    case(1): 
    SoftPWMSetPercent(chan1, 0); 
    break;
    case(2): 
    SoftPWMSetPercent(chan2, 0); 
    break;
    case(3): 
    SoftPWMSetPercent(chan3, 0); 
    break;
    case(4): 
    SoftPWMSetPercent(chan4, 0); 
    break;
    case(5): 
    SoftPWMSetPercent(chan5, 0); 
    break;
    case(6): 
    SoftPWMSetPercent(chan6, 0); 
    break;
    case(7): 
    SoftPWMSetPercent(chan7, 0); 
    break;
  }

}

//Looks at the current light setup and reverses it
void reverseLights(boolean currentSetup)
{
  if(currentSetup == SINGLE)
  {
    for(int x = 0 ; x < 8 ; x++)
    {
      if(x == channelNum) turnOnChannel(x);
      else turnOffChannel(x);
    }
  }
  else if(currentSetup == ALL_BUT)
  {
    for(int x = 0 ; x < 8 ; x++)
    {
      if(x == channelNum) turnOffChannel(x);
      else turnOnChannel(x);
    }
  }
}

void setupLEDs()
{
  SoftPWMBegin();

  SoftPWMSet(chan0, 0); //Setup this pin to be controlled with SoftPWM. Initialize to zero.
  SoftPWMSet(chan1, 0);
  SoftPWMSet(chan2, 0);
  SoftPWMSet(chan3, 0);
  SoftPWMSet(chan7, 0);
  SoftPWMSet(chan5, 0);
  SoftPWMSet(chan6, 0);
  SoftPWMSet(chan7, 0);

  SoftPWMSetFadeTime(ALL, 0, 0);
  //SoftPWMSetFadeTime(ALL, 100, 100);

  for(int x = 0 ; x < 8 ; x++)
    turnOnChannel(x);

  delay(500);

  for(int x = 0 ; x < 8 ; x++)
    turnOffChannel(x);
}
