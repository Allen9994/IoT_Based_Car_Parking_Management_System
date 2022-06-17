
//--------Start of the Arduino Program code---------
//Header Files
#include <Servo.h> // for interfacing motor
#include <Wire.h> //for the servo motor
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h> //For the I2C module that controls the LCD display.
//Library Files
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); //initialising the pins for lcd.
SoftwareSerial espSerial(26, 28);//Initiliasing pins for receiving and transmitting. The indentifier used to transmit to Arduino. 
Servo myservo; //initialising identifier for Servo motor.

//Initialising Global variables
#define ir_enter 11 //Entry IR Sensor. For detecting presence of car at the entrance.
#define ir_back  12 //Exit IR Sensor. For detecting presence of car at the exit.

#define ir_car1 5 //IR sensor variable for each car slot.
#define ir_car2 6
#define ir_car3 7
#define ir_car4 8
#define ir_car5 9
#define ir_car6 10

int S1=0, S2=0, S3=0, S4=0, S5=0, S6=0; //The variables hold free state(0) or occupied for each car slot(1).
int c; //Holds the value rerieved from NODEMCU : 1/2/3. Used to control the gate and for appropriate comment for the LCD display.
bool flag1=0, flag2=0; // Flags for holding the IR sensor value to keep the gate open.
int slot = 6; //The number of free slots remaining.
int le = 1;
String msg1,msg2; //msg1 for previous Blynk value string. msg2 for current blynk value string.


void setup(){ //Setting up the components, variables and basic functions. 
  Serial.begin(115200); //Baud rate of Arduino.
  espSerial.begin(115200);  //Baud rate for Serial communiation with NODE.
  pinMode(ir_car1, INPUT);  //Setting the pin mode for each IR pins.
  pinMode(ir_car2, INPUT);
  pinMode(ir_car3, INPUT);
  pinMode(ir_car4, INPUT);
  pinMode(ir_car5, INPUT);
  pinMode(ir_car6, INPUT);
  
  pinMode(ir_enter, INPUT);
  pinMode(ir_back, INPUT);
  pinMode(22,INPUT); //Pins 3 and 4 to recieve the signal from NodeMCU.
  pinMode(24,INPUT);
    
  myservo.attach(4);  //Attach servo variable to pin 4.
  myservo.write(100);
  delay(100);
  myservo.write(90); //Keep the gate in closed position (90 degrees)..
  
  lcd.begin(20, 4);  //Displaying the intro message onto the LCD.
  lcd.setCursor (0,1);
  lcd.print("    Car  parking  ");  lcd.setCursor (0,2);  lcd.print("       System     ");
  delay(1000);
  lcd.clear();   
  
  Read_Sensor(); //Reads value from IR sensors for the first time. Initialises the variable slot.
  msg1 = String (S1 * 100000 + S2 * 10000 + S3 * 1000 + S4 * 100 + S5 * 10 + S6 );
  espSerial.println(msg1);  //Converts the slot variables to string and trasmit it to NODE. 
  int total = S1+S2+S3+S4+S5+S6; //To find the number of free slots at the begining.
  slot = 6 - total; //Initialises the free slots variable once the Arduino is starting up.
}

bool g,h; 
void check() // To convert the 2-bit data from NodeMCU to a decimal value. 
{
  delay(100);
  g = digitalRead(22);
  h = digitalRead(24);
  if(g == LOW && h == LOW) //No response.
  {
    if(digitalRead(ir_enter)==0 || digitalRead(ir_back)==0) //To recurse only if the car is still waiting in the queue.
    {
      check(); //Recursion (To keep the arduino waiting for the response from NodeMCU).
    }
    else
    {
      flag1 = LOW;flag2 = LOW;
    }
  }else
  if(g == LOW && h == HIGH)
  {
    c = 1; //Insufficient balance from the card.
  }else if(g == HIGH && h == LOW)
  {
    c = 2; //Card not recognized.
  }else if(g == HIGH && h == HIGH)
  {
    c = 3; //Allow the entry of the user.
  }
}

