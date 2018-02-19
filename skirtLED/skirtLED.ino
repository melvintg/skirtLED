
// PIN DEFINITION
#define PIN_RX 0
#define PIN_TX 1

#define PIN_LED_R1 2
#define PIN_LED_G1 3
#define PIN_LED_B1 4

#define PIN_LED_R2 5
#define PIN_LED_G2 6
#define PIN_LED_B2 7

#define PIN_LED_R3 8
#define PIN_LED_G3 12
#define PIN_LED_B3 13

#define PIN_LED_FADE_R 9
#define PIN_LED_FADE_G 10
#define PIN_LED_FADE_B 11


#define PIN_BUTTON1 A1
#define PIN_BUTTON2 A0
#define PIN_SWITCH A2
#define SPIN_AUDIO A3
#define SPIN_ACCEL A4

// FLAGS DEFINITION
#define FLAG_R 1
#define FLAG_G 2
#define FLAG_B 4

#define FLAG_LVL1 1
#define FLAG_LVL2 2
#define FLAG_LVL3 4

// MODE DEFINITION
#define MODE_INPUT_AUDIO 0
#define MODE_INPUT_ACCEL 1

#define MODE_OUTPUT_OFF 0
#define MODE_OUTPUT_ALL 1
#define MODE_OUTPUT_LVL 2
#define MODE_OUTPUT_LVL_RANDOM 3
#define MODE_OUTPUT_COLOUR 4
#define MODE_OUTPUT_FADE 5

// LEVEL INTENSITY DEFINITION
// Up to 5 levels for ALL COLOUR NOT RANDOM Mode. (RGB)
#define INT1_LVL_1 1
#define INT1_LVL_2 3
#define INT1_LVL_3 5
#define INT1_LVL_4 6
#define INT1_LVL_5 8

#define INT2_LVL_1 2
#define INT2_LVL_2 5
#define INT2_LVL_3 7
#define INT2_LVL_4 9
#define INT2_LVL_5 12

#define INT3_LVL_1 3
#define INT3_LVL_2 6
#define INT3_LVL_3 8
#define INT3_LVL_4 12
#define INT3_LVL_5 15

#include <Wire.h>

// Variable Level Intensity
byte intLVL1;
byte intLVL2;
byte intLVL3;
byte intLVL4;
byte intLVL5;
byte intLVL;

// Variables
byte iter;
int sAudioCte;
int sAudioValueMin;

//Declaring accelerometer global variables
int gyro_x, gyro_y, gyro_z;
long acc_x, acc_y, acc_z;
int temperature;
long acc_x_cal;
long loop_timer;

// States
byte colourMode;
byte inputMode;
byte outputMode;

void setup() {

  // Used for init data protocols (aka. SPI)
  Wire.begin();
  // Initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // Setup accel registers, hardware config.
  setup_mpu_6050_registers();

  // Initialize Pins
  pinMode(PIN_LED_R1, OUTPUT);
  pinMode(PIN_LED_G1, OUTPUT);
  pinMode(PIN_LED_B1, OUTPUT);
  pinMode(PIN_LED_R2, OUTPUT);
  pinMode(PIN_LED_G2, OUTPUT);
  pinMode(PIN_LED_B2, OUTPUT);
  pinMode(PIN_LED_R3, OUTPUT);
  pinMode(PIN_LED_G3, OUTPUT);
  pinMode(PIN_LED_B3, OUTPUT);
  pinMode(PIN_LED_FADE_R, OUTPUT);
  pinMode(PIN_LED_FADE_G, OUTPUT);
  pinMode(PIN_LED_FADE_B, OUTPUT);

  // Initialize States
  colourMode = FLAG_R + FLAG_G + FLAG_B;
  inputMode = MODE_INPUT_AUDIO;
  outputMode = MODE_OUTPUT_ALL;

  // Initialize Variables
  iter = 0;
  sAudioCte = 0;
  sAudioValueMin = 255;

  // Initialize Intensity Level Variables
  intLVL = 2;
  intLVL1 = INT2_LVL_1;
  intLVL2 = INT2_LVL_2;
  intLVL3 = INT2_LVL_3;
  intLVL4 = INT2_LVL_4;
  intLVL5 = INT2_LVL_5;

}

