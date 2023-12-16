
#include <Keypad.h> 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

////define kepad  parameters
const byte ROWS = 4; //keypad rows number
const byte COLS = 3; //keypad columns number
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {3, 2, 1, 0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {6, 5, 4}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
////end of kepad definition

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

char CurrentFloor ; // inicate the current state of the elevator 1 means 1st Floor
char RequestedFloor; // the requested floor that the elevator need to go to. 0 means no request.

//sensors (limit switchs) at every door to insure that the specified floor 
//is accessed by the elevator.
byte Sensor1=7; // limit switch for presence of elevator at the 1st Floor
byte Sensor2=8; // limit switch for presence of elevator at the 2nd Floor 
byte Sensor3=9; // limit switch for presence of elevator at the 3rd Floor 
byte Sensor4=10; // limit switch for presence of elevator at the 4th Floor 

byte MovingUpLED = A1; // indicator led for moving down using anlaog pin in digatal mode
byte MovingDownLED = A0; // indicator led for moving up using anlaog pin in digatal mode

// declare the Motor pins
byte MotorPin1 = 12; 
byte MotorPin2 = 13;
byte MotorSpeed = 11; //use PWM pin (11) to set motor speed

//password for 4th floor authentication
char password[] = {'1', '2' , '3', '4'}; 

void setup() 
{
  // initialize the lcd
  lcd.init();   
  lcd.backlight();
  
  // initialize the motor pins as output  
  pinMode(MotorPin1, OUTPUT); 
  pinMode(MotorPin2, OUTPUT); 
  pinMode(MotorSpeed, OUTPUT); 

  // assign the Floor accessing sensors as input
  pinMode(Sensor1,INPUT);
  pinMode(Sensor2,INPUT);
  pinMode(Sensor3,INPUT);
  pinMode(Sensor4,INPUT);
  
  // assign the indicators' leds as output.
  pinMode(MovingUpLED, OUTPUT); 
  pinMode(MovingDownLED, OUTPUT); 
  
  //initialize motor statue  
  digitalWrite(MotorPin1, LOW);  
  digitalWrite(MotorPin2, LOW);
  digitalWrite(MotorSpeed, 0);

  //turn off motion led indicator for no motion  
  digitalWrite(MovingUpLED, LOW);  
  digitalWrite(MovingDownLED, LOW);
  
  CurrentFloor = '1'; 
  RequestedFloor=-1;

  lcd.print("Project sercuit "); 
     delay(800);
     lcd.clear();
     lcd.print(" Mahmoudi + Taki ");
     lcd.setCursor(2,1); 
     lcd.print("    SEM21  ");
     delay(800);
     lcd.clear();
}

void loop()
{
 
  CurrentFloor = checkCurrentFloor();
  // display a message on the LCD to indecate the current Floor of the elevator
  printCurrentFloor();
 
  //listen to floor requests.
  RequestedFloor = keypad.getKey();
  
  // check if a valid key presssed 
  if ((RequestedFloor =='1' || RequestedFloor =='2' || RequestedFloor =='3'|| RequestedFloor =='4' ))
      {
        manipulateRequest();  // excute the function to manipulate request.      
      }
}


//fuction used to move the elevator based on the requested floor.
void manipulateRequest()
{
      //authenticate the usere if requesting the 4th floor
      if(RequestedFloor == '4') 
         if (!checkPassword()) // reset the request and end if the password is wrong
              {
                RequestedFloor = -1; // reset requests  
                return ; // end function excution.
              }
              
      if( RequestedFloor > CurrentFloor ) // if requesting upper floor
        { 
          while(RequestedFloor > CurrentFloor) // don't break untill reaching the destination
          {
            up(); // move up as long as the currentfloor < requested floor          
            
            digitalWrite(MovingUpLED, HIGH); // light the moving up green LED indicator             
           
            //update the reached current floor by checking the sensors while moving.
            CurrentFloor = checkCurrentFloor();            
            // display the status of the reached current Floor 
            printCurrentFloor();
            
            ////// floor request interrupt while moving up
            char NewReqFloor = keypad.getKey(); // get the interrupting floor request          
            // if the interrupting floor request is valid 
            if ( NewReqFloor > CurrentFloor && NewReqFloor < RequestedFloor )              
             RequestedFloor = NewReqFloor; //if valid then change the destination. 
          }
        }   
      
      else if( RequestedFloor < CurrentFloor ) // if requesting down floor
       {
          while( RequestedFloor < CurrentFloor ) // don't break untill reaching the destination
          {
            down(); // move down as long as the CurrentFloor > Requested floor            
            
            digitalWrite(MovingDownLED, HIGH); // Light the moving down RED LED indicator            
            
            //update the reached current floor by checking the sensors while moving.
            CurrentFloor = checkCurrentFloor();           
            // display the status of the reached current Floor 
            printCurrentFloor();
            
            ////// floor request interrupt while moving up
            char NewReqFloor = keypad.getKey(); // get the interrupting floor request          
            //byte NewReqFloor = (byte)String(rf).toInt(); // convert char to byte 
            // if the interrupting floor request is valid 
            if ( NewReqFloor < CurrentFloor && NewReqFloor > RequestedFloor )              
             RequestedFloor = NewReqFloor; //if valid then change the destination. 

          }
        }  
      
      ///////////// End of the Floor request
      stop(); // stop moving
      digitalWrite(MovingUpLED, LOW);  
      digitalWrite(MovingDownLED, LOW); 
      RequestedFloor = -1; // reset requests  
}


//moving up the elvator (forward motor direction)
void up()
{
  digitalWrite(MotorPin1, HIGH);
  digitalWrite(MotorPin2, LOW); 
  analogWrite(MotorSpeed, 125); // run on half speed
}

//moving down the elvator (backward motor direction)
void down()
{
  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, HIGH); 
  analogWrite(MotorSpeed, 125); // run on half speed
}
void stop()
{
  digitalWrite(MotorPin1, LOW);
  digitalWrite(MotorPin2, LOW); 
  analogWrite(MotorSpeed, 0); 
}

