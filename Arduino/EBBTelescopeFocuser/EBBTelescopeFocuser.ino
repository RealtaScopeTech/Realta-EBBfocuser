#include <EEPROM.h>;
#include <SoftwareSerial.h>
#include "MotorClass.h";
#include "SerialComms.h";

#define D_STEP_PIN PD0
#define D_UART_PIN PA15
#define D_ENABLE_PIN PD2
#define D_HEATER_PIN PB13

#define D_DRIVER_ADDRESS 0b00 // PCB has both TMC2209 MS1 and MS2 pins grounded so this is the correct address for that configuration
#define D_R_SENSE 0.11f // These are the resistance values of the current monitoring resistors as per the PCB datasheet, it has two 0R11 resistors (0.11 Ohm). This is used by the TMC2209 stepper driver to measure the current going to the stepper motor. It has one resistor for each coil of the stepper. 
 
SoftwareSerial G_TMC_SERIAL (D_UART_PIN, D_UART_PIN);
Motor MyMotor(&G_TMC_SERIAL, D_R_SENSE, D_DRIVER_ADDRESS);

void setup() {
  // put your setup code here, to run once:
  byte isFirstRun;
  EEPROM.get(0, isFirstRun);

  if (isFirstRun != 6)
  {
    EEPROM.put(1, 50000); // Current position
    EEPROM.put(10,500); // Motor Current
    EEPROM.put(20,8); // Micro Steps
    EEPROM.put(0,6); 
  }
  
  EEPROM.get(1, MyMotor.CurrentPosition);
  EEPROM.get(10, MyMotor.current);
  EEPROM.get(20, MyMotor.steps);
  
  MyMotor.IsMoving = false;
  pinMode(D_STEP_PIN, OUTPUT);
  pinMode(D_ENABLE_PIN, OUTPUT);

  MyMotor.engageMotor(true);
  MyMotor.setHeaterPWM(0);
  
  Serial.begin(9600); // Start serial communication to PC

  G_TMC_SERIAL.begin(11520);  // Start serial communication with TMC
  MyMotor.beginSerial(11520); 
  MyMotor.begin();                                                                                                                                                                                                                                                                                                                            // UART: Init SW UART (if selected) with default 115200 baudrate
  MyMotor.toff(5);                          // Enables driver in software
  MyMotor.rms_current(MyMotor.current);     // Set motor RMS current, needs to be user configurable, unit is mA
  MyMotor.microsteps(MyMotor.steps);        // Set the micro steps of the motor driver

  MyMotor.en_spreadCycle(false);            // sets stepper to use silent mode
  MyMotor.pwm_autoscale(true);              // Needed for stealthChop

}

void loop() {
   
   // Check the serial port for command
   P_PROCESS_SERIAL_PORT();

   // If we got a command then split into G code and parameters
   if (G_SERIAL_LINE_FEED_RECEIVED)
   {
     P_SPLIT_G_CODE_AND_PARAMS();
   }

    // Now process G code
   if (G_END_COMMAND_FOUND)
   {
      G_END_COMMAND_FOUND = false;
      // Code goes here
      if(G_COMMAND == "G1")
      {
        MyMotor.SetMoveTarget(G_PARAMS.toInt());
      }

      if(G_COMMAND == "G2")
      {
        MyMotor.Halt();
      }
      
      if(G_COMMAND == "G3")
      {
        Serial.print(String(MyMotor.IsMoving) + "#");
      }

      if(G_COMMAND == "G4")
      {
        Serial.print(String(MyMotor.CurrentPosition) + "#");
      }

      if(G_COMMAND == "G5")
      {
        MyMotor.CurrentPosition = G_PARAMS.toInt();
      }

      if(G_COMMAND == "G6")
      {
        MyMotor.current = G_PARAMS.toInt();
        EEPROM.put(10,MyMotor.current);
        MyMotor.rms_current(MyMotor.current);
      }

      if(G_COMMAND == "G7")
      {
        Serial.print(String(MyMotor.current) + "#");
      }

      if(G_COMMAND == "G8")
      {
        if(G_HAS_PARAMS)
        {
          MyMotor.steps = G_PARAMS.toInt();
          EEPROM.put(20, MyMotor.steps);
          MyMotor.microsteps(MyMotor.steps);
        }
      }

      if(G_COMMAND == "G9")
      {
        Serial.print(String(MyMotor.steps) + "#");
      }

      if(G_COMMAND == "G10")
      {
        MyMotor.setHeaterPWM(G_PARAMS.toInt());
      }

      if(G_COMMAND == "G11")
      {
        Serial.print(String(MyMotor.CurrentHeaterValue) + "#");     
      }

      if(G_COMMAND == "G12")
      {
        boolean trueFalse;
        if( G_PARAMS == "1")
        {
          trueFalse = true;
        }
        else
        {
          trueFalse = false;
        }
        
        MyMotor.engageMotor(trueFalse);
      }

      if(G_COMMAND == "G13")
      {
        if(MyMotor.IsEngaged)
        {
          Serial.print("1#");
        }
        else
        {
          Serial.print("0#");
        }
      }

      if(G_COMMAND == "G14")
      {
        EEPROM.put(1, 50000); // Current position
        EEPROM.put(10,500); // Motor Current
        EEPROM.put(20,8); // Micro Steps
        // read these back out into motor classes variables
        EEPROM.get(1, MyMotor.CurrentPosition);
        EEPROM.get(10, MyMotor.current);
        EEPROM.get(20, MyMotor.steps);
        // send some of these settings to the motor driver
        MyMotor.rms_current(MyMotor.current);     // Set motor RMS current, needs to be user configurable, unit is mA
        MyMotor.microsteps(MyMotor.steps);  // Set the micro steps of the motor driver
      }
   }
  
  // put your main code here, to run repeatedly:
  if(MyMotor.IsMoving == true)
  {
     if(MyMotor.Move() == true)
     {
      // Change direction of TMC2209 driver to match MyMotor.move_direction//
      // Raise and lower stepper motor pin // 
     }
  }
}