void loop() {
  checkModes();

  byte level = 0;
  // Input needed only in LVL and FADE modes. Check.
  if (outputMode != MODE_OUTPUT_ALL) {
    switch(inputMode) {
      case MODE_INPUT_AUDIO:
      // Update Cte each x loops
      sensorAudioUpdateCte(SPIN_AUDIO, 10);

      // Get Average of a x number of scans.
      level = sensorAudioAverage(sAudioCte, SPIN_AUDIO, 10);
      break;

      case MODE_INPUT_ACCEL:
      // Read accelerometer data

      level = sensorAccelAverage(10);
      break;
    }
  }

  switch(outputMode) {
    // Deprecated
    case MODE_OUTPUT_OFF:
    // State set to off in the change mode operation.
    // Do nothing...
    break;

    case MODE_OUTPUT_ALL:
    // Set all levels to display all
    level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
    // FALL-THROUGH (needed for the case user change colour)

    case MODE_OUTPUT_LVL:
    // Active Leds
    ledLevel(level);
    break;

    case MODE_OUTPUT_LVL_RANDOM:
    changeColour();
    // Active Leds
    ledLevel(level);
    break;

    case MODE_OUTPUT_COLOUR:
    // Active Leds
    ledLevel(level);
    break;

    case MODE_OUTPUT_FADE:
    // Active Leds
    ledFade(level);
    break;
  }

  // print out the value you read:
  //Serial.print("Input mode: ");
  //Serial.println(inputMode);
  //Serial.print("Output mode: ");
  //Serial.println(outputMode);
  delay(1);
}

void sensorAudioUpdateCte(int inputSPIN, byte nLoops) {
  // For each reCte interval (x loops), get the minumum audio value.
  int sAudioValue = analogRead(inputSPIN);
  if (sAudioValue < sAudioValueMin) {
    sAudioValueMin = sAudioValue;
  }
  // Update Audio Average Constant at the end of each reCte interval.
  iter++;
  if (iter == nLoops) {
    iter = 0;
    sAudioCte = sAudioValueMin;
    sAudioValueMin = 255;
  }
}

// Gets x scans of Audio Samples and get Aveage.
// Then, compares the average with the constant value and return a level.
byte sensorAudioAverage(int sAudioCte, int inputSPIN, int nScans) {
  int sensorAverage = 0;
  for(int i = 0; i < nScans; i++) {
    sensorAverage = sensorAverage + analogRead(inputSPIN);
    delay(1);
  }
  sensorAverage = sensorAverage / nScans;
  sensorAverage = sensorAverage - sAudioCte;

  Serial.print("Sensor Constant: ");
  Serial.println(sAudioCte);
  Serial.print("Average: ");
  Serial.println(sensorAverage);

  byte level = 0;
  if (outputMode == MODE_OUTPUT_COLOUR) {
    level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
    colourMode = 0;
    if ((sensorAverage > intLVL1) || (sensorAverage < -intLVL1)) {
      colourMode = FLAG_R;
    }
    if ((sensorAverage > intLVL2) || (sensorAverage < -intLVL2)) {
      colourMode = FLAG_G;
    }
    if ((sensorAverage > intLVL3) || (sensorAverage < -intLVL3)) {
      colourMode = FLAG_B;
    }
    if ((sensorAverage > intLVL4) || (sensorAverage < -intLVL4)) {
      colourMode = FLAG_G + FLAG_B;
    }
    if ((sensorAverage > intLVL5) || (sensorAverage < -intLVL5)) {
      colourMode = FLAG_R + FLAG_G + FLAG_B;
    }

  } else {
    if ((sensorAverage > intLVL1) || (sensorAverage < -intLVL1)) {
      level += FLAG_LVL1;
    }
    if ((sensorAverage > intLVL2) || (sensorAverage < -intLVL2)) {
      level += FLAG_LVL2;
    }
    if ((sensorAverage > intLVL3) || (sensorAverage < -intLVL3)) {
      level += FLAG_LVL3;
    }
  }
  Serial.print("Level: ");
  Serial.println(level);
  return level;
}

