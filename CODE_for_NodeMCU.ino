
//--------Start of the NodeMCU Program code---------
//Header Files--------------------
#define BLYNK_TEMPLATE_ID "TMPLZXC-QBoy"
#define BLYNK_DEVICE_NAME "DEVICE NAME OF THE BLYNK DEVICE"
#define BLYNK_AUTH_TOKEN "BLYNK AUTH TOKEN"
#define BLYNK_PRINT Serial

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>

//IoT--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <BlynkSimpleEsp8266.h>

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>   

char auth[] = BLYNK_AUTH_TOKEN;

//************************************************************************

#define SS_PIN  D2  
#define RST_PIN D1  
//************************************************************************
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************
/* Set these to your desired credentials. */
const char *ssid =         "Wi-Fi SSID";
const char *password =     "Wi-Fi PASSWORD";
const char* device_token = "TOKEN ID OF THE WEB-APP";

BlynkTimer timer;
//************************************************************************

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
//************************************************************************
String URL = "http://IP-ADDRESS/rfidattendance/getdata.php"; //computer IP or the server domain.
String getData, Link, OldCardID = "";
unsigned long previousMillis = 0;
int n, p1, p2, p3, p4, p5, p6, nearest_slot = 1;
String slot_information; //To extract serial message from Arduino
//************************************************************************

void setup() {

  pinMode(D3,OUTPUT);   pinMode(D4,OUTPUT);
  digitalWrite(D3,LOW);  digitalWrite(D4,LOW);
  delay(1000);
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //---------------------------------------------
  connectToWiFi();
}
//************************************************************************


void loop() {
  Blynk.run(); //Starts Blynk 
  //check if there's a connection to Wi-Fi or not
  if(!WiFi.isConnected()){
  }
  if (Serial.available()) { //If Node observes presence of message.
        Blynk_function();
  }
  //---------------------------------------------
  if (millis() - previousMillis >= 15000) {
    previousMillis = millis();
    OldCardID="";
  }
  delay(50);
  //---------------------------------------------
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
  String CardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if( CardID == OldCardID ){
    return;
  }
  else{
    OldCardID = CardID;
  }
  //---------------------------------------------
  Serial.println(CardID);
  SendCardID(CardID);
  delay(1000);
}

void nearestParkingSlot()
{
         if(p1 == 0) nearest_slot = 1;
    else if(p2 == 0) nearest_slot = 2;
    else if(p3 == 0) nearest_slot = 3;
    else if(p4 == 0) nearest_slot = 4;
    else if(p5 == 0) nearest_slot = 5;
    else if(p6 == 0) nearest_slot = 6;
}

void sendMessage(String message, String token, String chat)
{
  delay(100); //Starts Telegram bot using the Token of each user's telegram.
  Serial.println(message);
  UniversalTelegramBot bot(token, secured_client);  
  //Sends the message obtained from the calling function 'y'. It uses chat ID to identify the user.
  bot.sendMessage(chat, message, ""); 
}

void telegram(String CI, String username, int amount, int time_, String admission, String token, String chat)
{
    if(admission == "login")  //If the user is entering the lot.
    {
      String directions;
           if(nearest_slot == 1 || nearest_slot == 2)    directions = "turn left and park your car";
      else if(nearest_slot == 3 || nearest_slot == 4)    directions = "head straight, turn left and park your car";
      else if(nearest_slot == 5 || nearest_slot == 6)    directions = "turn right and park your car along the wall";
      
      String message = "Welcome " + username + ". Your account balance is : " + String(amount) + ". Your nearest slot is " 
      + String(nearest_slot) + ". Kindly " + String(directions); //Produces the welcome message to be sent to their telegram.
      sendMessage(message, token, chat); //Calls the sending function.
    }
    if(admission == "logout") //If the user is entering the lot.
    {//Produces the exit message to be sent to their telegram.
      String message;
      if(amount >= 0)
      {
        message = "Dear " + username + ". You have spent " + String(time_) 
        + " seconds inside the parking lot. Your remaining balance is : " + String(amount); 
      }else
      {
        message = "Dear " + username + ". You have spent " + String(time_) 
        + " seconds inside the parking lot. You have a pending debt of " + String(-amount) 
        + ". Kindly clear your dues before your next arrival."; 
      } 
      sendMessage(message, token, chat);//Calls the sending function.
    }  
}