void Near() //To find the nearest parking slot.
{ //Variable to store the least number. 
  //Compares with empty slot order in ascending order and stores the value accordingly.
  if(S1 == 0){le = 1;}else if(S2 == 0){le = 2;}else if(S3 == 0){le = 3;}
  else if(S4 == 0){le = 4;}else if(S5 == 0){le = 5;}else if(S6 == 0){le = 5;}
   lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Kindly Park your car");
      lcd.setCursor(0,2); 
      lcd.print("at Slot ");lcd.print(le);    //Asks the driver to park their car in nearest slot.
      delay(2000);  
      lcd.clear();
}

String str;
void loop(){

  Read_Sensor(); //Read the values from the IR sensors in real time. Keeps the slot information updated. 
  int space = S1+S2+S3+S4+S5+S6; // For determining congestion.
  Serial.println(slot); 
  msg2 = String (S1 * 100000 + S2 * 10000 + S3 * 1000 + S4 * 100 + S5 * 10 + S6 ); 
  //Transforms the slot variables to string by arranging in order.                                                                                  
  if(msg2 != msg1)         //Checks if the slot string produced is the same as that of the last cycle. 
  { //Eliminates sending the same string repetedily and msg is trasmitted only if a change is observed in the string.
      espSerial.println(msg2);    //Sends the serial data to NodeMCU to be processed and make the Blynk application work.
      msg1 = msg2;      //Old message string is stored with new message and thus compared in real time.
  }
  
//Displays slot information and free slots in the LCD display.
lcd.setCursor (0,0);
lcd.print("  Free Slots: "); 
lcd.print(slot);
lcd.print("    ");  

lcd.setCursor (0,1);
if(S1==1){lcd.print("S1:Taken ");}
     else{lcd.print("S1:Empty");}

lcd.setCursor (10,1);
if(S2==1){lcd.print("S2:Taken ");}
     else{lcd.print("S2:Empty");}

lcd.setCursor (0,2);
if(S3==1){lcd.print("S3:Taken ");}
     else{lcd.print("S3:Empty");}

lcd.setCursor (10,2);
if(S4==1){lcd.print("S4:Taken ");}
     else{lcd.print("S4:Empty");}

 lcd.setCursor (0,3);
if(S5==1){lcd.print("S5:Taken ");}
     else{lcd.print("S5:Empty");}

lcd.setCursor (10,3);
if(S6==1){lcd.print("S6:Taken ");}
     else{lcd.print("S6:Empty");}    

if(digitalRead (ir_enter) == 0 && flag1==LOW) //When the entry sensor detects car and.
{   // the car is attempting to enter.
  if(slot>0 && space+slot>4)  //To allow the car inside only if there is 
  { // vacanat slot and no congestion.
      flag1=HIGH; //to prevent this 'if statement' to reitereate. Stores car presence in flag1.
 
    if(flag2==LOW)  //When no presence of car was detected in exit sensor (Car is entering).
    { 
      c = 0;  //Data from NodeMCU.
      Serial.println("Show your card");
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Kindly drive closer");
      lcd.setCursor(0,2);
      lcd.print("  to the Gate!");
      check();  //Calls the checking function to accept value from NodeMCU
      lcd.clear();
      Serial.println(c);
    if(c == 3)  //To allow the car to enter the parking lot.
    {
        delay(100);
        Serial.println("ACCESS");
        myservo.write(180);  //Opens the gate my signaling the motor to rotate 180D.
        slot = slot-1;  c = 0; // Decrements the slot count by 1.
        Near(); //Calls Near function to find and display nearest slot
     }
    else
    if(c == 1)  //Insufficient balance detected.
    {flag1 = LOW; //Removes the car data from the flag to continue sensing for new cars.
      delay(100);
      lcd.clear();
      c = 0;
      lcd.setCursor(0,1);
      lcd.print("Insufficient balance!");
      delay(1000);
      lcd.clear();
      delay(1000);
    }else
    if(c == 2)  //Invalid car detected.
    {flag1 = LOW; //Removes the car data from the flag to continue sensing for new cars.
      delay(100);
      c = 0;
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Invalid Card!");
      delay(1000);
      lcd.clear();
      Serial.print("Invalid Card!");
      delay(100);
    }
    }
    else    //The car is leaving. This is implied since flag2 is NOT LOW.
      {   // ie: the car has touched the exit sensor FIRST.
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Goodbye!");
        lcd.setCursor(0,2);
        lcd.print("See you soon.");
        delay(1000);
        lcd.clear();
        Serial.println("Vehicle exits");
      }
  }else if (slot==0)  //If all the slots are full.
  {
  lcd.setCursor (0,0);
  lcd.print(" Sorry Parking Full ");  
  delay(1500);
  }   
  else if(space+slot <5)
  {
    lcd.clear();
    lcd.setCursor (1,1);
    lcd.print("Parking lot crowded");  
    lcd.setCursor (4,2);
    lcd.print("Please Wait!");  
    delay(1500);
    lcd.clear();
  }
}

if(digitalRead (ir_back) == 0 && flag2==LOW)   //When the exit sensor detects car and car is attempting to leave.
{
  flag2=HIGH; //to prevent this 'if statement' to reitereate. Stores car presence in flag2.
  if(flag1==LOW)  //When no presence of car was detected in entry sensor (Car is leaving).
  {
      c = 0;
      Serial.println("Show your card");
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print(" Kindly place your");
      lcd.setCursor(0,2);
      lcd.print(" Card on the sensor!");
      check();
      lcd.clear();
      Serial.println(c);
      if(c == 3 || c == 1)  //To allow the car to leave the parking lot. Ignore the balance of the driver.
      {
        myservo.write(180); Serial.println("Gate Opens ");slot = slot+1;  //Opens the gate my signaling the motor to rotate 180D. Increments the slot count by 1.
        c = 0;
      }
      else if(c==2)  //Invalid car detected.
      {
        flag2 = LOW;  //Removes the car data from the flag to continue sensing for new cars.
        c = 0;
        lcd.clear();
        lcd.setCursor(0,1);
        lcd.print("Invalid Card!");
        delay(1000);
        lcd.clear();
        Serial.print("Invalid Card!");
        delay(100);
      }
  }else //The car is entering. This is implied since flag1 is NOT LOW.
    {   // ie: the car has touched the entry sensor FIRST.
      lcd.clear();
      lcd.setCursor(0,1);
      lcd.print("Welcome!");
      delay(5000);
      lcd.clear();
      delay(100);
    }
}

if(flag1==HIGH && flag2==HIGH)  //To close the gate. 
{ //If the car has passed both the sensors ie. flag1 = high and flag2 = high, close the gate.
  if(digitalRead(ir_enter) == 1 && digitalRead(ir_back) == 1)
  {
  delay (1000);
  myservo.write(90);  //Closes the gate by signaling the motor to rotate 90D..
  Serial.println("Gate Closes ");
  flag1=LOW, flag2=LOW; //Reinitialises the flag information for new car.
  }
}

delay(1);
}

void Read_Sensor(){ //Reads value from the IR sensors and updates the slot variables.
S1=0, S2=0, S3=0, S4=0, S5=0, S6=0;

if(digitalRead(ir_car1) == 0){S1=1;}
if(digitalRead(ir_car2) == 0){S2=1;}
if(digitalRead(ir_car3) == 0){S3=1;}
if(digitalRead(ir_car4) == 0){S4=1;}
if(digitalRead(ir_car5) == 0){S5=1;}
if(digitalRead(ir_car6) == 0){S6=1;}  
}