// Gets x scans of Accel Samples and get Aveage.
// Then, compares the average with the constant value and return a level.
byte sensorAccelAverage(int nScans) {
  int sensorAverage = 0;
  for(int i = 0; i < nScans; i++) {
    read_mpu_6050_data();
    acc_x -= acc_x_cal;
    sensorAverage += acc_x;
    delay(3);
  }
  sensorAverage /= nScans;

  Serial.print("Average: ");
  Serial.println(sensorAverage);

  byte level = 0;
  if (outputMode == MODE_OUTPUT_COLOUR) {
    level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
    colourMode = 0;
    if ((sensorAverage > 500) || (sensorAverage < -500)) {
      colourMode = FLAG_R;
    }
    if ((sensorAverage > 900) || (sensorAverage < -900)) {
      colourMode = FLAG_G;
    }
    if ((sensorAverage > 1300) || (sensorAverage < -1300)) {
      colourMode = FLAG_B;
    }
    if ((sensorAverage > 1800) || (sensorAverage < -1800)) {
      colourMode = FLAG_G + FLAG_B;
    }
    if ((sensorAverage > 2200) || (sensorAverage < -2200)) {
      colourMode = FLAG_R + FLAG_G + FLAG_B;
    }
  } else {
    if (sensorAverage > 500) {
      level += FLAG_LVL3;
    }
    if (sensorAverage > 1000) {
      level += FLAG_LVL2;
    }
    if (sensorAverage > 1500) {
      level += FLAG_LVL1;
    }

    if (sensorAverage < -500) {
      level += FLAG_LVL1;
    }
    if (sensorAverage < -1000) {
      level += FLAG_LVL2;
    }
    if (sensorAverage < -1500) {
      level += FLAG_LVL3;
    }
  }

  return level;
}

void ledLevel(byte level) {
  // For each colour activate or deactivate to change colour.
  if ((level & FLAG_LVL1) == FLAG_LVL1) {
    if((colourMode & FLAG_R) == FLAG_R)
      digitalWrite(PIN_LED_R1, HIGH);
    else
      digitalWrite(PIN_LED_R1, LOW);
    if((colourMode & FLAG_G) == FLAG_G)
      digitalWrite(PIN_LED_G1, HIGH);
    else
      digitalWrite(PIN_LED_G1, LOW);
    if((colourMode & FLAG_B) == FLAG_B)
      digitalWrite(PIN_LED_B1, HIGH);
    else
      digitalWrite(PIN_LED_B1, LOW);
  } else {
    digitalWrite(PIN_LED_R1, LOW);
    digitalWrite(PIN_LED_G1, LOW);
    digitalWrite(PIN_LED_B1, LOW);
  }

  if ((level & FLAG_LVL2) == FLAG_LVL2) {
    if((colourMode & FLAG_R) == FLAG_R)
      digitalWrite(PIN_LED_R2, HIGH);
    else
      digitalWrite(PIN_LED_R2, LOW);
    if((colourMode & FLAG_G) == FLAG_G)
      digitalWrite(PIN_LED_G2, HIGH);
    else
      digitalWrite(PIN_LED_G2, LOW);
    if((colourMode & FLAG_B) == FLAG_B)
      digitalWrite(PIN_LED_B2, HIGH);
    else
      digitalWrite(PIN_LED_B2, LOW);
  } else {
    digitalWrite(PIN_LED_R2, LOW);
    digitalWrite(PIN_LED_G2, LOW);
    digitalWrite(PIN_LED_B2, LOW);
  }

  if ((level & FLAG_LVL3) == FLAG_LVL3) {
    if((colourMode & FLAG_R) == FLAG_R)
      digitalWrite(PIN_LED_R3, HIGH);
    else
      digitalWrite(PIN_LED_R3, LOW);
    if((colourMode & FLAG_G) == FLAG_G)
      digitalWrite(PIN_LED_G3, HIGH);
    else
      digitalWrite(PIN_LED_G3, LOW);
    if((colourMode & FLAG_B) == FLAG_B)
      digitalWrite(PIN_LED_B3, HIGH);
    else
      digitalWrite(PIN_LED_B3, LOW);
  } else {
    digitalWrite(PIN_LED_R3, LOW);
    digitalWrite(PIN_LED_G3, LOW);
    digitalWrite(PIN_LED_B3, LOW);
  }
}