//check for the reached currentFloor by checking the limit switchs
//in every floor and return the currnt value
char checkCurrentFloor()
{
    if( digitalRead(Sensor1)) 
        return '1';
    else if( digitalRead(Sensor2)) 
        return '2';
    else if( digitalRead(Sensor3)) 
        return '3';
    else if( digitalRead(Sensor4)) 
        return '4';
    else 
         return CurrentFloor; // latch last state
}


//perform authetication and return true if authenticated
bool checkPassword()
{
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Saisire mot pass");
  
  byte counter = 0; 
  char pass[4]; // temp password array to store the entered password
  
  while(counter < 4) // listen for the kepad
  { 
    char key = keypad.getKey();
    if(key)
    {      
      pass[counter] = key; // store the temp input password digit.
      
      lcd.setCursor(counter,1);
      lcd.print(key); // show the entered digit two the user on the LCD
      delay(300);
      
      lcd.setCursor(counter,1);
      lcd.print('*'); // hide the entered digit after a while with "*".
      
      counter++;      
    }   
  }
  
   for (int i=0; i<sizeof(pass); i++) 
   if (pass[i] != password[i]) // if one element voilate equality exit.
   {
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Wrong Password");
     lcd.setCursor(0,1);
     lcd.print("rerequest floor"); 
     delay(500);
     lcd.clear();
     
     return false; 
   }
   
   lcd.clear();
   lcd.setCursor(0,0);
   lcd.print("Authenticated");
   delay(500);
   lcd.clear();
   return true; //return true if all the element are equal.
}

//lcd display aiding function
void printCurrentFloor()
{
    lcd.setCursor(3,0);
    lcd.print("Vous etes ");
    
    lcd.setCursor(2,1);
  
    if(CurrentFloor == '1')
      lcd.print("au " + String(CurrentFloor) + "er etage");
    if(CurrentFloor == '2')
      lcd.print("au " + String(CurrentFloor) + "eme etage");
    if(CurrentFloor == '3')
      lcd.print("au " + String(CurrentFloor) + "eme etage");
    if(CurrentFloor > '3')
      lcd.print("au " + String(CurrentFloor) + "eme etage");
}