//************send the Card UID to the website*************
void SendCardID( String Card_uid ){
  Serial.println("Sending the Card ID");
  if(WiFi.isConnected()){
    HTTPClient http;    
//Declare object of class HTTPClient
    //GET Data
    getData = "?card_uid=" + String(Card_uid) + "&device_token=" + String(device_token); // Add the Card ID to the GET array in order to send it
    //GET methode
    Link = URL + getData;
    http.begin( Link); //initiate HTTP request   //Specify content-type header
    
    int httpCode = http.GET();   //Send the request
    String payload = http.getString();    //Get the response payload

    Serial.println(Link);   //Print HTTP return code
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(Card_uid);     //Print Card ID
    Serial.println(payload);    //Print request response payload

    if(payload == "Not found!") //If it is invalid card, Sends the data 01 to Arduino.
    {
       digitalWrite(D3,HIGH);
       delay(1000);
       digitalWrite(D3,LOW);
    }else
    if(payload == "Low Balance")  //If there is no balance in User's card, Sends the data 10 to Arduino
    {
       digitalWrite(D4,HIGH);
       delay(1000);
       digitalWrite(D4,LOW);
    }else
    if (httpCode == 200) {  //httpCode = 200 => connection is successful and NodeMCU can receive desired data from the MyPHPAdmin.
                            //httpCode =  -1 => error in connection.
     if (payload == "succesful") {  //During edit stage, if card is new and read succesfully.
        delay(100);
      http.end(); 
      }
      else if (payload == "available" || payload == "Invalid Device!") {  //During edit stage, if card was already saved and swiped again.
      delay(100);
      http.end(); 
      }else   // NOT in edit stage ie. Under normal operation. Extracting useful information from the SQL response.
      {
        int y = 0;  //Initialising y = 0. y is used to measure size of each line.
        while(payload[y] != '{'){  //When payload reaches '{' symbol, measures the number of decimal places in amount.
          y++;
        }int z = y;
        while(payload[z] != '/'){  //When payload reaches '/' symbol, measures the number of decimal places in time remaining.
          z++;
        }int a = z;        
        while(payload[a] != '}'){ //When payload reaches '}' symbol, measures the number of decimal places in telegram token.
          a++;
        }int b = a;        
        while(payload[b] != 'l'){  //When payload reaches 'l' symbol, measures the number of decimal places in telegram chat ID.
          b++;
        }
        int amount = payload.substring(0,y).toInt(); //Extracts and converts the amount to integer variable j.
        int ti = payload.substring(y+1,z).toInt();  //Extracts and converts the time remaining to integer variable ti.
        String token = payload.substring(z+1,a);  //Extracts the token to String variable token.
        String chat = payload.substring(a+1,b);   //Extracts the chat ID to String variable chat.
        if (payload.substring(b, b+5) == "login") { //When there's enough balance and valid card, allows user to enter.
         digitalWrite(D3,HIGH); digitalWrite(D4,HIGH); //Sending signal 11 to Arduino         
         delay(1000);
         digitalWrite(D3,LOW);  digitalWrite(D4,LOW);
           
         String user_name = payload.substring(b+5); //Extracting the name of user from payload and storing in user_name.
         Serial.println(user_name);
         telegram(Card_uid, user_name, amount, ti, "login", token, chat);  //Calling telegram function  
          // and sending all the extracted information to telegram function
        }
        else if (payload.substring(b, b+6) == "logout") { //When the user is exiting.
          digitalWrite(D3,HIGH);  digitalWrite(D4,HIGH); //Sending signal 11 to Arduino        
          delay(1000);
          digitalWrite(D3,LOW);   digitalWrite(D4,LOW);         
          String user_name = payload.substring(b+6);  //Extracting the name of user from payload and storing in user_name.
          Serial.println(user_name);
          telegram(Card_uid, user_name, amount, ti, "logout", token, chat);  //Calling telegram function 
      }  //and sending all the extracted information to telegram function.    
    }
      delay(100);
      http.end();  //Close connection
    }
  }
}

void Blynk_function()
{
        slot_information = Serial.readStringUntil('\r'); //extracts the entire string
        if(slot_information != "\n") //To read till entire line
        {                     //Here, the data from arduino is converted from string to integer. 
                              //Then it each of its digit is stored in individual integer variables 
            n = slot_information.toInt();
            p1 = n/100000;    //For eg. msg = 110010. p1 = 1 p2 = 1 p3 = 0 p4 = 0 p5 = 1 p6 = 0 
            n = n - p1*100000;
            p2 = n/10000;
            n = n - p2*10000;
            p3 = n/1000;
            n = n - p3*1000;
            p4 = n/100;
            n = n - p4*100;
            p5 = n/10;
            n = n - p5*10;
            p6 = n;
            nearestParkingSlot();
            Blynk.virtualWrite(V7, 6 - p1 - p2 - p3 - p4 - p5 - p6); 
            //Sends remaining slot information to blynk's 7th virtual pin.
            //Here it turns on/off the Blynk LED depending on the value of p which was extracted before.
            if(p1 == 1)  Blynk.virtualWrite(V1, HIGH);
            else         Blynk.virtualWrite(V1, LOW);
            if(p2 == 1)  Blynk.virtualWrite(V2, HIGH);
            else         Blynk.virtualWrite(V2, LOW);
            if(p3 == 1)  Blynk.virtualWrite(V3, HIGH);
            else         Blynk.virtualWrite(V3, LOW);
            if(p4 == 1)  Blynk.virtualWrite(V4, HIGH);
            else         Blynk.virtualWrite(V4, LOW);
            if(p5 == 1)  Blynk.virtualWrite(V5, HIGH);
            else         Blynk.virtualWrite(V5, LOW);
            if(p6 == 1)  Blynk.virtualWrite(V6, HIGH);
            else         Blynk.virtualWrite(V6, LOW);
       }
}
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Connected");
    secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    time_t now = time(nullptr);
    delay(1000);
}