void ledFade(byte level) {
  int fade = 0;
  if ((level & (FLAG_LVL1 | FLAG_LVL2 | FLAG_LVL3)) == (FLAG_LVL1 | FLAG_LVL2 | FLAG_LVL3)) {
    fade = 255;
  } else if ((level & (FLAG_LVL1 | FLAG_LVL2)) == (FLAG_LVL1 | FLAG_LVL2)) {
    fade = 170;
  } else if ((level & FLAG_LVL1) == FLAG_LVL1) {
    fade = 85;
  }

  Serial.print("Fade Value: ");
  Serial.println(fade);

  // TODO
  // RFU
  // Get fade as global variable and update state with shorter changes depending of the new level.
  // Not only 3 PWM values, i.e. 255, then 200 if is lower and was 255.
  // RFU

  if ((colourMode & FLAG_R) == FLAG_R)
    analogWrite(PIN_LED_FADE_R, fade);
  if ((colourMode & FLAG_G) == FLAG_G)
    analogWrite(PIN_LED_FADE_G, fade);
  if ((colourMode & FLAG_B) == FLAG_B)
    analogWrite(PIN_LED_FADE_B, fade);
}

void ledSwitchOff() {
  digitalWrite(PIN_LED_R1, LOW);
  digitalWrite(PIN_LED_G1, LOW);
  digitalWrite(PIN_LED_B1, LOW);

  digitalWrite(PIN_LED_R2, LOW);
  digitalWrite(PIN_LED_G2, LOW);
  digitalWrite(PIN_LED_B2, LOW);

  digitalWrite(PIN_LED_R3, LOW);
  digitalWrite(PIN_LED_G3, LOW);
  digitalWrite(PIN_LED_B3, LOW);

  digitalWrite(PIN_LED_FADE_R, LOW);
  digitalWrite(PIN_LED_FADE_G, LOW);
  digitalWrite(PIN_LED_FADE_B, LOW);
}





