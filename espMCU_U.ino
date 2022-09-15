
#include <Arduino.h>
// Included Libraries
    #include <ESP8266WiFi.h>
    #include <ESPAsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include "LittleFS.h"
    #include <Arduino_JSON.h>
    #include <AsyncElegantOTA.h>

 /*================================== espMCU v6 [ Firmware Info + Hardcoded Options ] =============================================*/
//                                                                                                                                  ||                      
//                                                                                                                                  ||                      
                String fwVersion = "ESPmcu-v6-mk.U";             // Configure Firmware Revision Label                                || 
             //    const char* OTAuser = "admin";               // OTA Update Username                                              || 
             //    const char* OTApassword = "admin";           // OTA Update Password [Toggle Comments @ Lines 534/535 To Enable]  ||
//                                                                                                                                  || 
//                                                                                                                                  ||                        
                #define NUM_OUTPUTS  4                          // Set # Of Digital Outputs                                         || 
                int outputGPIOs[NUM_OUTPUTS] = {5, 4, 0, 2 };   // Assign GPIO Output Pins                                          || 
                const int analog0pin = A0;                      // Assign Analog Input Pin                                          || 
                const int PWMpin = 14;                          // Assign PWM Output Pin                                            || 
                const int resetSwitch = 16;                     // Assign Factory FW Reset Pin [Pull Low, 5 Seconds]                || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
          // Set ESP's LAN IP Address                                                                                               || 
                IPAddress localIP;                              // User Configurable LAN Gateway IP                                 || 
              //IPAddress localIP(192, 168, 1, 99);             // Hard Coded LAN Gateway IP                                        || 
//                                                                                                                                  || 
          // Set LAN Gateway IP Address                                                                                             || 
              //IPAddress localGateway;                         // User Configurable LAN Gateway IP                                 || 
                IPAddress localGateway(192, 168, 1, 1);         // Hard Coded LAN Gateway IP                                        || 
                IPAddress subnet(255, 255, 0, 0);               // Hard Coded LAN Subnet                                            ||                                                                                                                                 || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
//                                                                                                                                  ||                      
//                                                                                                                                  || 
 /*================================== espMCU v6 [ Firmware Info + Hardcoded Options ] ==============================================*/






//                                                                                                                               |
    AsyncWebServer server(80);          // Create AsyncWebServer object on port 80                                               |
    AsyncWebSocket ws("/ws");           // Create a WebSocket object                                                             |
    AsyncEventSource events("/events"); // Create an Event Source on /events                                                     |
    JSONVar readings;                   // Json Variable to Hold Sensor Readings                                                 |
//                                                                                                                               |
// Timer Variables - WiFi Manager                                                                                                |
    unsigned long previousMillis = 0;   // [Timer Counter Variable]//                                                            |
    const long interval = 20000;        // Time To Wait For Sucessful Wi-Fi Connection (Milliseconds)                            |
    boolean restart = false;            // [Device Restart Flag]                                                                 |
// Timer Variables - SSE                                                                                                         |
    unsigned long lastTime = 0;         // [Timer Counter Variable]                                                              |
    unsigned long timerDelay = 3000;    // Interval Between Sending Updated Sensor/Data Values (Milliseconds)                    |
// Timer Variables - Factory Reset                                                                                               |
    byte currSwitch, lastSwitch;        //                                                                                       |
    unsigned long timeStart;            //                                                                                       |
    bool bCheckingSwitch;               //                                                                                       |
    const int blinkLEDpin = LED_BUILTIN;// the number of the LED pin                                                             |
