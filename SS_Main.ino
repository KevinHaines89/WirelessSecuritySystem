#include <LowPower.h>
#include <SPI.h>
#include <WiFi101.h>
#include <WiFiClient.h>
#include <Temboo.h>
#define TEMBOO_ACCOUNT "kevinhawkm2m"  // Your Temboo account name 
#define TEMBOO_APP_KEY_NAME "myFirstApp"  // Your Temboo app key name
#define TEMBOO_APP_KEY "xiWq9fxnMW9bYJb3BZWjj4uOmmQJoVhZ"  // Your Temboo app key

#define WIFI_SSID "yourSSID" //the ssid of your network
#define WPA_PASSWORD "yourPassword"//the ssid password

#define doorIntPin 5
#define sirenSelect digitalRead(7)
#define SIREN_OFF  digitalWrite(6, HIGH)
#define SIREN_ON  digitalWrite(6, LOW)

WiFiClient client;

//various boolean variables 
static uint8_t sysEnabled = 0;
static uint8_t alarmOn = 0;
static uint8_t debounceTimerOn =0;
static uint8_t systemEnabling = 0;
//timer reference variables
static uint32_t debounceTimerPlace =0;
static uint32_t alarmPlace = 0;
static uint32_t systemEnablingPlace = 0;
//variables used to enable chirping
static uint16_t chirpCount1 = 1;
static uint16_t chirpCount2 = 1;
/*
 * The first function executed.
 * Performs miscellaneous initialization tasks
 */
void setup() {
  Serial.begin(9600);//Set baud rate and initialize Serial port
  pinMode(7, INPUT);//Initialize pin for siren mode
  pinMode(6, OUTPUT);//Initialize pin for siren
  SIREN_OFF;//Turn siren off, initially
  attachInterrupt(doorIntPin, doorOpens, LOW);//Initialize External interrupt on Low-level 
  interrupts();//Set global interrupt bit
  systemEnablingPlace = millis();//Record time from millis function for future reference
  systemEnabling=1;//turn on enabling timer
}
/*
 * This function loops forever starting at the 
 * end of the setup function.  If any of the three
 * timers is on the timers associated functions executes.
 * If no timer is on, the microcontroller is put to sleep
 */
void loop() {
  if(alarmOn)
    runAlarm();
  if(systemEnabling)
    systemEnablingFunc();
  if(debounceTimerOn)
    debounceTimer();
  if(!alarmOn && !systemEnabling && !debounceTimerOn)
    LowPower.standby();
}
/*
 * Function that connects to the wifi module
 */
void connectToWifi(){
  // For debugging, wait until the serial console is connected
  int wifiStatus = WL_IDLE_STATUS;
  // Determine if the WiFi Shield is present
  Serial.print("\n\nShield:");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("FAIL");
    // If there's no WiFi shield, stop here
    while (true);
  }
  Serial.println("OK");

  // Try to connect to the local WiFi network
  while (wifiStatus != WL_CONNECTED) {
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(WIFI_SSID, WPA_PASSWORD);

    if (wifiStatus == WL_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
  }
  Serial.println("Setup complete.\n");
}
/*
 * This function handles alarm activities
 */
void runAlarm() {
  unsigned long time = millis() - alarmPlace;
    if (time < 300000) {
       if(sirenSelect==LOW){
          if (time > (chirpCount1)*325){
              if(chirpCount1%2)
                  SIREN_ON;
              else
                  SIREN_OFF;
              chirpCount1++;
          }
       }
       else
        SIREN_ON;
    }
    else{
      chirpCount1 = 1;
      systemEnabling = 0;
      alarmOn = 0;
      SIREN_OFF;
   }
}
/*
 * This function handles operations during the time that
 * the system is being enabled.
 */
void systemEnablingFunc() {
  unsigned long time = millis() - systemEnablingPlace;
  if (time >= 9750) {
    systemEnabling = 0;
    sysEnabled = 1;
    chirpCount2 = 1;
    SIREN_OFF;
  }
  else {
    if (time >= ((chirpCount2 - 1) * 1325) && time <= ((chirpCount2 * 1000) + (chirpCount2 - 1) * 325)) {
      SIREN_OFF;
    }
    else if (time >= ((chirpCount2 * 1000) + (chirpCount2 - 1) * 325) && (chirpCount2 * 1325)) {
      SIREN_ON;
      chirpCount2++;
    }
  }
}
/*
 * Timer which sends email and turns on alarm timer after 10ms
 */
void debounceTimer(){
  unsigned long time = millis()- debounceTimerPlace;
  if(sysEnabled && time> 10){
    debounceTimerOn=0;
    delay(3000);
    connectToWifi();
    sendEmail("Intruder Alert!", "Unauthorized entry detected!");
    WiFi.disconnect();
    alarmPlace = millis();
    alarmOn = 1;
  }
}
/*
 * The function that is executed on a low-level interrupt on
 * the designated door pin
 */
void doorOpens() {
  if(!debounceTimerOn){
      debounceTimerPlace = millis();
      debounceTimerOn=1;
  }
}
/*
 * This functions sends an email with the given body and subject.
 * This function was taken from an example in the
 * Temboo library.
 */
void sendEmail(String subject, String body) {
  Serial.println("Running SendEmail");
  TembooChoreo SendEmailChoreo(client);
  // Invoke the Temboo client
  SendEmailChoreo.begin();
  // Set Temboo account credentials
  SendEmailChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendEmailChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendEmailChoreo.setAppKey(TEMBOO_APP_KEY);
  // Set Choreo inputs
  String UsernameValue = "sssystemm2m@gmail.com";
  SendEmailChoreo.addInput("Username", UsernameValue);
  SendEmailChoreo.addInput("Subject", subject);
  String ToAddressValue = "kevinhaines@u.boisestate.edu";
  SendEmailChoreo.addInput("ToAddress", ToAddressValue);
  String PasswordValue = "twfijevpbpuaazkv";
  SendEmailChoreo.addInput("Password", PasswordValue);
  SendEmailChoreo.addInput("MessageBody", body);
  // Identify the Choreo to run
  SendEmailChoreo.setChoreo("/Library/Google/Gmail/SendEmail");
  // Run the Choreo; when results are available, print them to serial
  SendEmailChoreo.run();
  while (SendEmailChoreo.available()) {
    char c = SendEmailChoreo.read();
    Serial.print(c);
  }
  SendEmailChoreo.close();
}