// Check Buttons and SW states (previsualizing the state) and define input and output modes.
void checkModes() {
  byte level = 0;   // Determine how many levels are active (3 levels).
  boolean isDoubleButton = false;

  // Switch Input Mode (Audio or Accelerometer)
  if (digitalRead(PIN_SWITCH) == LOW) {
    inputMode = MODE_INPUT_AUDIO;
  } else {
    // If Input Mode was Audio Mode, perform some animation of change.
    if (inputMode == MODE_INPUT_AUDIO) {
      // Animation of Increment all leds and stays for calibration.
      level = FLAG_LVL1;
      ledLevel(level);
      delay(75);
      level += FLAG_LVL2;
      ledLevel(level);
      delay(75);
      level += FLAG_LVL3;
      ledLevel(level);

      accelCalibration();

      // Animation of fantastic car.
      level = FLAG_LVL1;
      ledLevel(level);
      delay(75);
      level = FLAG_LVL2;
      ledLevel(level);
      delay(75);
      level = FLAG_LVL3;
      ledLevel(level);
      delay(75);
      level = FLAG_LVL2;
      ledLevel(level);
      delay(75);
      level = FLAG_LVL1;
      ledLevel(level);
      delay(75);
      level = FLAG_LVL2;
      ledLevel(level);
      delay(75);
      level = FLAG_LVL3;
      ledLevel(level);
      delay(75);
      ledSwitchOff();
      Serial.println("MODE_INPUT_ACCEL");
    }
    inputMode = MODE_INPUT_ACCEL;

  }

  // Button1 (Change Mode)
  if (digitalRead(PIN_BUTTON1) == HIGH) {
    /*
    #define MODE_OUTPUT_OFF 0
    #define MODE_OUTPUT_ALL 1
    #define MODE_OUTPUT_LVL 2
    #define MODE_OUTPUT_LVL_RANDOM 3
    #define MODE_OUTPUT_COLOUR 4
    #define MODE_OUTPUT_FADE 5
    */
    outputMode++;
    // MODE_OUTPUT_FADE is in RFU state
    // Elimitate state button with the last active state. MODE_OUTPUT_LVL
    if (outputMode > MODE_OUTPUT_FADE)
      outputMode = MODE_OUTPUT_ALL;

    // Reset Leds
    ledSwitchOff();

    Serial.print("Change Mode: ");

    switch(outputMode) {
      // Deprecated
      case MODE_OUTPUT_OFF:
      // Leds are off, continue...
      Serial.println("MODE_OUTPUT_OFF");
      break;

      case MODE_OUTPUT_ALL:
      // Set all levels to display all
      level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
      // Set colour RGB
      colourMode = FLAG_R + FLAG_G + FLAG_B;
      // Active Leds
      ledLevel(level);
      Serial.println("MODE_OUTPUT_ALL");
      break;

      case MODE_OUTPUT_LVL:
      // Animation of incrementing and after decrementing levels (light wave).
      level = FLAG_LVL1;
      ledLevel(level);
      delay(75);
      level += FLAG_LVL2;
      ledLevel(level);
      delay(75);
      level += FLAG_LVL3;
      ledLevel(level);
      delay(75);
      level -= FLAG_LVL3;
      ledLevel(level);
      delay(75);
      level -= FLAG_LVL2;
      ledLevel(level);
      delay(75);
      level -= FLAG_LVL1;
      ledLevel(level);
      delay(75);
      ledSwitchOff();
      Serial.println("MODE_OUTPUT_LVL");
      break;


      case MODE_OUTPUT_LVL_RANDOM:
      // Animation of incrementing and after decrementing levels with RANDOM COLOUR (light wave).
      level = FLAG_LVL1;
      changeColour();
      ledLevel(level);
      delay(75);
      level += FLAG_LVL2;
      changeColour();
      ledLevel(level);
      delay(75);
      level += FLAG_LVL3;
      changeColour();
      ledLevel(level);
      delay(75);
      level -= FLAG_LVL3;
      changeColour();
      ledLevel(level);
      delay(75);
      level -= FLAG_LVL2;
      changeColour();
      ledLevel(level);
      delay(75);
      level -= FLAG_LVL1;
      changeColour();
      ledLevel(level);
      delay(75);
      ledSwitchOff();
      Serial.println("MODE_OUTPUT_LVL");
      break;

      case MODE_OUTPUT_COLOUR:
      // Animation of ALL LIGHTS ON in a RANDOM COLOUR way.
      level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
      colourMode = FLAG_R + FLAG_G + FLAG_B;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_R;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_R + FLAG_G;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_G;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_G + FLAG_B;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_B;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_R + FLAG_B;
      ledLevel(level);
      delay(75);
      colourMode = FLAG_R + FLAG_G + FLAG_B;
      ledLevel(level);
      delay(75);
      ledSwitchOff();
      break;

      case MODE_OUTPUT_FADE:
      // Animation of fadding LIGHTS in a level way.
      colourMode = FLAG_R + FLAG_G + FLAG_B;
      level = FLAG_LVL1;
      ledFade(level);
      delay(75);
      level += FLAG_LVL2;
      ledFade(level);
      delay(75);
      level += FLAG_LVL3;
      ledFade(level);
      delay(75);
      level -= FLAG_LVL3;
      ledFade(level);
      delay(75);
      level -= FLAG_LVL2;
      ledFade(level);
      delay(75);
      level -= FLAG_LVL1;
      ledFade(level);
      delay(75);
      ledSwitchOff();
      Serial.println("MODE_OUTPUT_FADE");
      break;
    }

    while (digitalRead(PIN_BUTTON1) == HIGH) {
      // Waiting for button release...
      if((digitalRead(PIN_BUTTON2) == HIGH) && (isDoubleButton == false)) {
        isDoubleButton = true;
        checkIntLevel();
      }
    }
  }

  // Button2 (Change Colour)
  if (digitalRead(PIN_BUTTON2) == HIGH) {
    changeColour();
    // Previsualize colour
    level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    Serial.println(colourMode);

    while (digitalRead(PIN_BUTTON2) == HIGH) {
      // Waiting for button release...
      if((digitalRead(PIN_BUTTON1) == HIGH) && (isDoubleButton == false)) {
        isDoubleButton = true;
        checkIntLevel();
      }
    }
  }
}