//                                                                                                                               |
//                                                                                                                               |
//                                                                                                                               |
      const char* PARAM_INPUT = "value";    // Variable To Store 8bit PWM Parameter From Range Slider [Via HTTP POST Request]    |
      const char* PARAM_INPUT_0 = "apmode";   // Field Name Var. To Store SSID Parameter WiFi Configuration [Via HTTP POST Req.]   |
      const char* PARAM_INPUT_1 = "ssid";   // Field Name Var. To Store SSID Parameter WiFi Configuration [Via HTTP POST Req.]   |
      const char* PARAM_INPUT_2 = "pass";   // Field Name Var. To Store PASS Parameter WiFi Configuration [Via HTTP POST Req.]   |
      const char* PARAM_INPUT_3 = "ip";     // Field Name Var. To Store IP Parameter WiFi Configuration [Via HTTP POST Req.]     |
      const char* PARAM_INPUT_4 = "gateway";// Field Name Var. To Store GATE Parameter WiFi Configuration [Via HTTP POST Req.]   |  
      const char* PARAM_INPUT_5 = "espid";  // Field Name Var. To Store ID Parameter WiFi Configuration [Via HTTP POST Req.]     |  
      String ssid;// Variable To Store SSID Parameter In RAM Before Writing To Flash w/ LittleFS |
      String pass;// Variable To Store PWD Parameter In RAM Before Writing To Flash w/ LittleFS |
      String ip;  // Variable To Store IP Parameter In RAM Before Writing To Flash w/ LittleFS |
      String gateway;// Variable To Store GTWY Parameter In RAM Before Writing To Flash w/ LittleFS |
      String espid; // Variable To Store ID Parameter In RAM Before Writing To Flash w/ LittleFS |
      const char* ssidPath = "/ssid.txt";// Var. For Path/Filename Of SSID Parameter Location In Flash Mem. w/ LittleFS |
      const char* passPath = "/pass.txt";// Var. For Path/Filename Of PASS Parameter Location In Flash Mem. w/ LittleFS |
      const char* ipPath = "/ip.txt";// Var. For Path/Filename Of IP Parameter Location In Flash Mem. w/ LittleFS |
      const char* gatewayPath = "/gateway.txt";// Var. For Path/Filename Of GATE Parameter Location In Flash Mem. w/ LittleFS |
      const char* espidPath = "/espid.txt";// Var. For Path/Filename Of ID Parameter Location In Flash Mem. w/ LittleFS |

      String sliderValue = "0";       

      int AnalogPercent;
      float analoginput;
      int absRSSI = abs(WiFi.RSSI());
      int WiFipercentage;
      String wifiIcon = "<i class=\"sdh-sig0 sigbars\"></i>"; // HTML/CSS Vector Font Representation Of WiFi Signal Stregnth Meter
      int WifiBars; // Number of WiFi "Signal Bars" Represented Numerically 0-4




    String getAnalogInput() {
      float analoginput = analogRead(analog0pin);
      return String(analoginput);
    }
    String getMACAddress() {
      String MACAddressDisplay = String() + WiFi.macAddress()[0] + WiFi.macAddress()[1] + WiFi.macAddress()[2] + WiFi.macAddress()[3] + WiFi.macAddress()[4] + WiFi.macAddress()[5] + WiFi.macAddress()[6] + WiFi.macAddress()[7] + WiFi.macAddress()[8] + WiFi.macAddress()[9] + WiFi.macAddress()[10] + WiFi.macAddress()[11] + WiFi.macAddress()[12] + WiFi.macAddress()[13] + WiFi.macAddress()[14]+ WiFi.macAddress()[15] + WiFi.macAddress()[16];
      return String(MACAddressDisplay);
    }
    String getIPAddress() {
      String IPaddress =  WiFi.localIP().toString();
      return String(IPaddress);
    }
    String getWLANSSID() {
      String WLANSSIDDisplay = (WiFi.SSID());
      return String(ssid);
    }
    String getWIFIpct() {
      return String(WiFipercentage);
    }
    String getAnalogPercent() {
      analoginput = analogRead(analog0pin);
      int a0pctInt;
      int oldVal;
      int TOLERANCE= 5;
            int val = analogRead(analog0pin);
            val = map(val, 0, 1023, 0, 100);
            int diff = abs(val - oldVal);
              if (diff > TOLERANCE){
                 oldVal = val;// only save if the val has changed enough to avoid slowly drifting
              }
            a0pctInt = val;
      return String(a0pctInt);
    }
      String getWiFiSigStr() {
      getWiFisignal();
      return String(WiFipercentage);
    }
      String getWiFiIcon() {
      getWiFisignal();
      return String(wifiIcon);
    }
    String getFWversion() {
      return String(fwVersion);
    }
    String getESPid() {
      return String(espid);
    }
    String getChipID() {
         int chipIDint = ESP.getChipId();
           String chipID = String(chipIDint);
      return String(chipID);
    }
    String getFlashID() {
         int flashIDint = ESP.getFlashChipId();
           String flashID = String(flashIDint);
      return String(flashID);
    }
    String getCPUfreq() {
         int freqCPUint = ESP.getCpuFreqMHz();
           String CPUfreq = String(freqCPUint);
      return String(CPUfreq);
    }
    String getSketchSz() {
         int sketchSzint = ESP.getSketchSize();
         sketchSzint = ((sketchSzint)/1024);
           String sketchSz = String(sketchSzint);
      return String(sketchSz);
    }
    String getFreeSpace() {
         int freeSpaceint = ESP.getFreeSketchSpace();
              freeSpaceint = ((freeSpaceint)/1024);
           String freespace = String(freeSpaceint);
      return String(freespace);
    }
   String processor(const String& var){
      if (var == "ANALOGINPUT"){
        return getAnalogInput();
      }
      else if (var == "ANALOGPERCENT"){
        return getAnalogPercent();
      }
      else if (var == "PWMVALUE"){
        return sliderValue;
      }
      else if (var == "IPADDRESS"){
        return getIPAddress();
      }
      else if (var == "MACADDRESS"){
        return getMACAddress();
      }
       else if (var == "WLANSSID"){
        return getWLANSSID();
      }
       else if (var == "FWVERSION"){
        return getFWversion();
      }
         else if (var == "ESPID"){
        return getESPid();
      }
       else if (var == "CHIPID"){
        return getChipID();
      }
         else if (var == "FLASHID"){
        return getFlashID();
      }
       else if (var == "CPUFREQ"){
        return getCPUfreq();
      }
         else if (var == "WIFIPERCENT"){
        return getWIFIpct();
      }
       else if (var == "SKETCHSZ"){
        return getSketchSz();
      }
         else if (var == "FREESPACE"){
        return getFreeSpace();
      }
     return String(); // [/String&var]
  }                   // [/String Processor]

    void getWiFisignal(){
       absRSSI = abs(WiFi.RSSI());
       WiFipercentage = map(absRSSI, 80, 30, 0, 100);
       if (WiFipercentage>=90&&WiFipercentage<=100){
        WifiBars = 4;
        wifiIcon = "<i class=\"sdh-bars4 sigbars\"></i>";
       }
       else if(WiFipercentage>=70&&WiFipercentage<=89){
        WifiBars = 3;
        wifiIcon = "<i class=\"sdh-bars3 sigbars\"></i>";
       }
       else if(WiFipercentage>=50&&WiFipercentage<=69){
        WifiBars = 2;
        wifiIcon = "<i class=\"sdh-bars2 sigbars\"></i>";
       }
       else if(WiFipercentage>=20&&WiFipercentage<=49){
        WifiBars = 1;
        wifiIcon = "<i class=\"sdh-bars1 sigbars\"></i>";
       }
       else if(WiFipercentage>=0&&WiFipercentage<=19){
        WifiBars = 0;
        wifiIcon = "<i class=\"sdh-bars0 sigbars\"></i>";
       }
    }

    void analogtoPCT() {
      int a0pctInt;
      int oldVal;
      int TOLERANCE= 5;
            int val = analogRead(analog0pin);
            val = map(val, 0, 1023, 0, 100);
            int diff = abs(val - oldVal);
              if (diff > TOLERANCE){
                 oldVal = val;// only save if the val has changed enough to avoid slowly drifting
              }
            a0pctInt = val;
            AnalogPercent = val; 

      }
   

    String getSensorReadings(){    // Get Sensor Readings and Return JSON object
      readings["temperature"] = String(analogRead(analog0pin));
      //readings["humidity"] =  String(AnalogPercent);
     readings["analog0"] = String(analogRead(analog0pin));
     readings["humidity"] =  String(getWiFiSigStr());
      readings["rssi"] =  String(getWiFiSigStr());  
      String jsonString = JSON.stringify(readings);
      return jsonString;
    }
    
    String getOutputStates(){    // Get Digital Output (GPIO) Readings and Return JSON object
      JSONVar myArray;
      for (int i =0; i<NUM_OUTPUTS; i++){
        myArray["gpios"][i]["output"] = String(outputGPIOs[i]);
        myArray["gpios"][i]["state"] = String(digitalRead(outputGPIOs[i]));
      }
      String jsonString = JSON.stringify(myArray);
      return jsonString;
    }
    
    void notifyClients(String state) {
      ws.textAll(state);
    }
    
    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
      AwsFrameInfo *info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        if (strcmp((char*)data, "states") == 0) {
          notifyClients(getOutputStates());
        }
        else{
          int gpio = atoi((char*)data);
          digitalWrite(gpio, !digitalRead(gpio));
          notifyClients(getOutputStates());
        }
      }
    }
    
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,AwsEventType type,
                 void *arg, uint8_t *data, size_t len) {
      switch (type) {
        case WS_EVT_CONNECT:
          Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
          break;
        case WS_EVT_DISCONNECT:
          Serial.printf("WebSocket client #%u disconnected\n", client->id());
          break;
        case WS_EVT_DATA:
          handleWebSocketMessage(arg, data, len);
          break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
          break;
      }
    }

    void initWebSocket() {
        ws.onEvent(onEvent);
        server.addHandler(&ws);
    }
    
    // Initialize LittleFS
    void initFS() {
      if (!LittleFS.begin()) {
        Serial.println("An error has occurred while mounting LittleFS");
      }
      else{
        Serial.println("LittleFS mounted successfully");
      }
    }
    
    // Read File from LittleFS
    String readFile(fs::FS &fs, const char * path){
      Serial.printf("Reading file: %s\r\n", path);
    
      File file = fs.open(path, "r");
      if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return String();
      }
    
      String fileContent;
      while(file.available()){
        fileContent = file.readStringUntil('\n');
        break;
      }
      file.close();
      return fileContent;
    }
    
    // Write file to LittleFS
    void writeFile(fs::FS &fs, const char * path, const char * message){
      Serial.printf("Writing file: %s\r\n", path);
    
      File file = fs.open(path, "w");
      if(!file){
        Serial.println("- failed to open file for writing");
        return;
      }
      if(file.print(message)){
        Serial.println("- file written");
      } else {
        Serial.println("- frite failed");
      }
      file.close();
    }

    void factoryReset(){
       Serial.println("Beginning Factory Reset...");
       digitalWrite(blinkLEDpin, HIGH); delay(300); digitalWrite(blinkLEDpin, LOW); delay(500); digitalWrite(blinkLEDpin, HIGH); delay(300); digitalWrite(blinkLEDpin, LOW); delay(300);
       LittleFS.remove("/espid.txt");        // Delete .TXT File Storing ID Parameter In LittleFS/Flash Memory      |
       LittleFS.remove("/ssid.txt");         // Delete .TXT File Storing SSID Parameter In LittleFS/Flash Memory    |
       LittleFS.remove("/pass.txt");         // Delete .TXT File Storing PASS Parameter In LittleFS/Flash Memory    |
       LittleFS.remove("/ip.txt");           // Delete .TXT File Storing IP Parameter In LittleFS/Flash Memory      |
       LittleFS.remove("/gateway.txt");      // Delete .TXT File Storing Gateway Parameter In LittleFS/Flash Memory |
       Serial.println("Removed All Device Configuration Settings. Rebooting...");
       delay(3000);
       restart = true;
       ESP.restart();
    }
    

    bool initWiFi() {       // Initialize WiFi
      if(ssid=="" || ip==""){
        Serial.println("Undefined SSID or IP address.");
        return false;
      }
      WiFi.mode(WIFI_STA);
      WiFi.hostname(espid.c_str());
      localIP.fromString(ip.c_str());
      localGateway.fromString(gateway.c_str());
      if (!WiFi.config(localIP, localGateway, subnet)){
        Serial.println("STA Failed to configure");
        return false;
      }
      WiFi.begin(ssid.c_str(), pass.c_str());
      Serial.println("Connecting to WiFi...");
      delay(15000);
      if(WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect.");
        return false;
      }
      Serial.println(WiFi.localIP());
      return true;
    }

    void setup() {
          Serial.begin(115200);  // Serial port for debugging purposes
            
            // Set GPIOs as outputs
              for (int i =0; i<NUM_OUTPUTS; i++){
                pinMode(outputGPIOs[i], OUTPUT);
              }

  analogWrite(PWMpin, sliderValue.toInt());

            //  BEGIN Factory Reset Setup Code
            pinMode( resetSwitch, INPUT_PULLUP );
            lastSwitch = digitalRead( resetSwitch );
            bCheckingSwitch = false;
            //  END Factory Reset Setup Code

          initFS();
          initWebSocket();
        
          // Load values saved in LittleFS
          espid = readFile(LittleFS, espidPath);
          ssid = readFile(LittleFS, ssidPath);
          pass = readFile(LittleFS, passPath);
          ip = readFile(LittleFS, ipPath);
          gateway = readFile (LittleFS, gatewayPath);
          Serial.println(espid);
          Serial.println(ssid);
          Serial.println(pass);
          Serial.println(ip);
          Serial.println(gateway);
    
      if(initWiFi()) {
        // Route for root / web page
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send(LittleFS, "/index.html", "text/html", false, processor);
        });
      server.serveStatic("/", LittleFS, "/");   // Serves Contents Of Root LittleFS 'data' Directory
    
      // Web Server Root URL
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){   // Serves index.html on HTTP Request @ WWW Root Level (Except in AP 
        request->send(LittleFS, "/index.html", "text/html",false, processor);
      });

     server.on("/fwversion", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getFWversion().c_str());
        });

        server.on("/analogpercent", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain",  getAnalogPercent().c_str());
        });


     server.on("/wifipercent", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getWIFIpct().c_str());
        });

        
     server.on("/wifisig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getWiFiSigStr().c_str());
        });
        
     server.on("/wifiicon", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getWiFiIcon().c_str());
        });

     server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(200, "text/plain", "Done. ESP will restart.");
          restart = true;
        });

     server.on("/factoryreset", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(200, "text/plain", "Done. ESP Will Reset To Default Configuration.");
          factoryReset();
        });

      // Request for the latest sensor readings
      server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorReadings();
        request->send(200, "application/json", json);
        json = String();
      });
      
      // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
      server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam(PARAM_INPUT)) {
          inputMessage = request->getParam(PARAM_INPUT)->value();
          sliderValue = inputMessage;
          analogWrite(PWMpin, sliderValue.toInt());
        }
        else {
          inputMessage = "No message sent";
        }
        Serial.println(inputMessage);
        request->send(200, "text/plain", "OK");
      });
    
      events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
          Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 10000);
      });

      server.addHandler(&events);

         AsyncElegantOTA.begin(&server);                           // Begin ElegantOTA Without Usr/Pwd For Remote Login
      // AsyncElegantOTA.begin(&server, "OTAuser", "OTApassword"); // Begin ElegantOTA And Require Usr/Pwd For Remote Login
      
      server.begin();  // Start HTTP Server [Inside Environment w/ Sucessful Connection To WiFi Router/AP in STA Mode ]

  } // [ /if(initWiFi()){ (Sucessful Connection To AP in STA Mode)]

      else {
        // Connect to Wi-Fi network with SSID and password
        Serial.println("Setting AP (Access Point)");
        
        WiFi.softAP("espMCU-WiFiSetup", NULL);      // 'NULL' (Without Single Quotations) Sets WPA Open Access Point
     // WiFi.softAP("espMCU_Configuration", "ESP8266"); //  Sets WiFi Access Point's WPA2 SSID/Passphrase
    
        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP); 
    
        // Web Server Root URL
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(LittleFS, "/wifimanager.html", "text/html",false, processor);
        });
              server.serveStatic("/", LittleFS, "/");   // Serves Contents Of Root LittleFS 'data' Directory
        server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send(LittleFS, "/index.html", "text/html", false, processor);
        });
          server.on("/softap", HTTP_GET, [](AsyncWebServerRequest *request) {
          request->send(LittleFS, "/index.html", "text/html", false, processor);
        });
    
        server.on("/fwversion", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getFWversion().c_str());
        });

        server.on("/analogpercent", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain",  getAnalogPercent().c_str());
        });


     server.on("/wifipercent", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getWIFIpct().c_str());
        });

        
     server.on("/wifisig", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getWiFiSigStr().c_str());
        });
        
     server.on("/wifiicon", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/plain", getWiFiIcon().c_str());
        });

     server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(200, "text/plain", "Done. ESP will restart.");
          restart = true;
        });

     server.on("/factoryreset", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(200, "text/plain", "Done. ESP Will Reset To Default Configuration.");
          factoryReset();
        });

      // Request for the latest sensor readings
      server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorReadings();
        request->send(200, "application/json", json);
        json = String();
      });
      
      // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
      server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String inputMessage;
        // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
        if (request->hasParam(PARAM_INPUT)) {
          inputMessage = request->getParam(PARAM_INPUT)->value();
          sliderValue = inputMessage;
          analogWrite(PWMpin, sliderValue.toInt());
        }
        else {
          inputMessage = "No message sent";
        }
        Serial.println(inputMessage);
        request->send(200, "text/plain", "OK");
      });
    
      events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
          Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 10000);
      });

      server.addHandler(&events);

         AsyncElegantOTA.begin(&server);                           // Begin ElegantOTA Without Usr/Pwd For Remote Login
      // AsyncElegantOTA.begin(&server, "OTAuser", "OTApassword"); // Begin ElegantOTA And Require Usr/Pwd For Remote Login
      
        server.serveStatic("/", LittleFS, "/");
        
        server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
          int params = request->params();
          for(int i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isPost()){
              if (p->name() == PARAM_INPUT_1) { // HTTP POST Value For: WiFi AP SSID (Network Name)
                ssid = p->value().c_str();
                Serial.print("SSID set to: ");
                Serial.println(ssid);
                writeFile(LittleFS, ssidPath, ssid.c_str());
              }
              if (p->name() == PARAM_INPUT_2) { // HTTP POST Value For: WiFi AP Password
                pass = p->value().c_str();
                Serial.print("Password set to: ");
                Serial.println(pass);
                writeFile(LittleFS, passPath, pass.c_str());
              }
              if (p->name() == PARAM_INPUT_3) { // HTTP POST Value For: IP Address
                ip = p->value().c_str();
                Serial.print("IP Address set to: ");
                Serial.println(ip);
                writeFile(LittleFS, ipPath, ip.c_str());
              }
              if (p->name() == PARAM_INPUT_4) { // HTTP POST Value For: Gateway
                gateway = p->value().c_str();
                Serial.print("Gateway set to: ");
                Serial.println(gateway);
                writeFile(LittleFS, gatewayPath, gateway.c_str());
              }
              if (p->name() == PARAM_INPUT_5) { // HTTP POST Value For: ESPid ('Friendly' Device Name/Label)
                espid = p->value().c_str();
                Serial.print("espid set to: ");
                Serial.println(espid);
                writeFile(LittleFS, espidPath, espid.c_str());
              }
              //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
          }
          restart = true;
          request->send(200, "text/plain", "Done. ESP will restart and connect to SSID: " + ssid +". Device is labeled: " + espid + "and will have IP address: " + ip);
        });

        server.begin();  // Start HTTP Server [In SoftAP Mode, Inside Environment w/ Failed Connection To WiFi Router/AP]
     
      } // [ /else (initWiFi()){ (After Starting In SoftAP Mode For WiFi Config Portal)]
  
  } //[END SETUP]
    
    void loop() {
      if ((millis() - lastTime) > timerDelay) {
        // Send Events to the client with the Sensor Readings Every __ seconds
        events.send("ping",NULL,millis());
        events.send(getSensorReadings().c_str(),"new_readings" ,millis());
        lastTime = millis();
      }


    //  BEGIN End Restart Check Loop
       if (restart){
        delay(5000);
        ESP.restart();
      }// End Restart Check Loop

    //  BEGIN Factory Reset Check Loop
    currSwitch = digitalRead( resetSwitch );
    if( currSwitch != lastSwitch ){
        if( currSwitch == LOW ) {
            timeStart = millis();
            bCheckingSwitch = true;
        }//if
        else {
         bCheckingSwitch = false;
             }//else
        lastSwitch = currSwitch;
    }//if
    if( bCheckingSwitch ){
        if( (millis() - timeStart ) >= 5000 )
            factoryReset();
         }//if
    //  END Factory Reset Check Loop


    // Handle Network/WiFi Functions
    //========================================
            AsyncElegantOTA.loop();
            ws.cleanupClients();
    
    // Handle GPIO Functions
    //========================================
            analogtoPCT();           // Display Analog Signal Percentage
            delay(1000); 
            getWiFisignal();
            delay(1000); 

       if (restart){
        delay(5000);
        ESP.restart();
      }

  } //[END LOOP]
