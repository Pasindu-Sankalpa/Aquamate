/*
  this example will show
  1. how to use and ESP 32 for reading pins
  2. building a web page for a client (web browser, smartphone, smartTV) to connect to
  3. sending data from the ESP to the client to update JUST changed data
  4. sending data from the web page (like a slider or button press) to the ESP to tell the ESP to do something

  If you are not familiar with HTML, CSS page styling, and javascript, be patient, these code platforms are
  not intuitive and syntax is very inconsitent between platforms

  I know of 4 ways to update a web page
  1. send the whole page--very slow updates, causes ugly page redraws and is what you see in most examples
  2. send XML data to the web page that will update just the changed data--fast updates but older method
  3. JSON strings which are similar to XML but newer method
  4. web sockets very very fast updates, but not sure all the library support is available for ESP's

  I use XML here...

  compile options
  1. esp32 dev module
  2. upload speed 921600
  3. cpu speed 240 mhz
  flash speed 80 mhz
  flash mode qio
  flash size 4mb
  partition scheme default

  NOTE: if your ESP fails to program press the BOOT button during programm when the IDE is "looking for the ESP"
  NOTE: there are 6 changes to make to switch from ESP32 to ESP8266 or vise versa.
*/
#include <WiFiManager.h>       // library for Option 3
#include "WebData.h"           // .h file that stores your html page code

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[1] For ESP8266
// #include <ESP8266WiFi.h>       // standard library
// #include <ESP8266WebServer.h>  // standard library
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[1] For ESP32
#include <WiFi.h>       // standard library
// Note: For ESP32,
// ADC1 controls ADC function for pins GPIO 32-39
// ADC2 controls ADC function for pins GPIO 0, 2, 4, 12-15, 25-27
// ADC2 pins cannot work when WiFi is in use.
#include <WebServer.h>  // standard library

// Libraries for temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>

// Libraries for servo, normal arduino "Servo.h" library doesn't work with ESP32
#include <ESP32_Servo.h>

// here you post web pages to your home's intranet which will make page debugging easier, as you just need to refresh the browser as opposed to reconnection to the web server.
// uncomment one line on your preference.
// #define USE_INTRANET
// #define USE_AP
#define USE_WiFi

// replace this with your homes intranet connect parameters (for Option 1)
#define LOCAL_SSID "Dialog 4G"
#define LOCAL_PASS "DABDFFF8DE4"

// once you are ready to go live, these settings are what you client will connect to (for Option 2 and Option 3)
#define AP_SSID "Aquamate Tank 1"
#define AP_PASS "11111111"

// defines for pins for sensors, outputs etc.
#define PIN_OUT_0 16  // Output 1 (On board LED is D4 or GPIO2)
#define PIN_OUT_1 18  // Output 2
#define PIN_PWM 27    // PWM signal to control a fan speed
#define PIN_A0 33     // analog input of pH sensor
#define PIN_A1 14     // anaog input of temperature sensor

// variables to store measure data and sensor states
bool Device0 = false, Device1 = false;

int PWMRange = 0;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[2] Setting of PWM properties (Only for ESP32)
const int pwm_ch = 0;
const int pwm_freq = 1000;
const int pwm_res = 8;

int BitsA0 = 0;
float VoltsA0 = 0;
float BitsA1 = 0, VoltsA1 = 0; // temperature in Celsius

uint32_t SensorUpdate = 0;
uint32_t FeederUpdate = 0;
uint32_t FeederDelay = 0;
bool servo_state = false;

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char buf[32];

// pH sensor parameters
int avg_phValue;   //Store the average value of the sensor feedback
int ph_buf[10],temp;

// temperature sensor parameters
OneWire oneWire(PIN_A1);                // setup a oneWire instance
DallasTemperature tempSensor(&oneWire); // pass oneWire to DallasTemperature library

Servo servo_1;

// variable for the IP address
IPAddress ip;

// static wifi configuration, instead of DHCP
IPAddress s_PageIP(192, 168, 0, 170);
IPAddress s_gateway(192, 168, 0, 1);
IPAddress s_subnet(255, 255, 255, 0);

// custom AP configuration
IPAddress PageIP(10,0,1,1);
IPAddress gateway(10,0,1,1);
IPAddress subnet(255,255,255,0);

// declare an object of ESP8266WebServer library
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[3] For ESP8266
// ESP8266WebServer server(80);
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[3] For ESP32
WebServer server(80);

