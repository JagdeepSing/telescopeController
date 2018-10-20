/*
    Dome Paddle for MRO using Xbox 360 Controller. Written by Sierra Dodd, Jagdeep Singh and Eva Smerekanych.
    May 2018
    Pin Connections on Arduino:
        1) North (black) to pin 2
        2) South (blue) to pin 3
        3) West (green) to pin 4
        5) East (brown) to pin 5
    How to Use:
        1) Connect controller to the arduino using wireless reciever.
        2) Make sure arduino is on
        3) Test controller ouput by holding left trigger on controller and pressing buttons on d-pad,
           lights on controller should change.
        4) Start Bifrost
        5) Turn telescope key
        6) Initialize the telescope
        7) Good to go!
    Controls:
        Telescope:
            North: Up
            West:  Right
            East:  Left
            South: Down
        Dome:
            Left:  B
            Right: X
*/

#include <XBOXRECV.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

USB Usb;
XBOXRECV Xbox(&Usb);

class Axis {
  public:

  Axis(int PositivePin, int NegativePin) {
    // movement pins
    _posPin = PositivePin;
    _negPin = NegativePin;
    // timer to prevent telescope abuse
    _lastCommandTime = 0;
    _state = 0;

    pinMode(_posPin, OUTPUT);
    pinMode(_negPin, OUTPUT);
  }

  boolean MovePositive() {
    if (_state != 'N') {
      digitalWrite(_posPin, LOW);
      _state = 'P';
      _lastCommandTime = millis();
      return true;//returns true if it moved positively
    }
    return false;//return false if it did not move positively
  }

  boolean MoveNegative() {
    if (_state != 'P') {
      digitalWrite(_negPin, LOW);
      _state = 'N';
      _lastCommandTime = millis();
      return true;//return true if it moved negatively
    }
    return false;//return false if it did not move negatively
  }

  // stops telescope movement
  void Stop() {
    if (millis() - _lastCommandTime > 200) {
      digitalWrite(_posPin, HIGH);
      digitalWrite(_negPin, HIGH);
      _state = 0;
    }
  }

  private:
    char _state; // current movement state
    double _posPin; // positive direction
    double _negPin; // negative direction
    unsigned long _lastCommandTime; // movement timer
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////Dome
class DomeAxis {
  public:

  // Dome Axis class
  // for each movement axis direction
  DomeAxis(char LeftPin, char RightPin) {
    _LPin = LeftPin;
    _RPin = RightPin;
    _lastCommandTime = 0;
    _state = 0;

    pinMode(_LPin, OUTPUT);
    pinMode(_RPin, OUTPUT);
  }

  // rotates dome left
  boolean MoveLeft() {
    if (_state != 'R') {
      digitalWrite(_LPin, HIGH);
      _state = 'L';
      _lastCommandTime = millis();
      return true;//returns true if it moved Left
    }
    return false;//return false if it did not move Left
  }

  // rotates dome left
  boolean MoveRight() {
    if (_state != 'L') {
      digitalWrite(_RPin, HIGH);
      _state = 'R';
      _lastCommandTime = millis();
      return true;//return true if it moved Right
    }
    return false;//return false if it did not move Right
  }

  // stops dome rotation
  void Stop() {
    if (millis() - _lastCommandTime > 200) {
      digitalWrite(_LPin, LOW);
      digitalWrite(_RPin, LOW);
      _state = 0;
    }
  }
  private:
    char _state; // state of dome
    char _LPin;  // left movement pin
    char _RPin;  // right movement pin
    unsigned long _lastCommandTime;  // timer to prevent dome abuse
};
 ////////////////////////////////////////////////////////////////////////////////Dome End

Axis N_S_Axis(2,3);
Axis E_W_Axis(4,5);
DomeAxis _DomeAxis(A2, A0);
int SetSlewPin = 6;

void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\nOSC did not start"));
    while (1); //halt
  }
  Serial.print(F("\r\nXBOX USB Library Started"));

  pinMode(SetSlewPin, OUTPUT);
}

void loop() {
  Usb.Task();
  if (Xbox.XboxReceiverConnected) {
    for (uint8_t i = 0; i < 4; i++){
      if (Xbox.Xbox360Connected[i]) {

        // Speed high and low for telescope movement
        if (Xbox.getButtonClick(START, i)) {
          Xbox.setLedMode(ROTATING, i);
          digitalWrite(SetSlewPin, LOW);
        }

        if (Xbox.getButtonClick(BACK, i)) {
          Xbox.setLedBlink(ALL, i);
          digitalWrite(SetSlewPin, HIGH);
        }

        if (Xbox.getButtonPress(L2, i)) {
          if (Xbox.getButtonPress(UP, i) && Xbox.getButtonPress(RIGHT, i)) {
            // Northwest
            N_S_Axis.MovePositive();
            E_W_Axis.MovePositive();
          } else if (Xbox.getButtonPress(DOWN, i) && Xbox.getButtonPress(RIGHT, i)) {
            // Southwest
            N_S_Axis.MoveNegative();
            E_W_Axis.MovePositive();
          } else if (Xbox.getButtonPress(UP, i) && Xbox.getButtonPress(LEFT, i)) {
            // Northeast
            N_S_Axis.MovePositive();
            E_W_Axis.MoveNegative();
          } else if (Xbox.getButtonPress(DOWN, i) && Xbox.getButtonPress(LEFT, i)) {
            N_S_Axis.MoveNegative();
            E_W_Axis.MoveNegative();
          } else if (Xbox.getButtonPress(UP, i)) {
            // North
            E_W_Axis.Stop();
            N_S_Axis.MovePositive();
            break;
          } else if (Xbox.getButtonPress(DOWN, i)) {
            // South
            E_W_Axis.Stop();
            N_S_Axis.MoveNegative();
            Serial.print("S");
            break;
          } else if (Xbox.getButtonPress(RIGHT, i)) {
            // West
            N_S_Axis.Stop();
            E_W_Axis.MovePositive();
            Serial.print("W");
            break;
          } else if (Xbox.getButtonPress(LEFT, i)) {
            // East
            N_S_Axis.Stop();
            E_W_Axis.MoveNegative();
            Serial.print("E");
            break;
          } else {
            // stop all movement when no button pressed
            N_S_Axis.Stop();
            E_W_Axis.Stop();
          }
        } else if (Xbox.getButtonPress(R2, i)) {  // dome rotation
          if (Xbox.getButtonPress(X, i)){
            //Dome Left
            _DomeAxis.MoveLeft();
          }else if (Xbox.getButtonPress(B, i)){
            //Dome Right
            _DomeAxis.MoveRight();
          }else{
            // stop dome rotation
             _DomeAxis.Stop();
          }

        } else {
          // stop all telescope and dome movement
          N_S_Axis.Stop();
          E_W_Axis.Stop();
          _DomeAxis.Stop();
        }
      }
    }
  delay(1);
  }
}