void changeColour() {
  // Check colour state and set next state
  // [RGB-R-RG-G-GB-B-BR]

  Serial.print("Change Colour: ");

  if (colourMode == (FLAG_R | FLAG_G | FLAG_B)) {
    colourMode = FLAG_R;
    Serial.println("R");
  } else if (colourMode == FLAG_R){
    colourMode = (FLAG_R | FLAG_G);
    Serial.println("RG");
  } else if (colourMode == (FLAG_R | FLAG_G)) {
    colourMode = FLAG_G;
    Serial.println("G");
  } else if (colourMode == FLAG_G) {
    colourMode = (FLAG_G | FLAG_B);
    Serial.println("GB");
  } else if (colourMode == (FLAG_G | FLAG_B)) {
    colourMode = FLAG_B;
    Serial.println("B");
  } else if (colourMode == FLAG_B) {
    colourMode = (FLAG_R | FLAG_B);
    Serial.println("RB");
  } else if (colourMode == (FLAG_R | FLAG_B)) {
    colourMode = (FLAG_R | FLAG_G | FLAG_B);
    Serial.println("RGB");
  } else {
    colourMode = (FLAG_R | FLAG_G | FLAG_B);
    Serial.println("RGB");
  }
}

void checkIntLevel() {
  // Increment intensity light level
  intLVL++;
  // Max 4 levels
  if (intLVL == 4) {
    intLVL = 1;
  }
  byte level = FLAG_LVL1 + FLAG_LVL2 + FLAG_LVL3;
  switch(intLVL) {
    case 1:
    intLVL1 = INT1_LVL_1;
    intLVL2 = INT1_LVL_2;
    intLVL3 = INT1_LVL_3;
    intLVL4 = INT1_LVL_4;
    intLVL5 = INT1_LVL_5;
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    delay(75);
    break;

    case 2:
    intLVL1 = INT2_LVL_1;
    intLVL2 = INT2_LVL_2;
    intLVL3 = INT2_LVL_3;
    intLVL4 = INT2_LVL_4;
    intLVL5 = INT2_LVL_5;
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    delay(75);
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    delay(75);
    break;

    case 3:
    intLVL1 = INT3_LVL_1;
    intLVL2 = INT3_LVL_2;
    intLVL3 = INT3_LVL_3;
    intLVL4 = INT3_LVL_4;
    intLVL5 = INT3_LVL_5;
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    delay(75);
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    delay(75);
    ledLevel(level);
    delay(75);
    ledSwitchOff();
    break;
  }
  Serial.println(intLVL);
}


void read_mpu_6050_data(){                                             //Subroutine for reading the raw gyro and accelerometer data
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x3B);                                                    //Send the requested starting register
  Wire.endTransmission();                                              //End the transmission
  Wire.requestFrom(0x68,14);                                           //Request 14 bytes from the MPU-6050
  while(Wire.available() < 14);                                        //Wait until all the bytes are received
  acc_x = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_x variable
  acc_y = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_y variable
  acc_z = Wire.read()<<8|Wire.read();                                  //Add the low and high byte to the acc_z variable
  temperature = Wire.read()<<8|Wire.read();                            //Add the low and high byte to the temperature variable
  gyro_x = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_x variable
  gyro_y = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_y variable
  gyro_z = Wire.read()<<8|Wire.read();                                 //Add the low and high byte to the gyro_z variable
}

void setup_mpu_6050_registers(){
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6B);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1C);                                                    //Send the requested starting register
  Wire.write(0x10);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1B);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
}

void accelCalibration() {
  // Perform a lecture of 500 cycles. Average at the end of the loop.
  Serial.print("Calibrating accel");
  for (int cal_int = 0; cal_int < 500 ; cal_int ++){   //Run this code 2000 times
    if(cal_int % 125 == 0)Serial.print(".");           //Print a dot on the LCD every 125 readings
    read_mpu_6050_data();                              //Read the raw acc and gyro data from the MPU-6050
    acc_x_cal += acc_x;
    delay(3);                                          //Delay 3us to simulate the 250Hz program loop
  }
  acc_x_cal /= 500;
  Serial.println("");
}