void setup() {

  // standard stuff here
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[4] For ESP8266
  //Serial.begin(115200);
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[4] For ESP32
  Serial.begin(9600);
  tempSensor.begin();    // initialize the sensor

  servo_1.attach(PIN_PWM);
  servo_1.write(90);
  pinMode(PIN_OUT_0, OUTPUT);
  pinMode(PIN_OUT_1, OUTPUT);

  // turn off devices by default
  digitalWrite(PIN_OUT_0, Device0);
  digitalWrite(PIN_OUT_1, Device1);

  // configure PWM functionalitites
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[5] For ESP8266
  // analogWrite(PIN_PWM, PWMRange);
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[5] For ESP32
  // ledcSetup(pwm_ch, pwm_freq, pwm_res);
  // ledcAttachPin(PIN_PWM, pwm_ch); //there are 16 pwm channels from 0 to 15
  // ledcWrite(pwm_ch, PWMRange);
  
  delay(1000);

  // just an update to progress
  Serial.print("Starting server \n");
  Serial.print("--------------- \n");

  // Option 1
  // if you have this #define USE_INTRANET, you will connect to your home intranet, again makes debugging easier.
  // for this you can only connect to above defined home intranet
  #ifdef USE_INTRANET
    //static ip configuration
    if (!WiFi.config(s_PageIP, s_gateway, s_subnet)) {
      Serial.print("WiFi Configuration failed. \n");
    }
    WiFi.begin(LOCAL_SSID, LOCAL_PASS);
    // check ESP is connected to a wi-fi network
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.print("\n");
    Serial.print("WiFi connected..! \n");
    printWifiStatus();
  #endif

  // Option 2
  // if you have this #define USE_AP, ESP will create a soft access point.
  // (an intranet with no internet connection. But Clients can connect to your intranet and see the web page you are about to serve up.)
  #ifdef USE_AP
    //set custom ip for portal
    WiFi.softAPConfig(PageIP, gateway, subnet);
    delay(100);
    WiFi.softAP(AP_SSID, AP_PASS);
    delay(100);
    ip = WiFi.softAPIP();
    Serial.print("Access Point Created..! \n");
    Serial.print("SSID: "); Serial.println(AP_SSID);
    Serial.print("IP address: "); Serial.println(ip);
  #endif

  // Option 3
  // if you have this #define USE_WiFi, ESP will use WiFi manager.
  // Connect to an available any WiFi network (should know the password)
  #ifdef USE_WiFi
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    // If we don't reset WiFi manager settings, ESP will automatically connect to the previously connected network, if it's password is unchanged.
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //wm.resetSettings(); //reset previous ssids
    //set custom ip for portal
    wm.setAPStaticIPConfig(PageIP, gateway, subnet);
    //static ip configuration
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //wm.setSTAStaticIPConfig(s_PageIP, s_gateway, s_subnet);
    bool res;
    res  = wm.autoConnect(AP_SSID, AP_PASS);
    Serial.println();
    if (!res) {
      Serial.println("WiFi connecting failed.");
      ESP.restart();
    }
    else {
      Serial.println("WiFi connected..!");
    }
    printWifiStatus();
  #endif

  Serial.print("\n\n");

  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent
  server.on("/", SendWebsite);

  // upon esp getting /XML string, ESP will build and send the XML, this is how we refresh just parts of the web page
  server.on("/xml", SendXML);

  // upon ESP getting /UPDATE_SLIDER string, ESP will execute the UpdateSlider function
  // same notion for the following .on calls
  // add as many as you need to process incoming strings from your web page
  // as you can imagine you will need to code some javascript in your web page to send such strings
  // this process will be documented in the SuperMon.h web page code
  server.on("/UPDATE_SLIDER", UpdateSlider);
  server.on("/BUTTON_0", ProcessButton_0);
  server.on("/BUTTON_1", ProcessButton_1);

  // finally begin the server
  server.begin();
}

void loop() {
  // you main loop that measures, processes, runs code, etc.
  // note that handling the "on" strings from the web page are NOT in the loop
  // that processing is in individual functions all managed by the wifi lib

  // in my example here every 50 ms, i measure some analog sensor data (my finger dragging over the pins and process accordingly.
  // analog input can be temperature sensors, light sensors, digital pin sensors, etc.
  if ((millis() - SensorUpdate) >= 50) {
    //Serial.println("Reading Sensors");
    SensorUpdate = millis();

    phSense();
    
    tempSensor.requestTemperatures();             // send the command to get temperatures
    BitsA1 = tempSensor.getTempCByIndex(0)+ 1.2;  // read temperature in Celsius
    VoltsA1 = BitsA1;
  }

  if ((Device0 == 1) && ((millis() - FeederUpdate) >= 60000*PWMRange)) {
    //Serial.print("servo on ");
    servo_1.write(0);
    FeederUpdate = millis();
    FeederDelay = FeederUpdate;
    servo_state = true;
  }
  if ((servo_state == true) && ((millis() - FeederDelay) >= 5000)) {
    //Serial.print("servo off ");
    servo_1.write(90);
    servo_state = false;
  }
  
  // no matter what, you must call this handleClient repeatidly--otherwise the web page will not get instructions to do something.
  server.handleClient();
}

void phSense() {
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  {
    ph_buf[i]=analogRead(PIN_A0);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(ph_buf[i]>ph_buf[j])
      {
        temp=ph_buf[i];
        ph_buf[i]=ph_buf[j];
        ph_buf[j]=temp;
      }
    }
  }
  avg_phValue=0;
  for(int i=2;i<8;i++)             //take the average value of 6 center sample
    avg_phValue+=ph_buf[i];
  BitsA0=avg_phValue/6;
  VoltsA0=BitsA0*3.3*3.3/4095-1;   //convert the analog into ph value
}

// function managed by an .on method to handle slider actions on the web page
// this example will get the passed string called VALUE and convert to a pwm value and control the fan speed
void UpdateSlider() {

  // many I hate strings, but wifi lib uses them...
  String t_state = server.arg("VALUE");

  // convert the string sent from the web page to an int
  PWMRange = t_state.toInt();
  // Serial.print("UpdateSlider"); Serial.println(PWMRange);
  // now set the PWM duty cycle
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[6] For ESP8266
  // analogWrite(PIN_PWM, PWMRange);
  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[6] For ESP32
  // ledcWrite(pwm_ch, PWMRange);

  // YOU MUST SEND SOMETHING BACK TO THE WEB PAGE--BASICALLY TO KEEP IT LIVE

  // option 1: send no information back, but at least keep the page live
  // just send nothing back
  // server.send(200, "text/plain", ""); //Send web page

  // option 2: send something back immediately, maybe a pass/fail indication, maybe a measured value.
  // here is how you send data back immediately and NOT through the general XML page update code.
  // my simple example guesses at fan speed--ideally measure it and send back real data.
  // i avoid strings at all caost, hence all the code to start with "" in the buffer and build a simple piece of data.
  strcpy(buf, "");
  sprintf(buf, "%d", PWMRange);
  sprintf(buf, buf);

  // now send it back
  server.send(200, "text/plain", buf); //Send web page
}

// now process button_0 press from the web site. Typical applications are the used on the web client can turn on/off a light, a fan, disable something etc.
void ProcessButton_0() {

  Device0 = !Device0;
  digitalWrite(PIN_OUT_0, Device0);
  Serial.print("Button 0 "); Serial.println(Device0);
  // regardless if you want to send stuff back to client or not you must have the send line--as it keeps the page running,
  // if you don't want feedback from the MCU--or let the XML manage sending feeback.

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status like the illumination status will be send in the main XML page update code
  server.send(200, "text/plain", ""); //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feedback immediataly
  // note you must have reading code in the java script
  /*
    if (Device0) {
      server.send(200, "text/plain", "1"); //Send web page
    }
    else {
      server.send(200, "text/plain", "0"); //Send web page
    }
  */
}

// same notion for processing button_1
void ProcessButton_1() {

  // just a simple way to toggle a LED on/off. Much better ways to do this
  Serial.println("Button 1 press");
  Device1 = !Device1;
  digitalWrite(PIN_OUT_1, Device1);
  Serial.print("Button 1 "); Serial.println(Device1);
  // regardless if you want to send stuff back to client or not you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or send all data via XML use this method
  // sending feeback
  server.send(200, "text/plain", ""); //Send web page

  // if you want to send feedback immediataly
  // note you must have proper code in the java script to read this data stream
  /*
    if (Device1) {
      server.send(200, "text/plain", "SUCCESS"); //Send web page
    }
    else {
      server.send(200, "text/plain", "FAIL"); //Send web page
    }
  */
}

// code to send the main web page
// PAGE_MAIN is a large char defined in SuperMon.h
void SendWebsite() {
  Serial.println("sending web page");
  // you may have to play with this value, big pages need more porcessing time, and hence a longer timeout that 200 ms
  server.send(200, "text/html", PAGE_MAIN);
}

// code to send the main web page
// I avoid string data types at all cost, hence all the char manipulation code
void SendXML() {
  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // send bits0
  sprintf(buf, "<B0>%d.%d</B0>\n", BitsA0);
  strcat(XML, buf);
  // send Volts0
  sprintf(buf, "<V0>%d.%d</V0>\n", (int) (VoltsA0), abs((int) (VoltsA0 * 10)  - ((int) (VoltsA0) * 10)));
  strcat(XML, buf);

  // send bits1
  sprintf(buf, "<B1>%d.%d</B1>\n", (int) (BitsA1), abs((int) (BitsA1 * 10)  - ((int) (BitsA1) * 10)));
  strcat(XML, buf);
  // send Volts1
  sprintf(buf, "<V1>%d.%d</V1>\n", (int) (VoltsA1), abs((int) (VoltsA1 * 10)  - ((int) (VoltsA1) * 10)));
  strcat(XML, buf);

  // show devices status
  if (Device0) {
    strcat(XML, "<DEVICE0>1</DEVICE0>\n");
  }
  else {
    strcat(XML, "<DEVICE0>0</DEVICE0>\n");
  }

  if (Device1) {
    strcat(XML, "<DEVICE1>1</DEVICE1>\n");
  }
  else {
    strcat(XML, "<DEVICE1>0</DEVICE1>\n");
  }

  strcat(XML, "</Data>\n");
  // wanna see what the XML code looks like?
  // actually print it to the serial monitor and use some text editor to get the size
  // then pad and adjust char XML[2048]; above
  Serial.println(XML);

  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/xml", XML);
}

// I think I got this code from the wifi example
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Open http://");
  Serial.println(ip);
}

// end of code
