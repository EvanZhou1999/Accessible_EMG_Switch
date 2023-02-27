/*
 * WIRELESS ACCESSIBLE EMG SWITCH WEARABLE===============================
 * Using ESPAsyncWebServer Library
 * HARDWARE CONFIGURATIONS ==============================================
 * - ESP32-WROOM-DA Module
 * - RGB LED Pixel Light
 *    - LED Pin 32
 *    - num-Pixels 7
 * - Software Switch Relay
 *    - Pin 27
 * - EMG Sensor
 *    - Pin 34
 * - Vibration Motor
 *    - Pin 18
 * =======================================================================
 * =======================================================================
 */

// Import WiFi Libraries
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp_wpa2.h>
#include <esp_wifi.h>
#include <ESPAsyncWebServer.h>
// Import DNS Library
#include <ESPmDNS.h>

// Import FreeRTOS
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Other Libraries
#include <time.h>
#include <stdlib.h>
#include <EEPROM.h>

// Include html files
#include "index_html.h"

// Import wireless configurations
#include "wireless_config.h"
// Import LED tasks
#include "led_task.h"


// Software Switch Relay Pin Setup
#define SOFT_RELAY_PIN 27

// EMG Sensor Pin Setup
#define EMG_SENSOR_PIN 34
#define THRESHOLD_ADDR 2

// Vibration Motor Pin Setup
#define VIBRATION_MOTOR_PIN 18
#define MOTOR_CHANNEL 1
#define STRENGTH_SOFT 80
#define STRENGTH_MILD 160
#define STRENGTH_HIGH 250
#define VIBRATION_SETTING_ADDR 1

/*
 * =======================================================
 * Instantiate Global Objects/Devices
 * =======================================================
 */

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


/*
 * =======================================================
 * Global Variables
 * =======================================================
 */

// Reading acquired from EMG Sensor
int EMGreading = 0;
// Sensor will take the average of the following BatchSize and save as its reading
const int sampleBatchSize = 50;
// Threshold value to activate switch
// Testing run threshold: 500 (just for reference)
int EMGthreshold = 0;  // 0-4095

// vibration motor settings
int enableVibration = 1;  // 0 or 1

// Initialize Normal Status indicator color: 100, 20, 0
int color_normal_r = 100;
int color_normal_g = 20;
int color_normal_b = 0;


/*
 * =======================================================
 * Callback Functions
 * =======================================================
 */
void handleRoot(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", INDEX_HTML);
}

void handleClick (AsyncWebServerRequest *request) {

    // Display animation
    LED_Message_queue_send(LED_CIRCLE_IN, 0, 40, 40, false);

    if (enableVibration) {
        device_vibrate(STRENGTH_HIGH, 800);
    }
    
    // Engage Switch for 0.5 seconds
    digitalWrite(SOFT_RELAY_PIN, HIGH);
    vTaskDelay(700 / portTICK_PERIOD_MS);
    digitalWrite(SOFT_RELAY_PIN, LOW);

    if(WiFi.status() == WL_CONNECTED){
        if(UseWAPEnterprise){
            httpGetRequest("http://wifiswitch01.bc.edu/control/click");
        }else{
            httpGetRequest("http://wifiswitch01.local/control/click");
        }
    }

    // Display animation
    LED_Message_queue_send(LED_LOAD_OUT, 0, 40, 40, false);

    // Acknowledge with 200 response
    request->send_P(200, "text/plain", "OK");

    // return to normal status indicator
    LED_Message_queue_send(LED_PERSIST_STATUS_2, color_normal_r, color_normal_g, color_normal_b, false);
}

void handleMotor (AsyncWebServerRequest *request) {

    // Display animation
    LED_Message_queue_send(LED_FLASH_FAST, 0, 40, 40, true);

    // Acknowledge with 200 response
    request->send_P(200, "text/plain", "OK");

    // Vibrate motor
    device_vibrate(STRENGTH_HIGH, 1000);

    // return to normal status indicator
    LED_Message_queue_send(LED_PERSIST_STATUS_2, color_normal_r, color_normal_g, color_normal_b, false);
}

void updateEMGreading (AsyncWebServerRequest *request) {
    String reading = String(EMGreading);
    // Respond sensor reading with 200 response
    request->send_P(200, "text/plain", reading.c_str());
}

void updateThresholdValue (AsyncWebServerRequest *request) {
    String thresh = String(EMGthreshold);

    // Respond sensor reading with 200 response
    request->send_P(200, "text/plain", thresh.c_str());
}

void updateVibrationStatus (AsyncWebServerRequest *request) {
    if (enableVibration){
        request->send_P(200, "text/plain", "ON");
    }else{
        request->send_P(200, "text/plain", "OFF");
    }
}

void handleChangeThreshold (AsyncWebServerRequest *request) {
    String paramValue;
    if (request->hasParam("value") ) {
        paramValue = request->getParam("value")->value();
        Serial.println("Get Parameter: "+paramValue);
        // Update and Store value to eeprom
        EMGthreshold = paramValue.toInt();
        EEPROM.put(THRESHOLD_ADDR, EMGthreshold);
        EEPROM.commit();
    }else{
        Serial.println("[ERROR] >>> handleChangeThreshold: no url Param - value");
    }
    request->send_P(200, "text/plain", "OK");
}

void handleChangeVibration (AsyncWebServerRequest *request) {
    String paramState;
    if (request->hasParam("state") ) {
        paramState = request->getParam("state")->value();
        // Update and Store value to eeprom
        enableVibration = paramState.toInt();
        if(enableVibration!=EEPROM.read(VIBRATION_SETTING_ADDR)) {
            EEPROM.write(VIBRATION_SETTING_ADDR, byte(enableVibration));
            EEPROM.commit();
        }

    }else{
        Serial.println("[ERROR] >>> handleChangeVibration: no url Param - state");
    }
    request->send_P(200, "text/plain", "OK");
}

void handleChangeColor (AsyncWebServerRequest *request) {
    if (request->hasParam("red") && request->hasParam("green") && request->hasParam("blue")) {
        color_normal_r = request->getParam("red")->value().toInt();
        color_normal_g = request->getParam("green")->value().toInt();
        color_normal_b = request->getParam("blue")->value().toInt();

        // Turn on normal status indicator with updated color
        LED_Message_queue_send(LED_PERSIST_STATUS_2, color_normal_r, color_normal_g, color_normal_b, false);
    }else{
        Serial.println("[ERROR] >>> handleChangeVibration: no url Param - red, green, or blue");
    }
    request->send_P(200, "text/plain", "OK");
}

/*
 * =======================================================
 * Functions
 * =======================================================
 */


bool connectWifi_WAP2_Personal() {
    /*
    //ESP32 As access point
      WiFi.mode(WIFI_AP); //Access Point mode
      WiFi.softAP(ssid, password);
    */
    WiFi.mode(WIFI_STA);
    //Serial.print("MAC address >> ");
    //Serial.println(WiFi.macAddress());
    WiFi.begin(ssid, password);
    // Make Sure Wifi is connected
    float timeout = 0.0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
        timeout += 0.5;
        if (timeout > 30){ // timeout is set to 30 seconds
            return false;
        }
    }
    Serial.println("Connected to the WiFi network");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);

    return true;
}

bool connectWifi_WAP2_Enterprise() {
    // Set NULL WiFi mode for hostname change
    WiFi.mode(WIFI_MODE_NULL);
    // Set host name
    WiFi.disconnect(true);
    delay(1000);
    //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.setHostname(domainName); // have to do this everytime since new host name won't be remembered
    // Set ESP32 to sation mode
    WiFi.mode(WIFI_MODE_STA);
    // Change mac address if needed
    //esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
    // Configure enterprise network
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    esp_wifi_sta_wpa2_ent_enable();
    // Start connection
    WiFi.begin(WAP2_SSID);
    // Make Sure Wifi is connected
    float timeout = 0.0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
        timeout += 0.5;
        if (timeout > 30){ // timeout is set to 30 seconds
            return false;
        }
    }
    Serial.println("Connected to the WiFi network");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(500);
    Serial.print("Host Name >> ");
    Serial.println(WiFi.getHostname());

    return true;
}

// Function that pushes a led message to the queue
// return false if queue full
// ledmessage.useprevcolor set to false
bool LED_Message_queue_send(int pattern, int red, int green, int blue, bool replay) {
    struct LEDMessage msg;
    msg.pattern = pattern;
    msg.colors[0] = red;
    msg.colors[1] = green;
    msg.colors[2] = blue;
    msg.useprevcolor = false;
    msg.allowreplay = replay;

    if (xQueueSend(xLedQueue,( void * ) &msg,( TickType_t ) 0 ) == pdTRUE){
        // xQueueSend returns pdTRUE upon success
        return true;
    }else{
        // xQueueSend returns errQUEUE_FULL if the queue is full
        return false;
    }   
}

// Function that pushes a led message to the queue
// return false if queue full
// ledmessage.useprevcolor set to true
bool LED_Message_queue_send(int pattern, bool replay) {
    struct LEDMessage msg;
    msg.pattern = pattern;
    msg.useprevcolor = true;
    msg.allowreplay = replay;

    if (xQueueSend(xLedQueue,( void * ) &msg,( TickType_t ) 0 ) == pdTRUE){
        // xQueueSend returns pdTRUE upon success
        return true;
    }else{
        // xQueueSend returns errQUEUE_FULL if the queue is full
        return false;
    }  
}

//
int getEMGreading(){
    double sampledReading = 0;
    // Read from EMG Sensor
    for (int i=0; i < sampleBatchSize; i++){
        sampledReading += analogRead(EMG_SENSOR_PIN);
        vTaskDelay(5 / portTICK_PERIOD_MS); 
    }
    
    EMGreading = int(sampledReading/sampleBatchSize);
    Serial.println(EMGreading);
    return EMGreading;
}

// controls the vibration motor
// specify vibration length in millisecond
// Strength: STRENGTH_SOFT, STRENGTH_MILD, STRENGTH_HIGH
void device_vibrate(int strength, int durationmillis){
    // Writing PWM Values
    ledcWrite(MOTOR_CHANNEL, strength);
    vTaskDelay(durationmillis / portTICK_PERIOD_MS);
    ledcWrite(MOTOR_CHANNEL, 0);
}

// Activate Switch
void activate_Switch() {
    // Display animation
    LED_Message_queue_send(LED_CIRCLE_IN, 0, 40, 40, false);

    if (enableVibration) {
        device_vibrate(STRENGTH_HIGH, 800);
    }
    
    // Engage Switch for 0.5 seconds
        digitalWrite(SOFT_RELAY_PIN, HIGH);
        vTaskDelay(700 / portTICK_PERIOD_MS);
        digitalWrite(SOFT_RELAY_PIN, LOW);

    if(WiFi.status() == WL_CONNECTED){
        if(UseWAPEnterprise){
            httpGetRequest("http://wifiswitch01.bc.edu/control/click");
        }else{
            httpGetRequest("http://wifiswitch01.local/control/click");
        }
    }

    // Display animation
    LED_Message_queue_send(LED_LOAD_OUT, 0, 40, 40, false);


    // return to normal status indicator
    LED_Message_queue_send(LED_PERSIST_STATUS_2, color_normal_r, color_normal_g, color_normal_b, false);
}

void httpGetRequest(const char* serverName){
    WiFiClient client;
    HTTPClient http;

    http.begin(client, serverName);

    int httpResponseCode = http.GET();

    String payload = "--";

    if (httpResponseCode > 0){
        Serial.println("Got http response code: " + httpResponseCode);
        payload = http.getString();
    }else{
        Serial.println("[ERROR] >>> Bad http response code: " + httpResponseCode);
    }

    // Free resources
    http.end();
}

/*
 * =======================================================
 * Setup Function
 * =======================================================
 */
void setup() {
    // Initialize Serial Communication
    Serial.begin(115200);
    Serial.println("\n"); // This is to format output so it does not start on the same line with the gibberish code

    // Setup Device Vibration Motor
    // Configure PWM functionalities: PWMChannel(0-15), Signal Frequency, Duty Cycle resolution
    ledcSetup(MOTOR_CHANNEL, 5000, 8); 
    // Attach the channel to the GPIO to e controlled
    ledcAttachPin(VIBRATION_MOTOR_PIN, MOTOR_CHANNEL); // Pin and channel

    // restore device settings from EEPROM:
    EEPROM.begin(512); // 512 = EEPROM size
    delay(500);
    EEPROM.get(THRESHOLD_ADDR, EMGthreshold);
    enableVibration = int(EEPROM.read(VIBRATION_SETTING_ADDR));
    
    // Create LED Message Queue
    xLedQueue = xQueueCreate(10, sizeof(LEDMessage));
    // Check if Queue was created successfully
    if (xLedQueue == NULL) {
        Serial.print("[ERROR] >>> xLedQueue failed to create");
    }

    // Create LED ring task
    xTaskCreate(
        LED_ring_task,  // Task Function.
        "LED_Ring",     // String name of the Task.
        10000,          // Stack Size in words.
        NULL,           // Parameter passed as input.
        1,              // Task Priority.
        NULL);          // Task Handle.

    // Device start up animation
    LED_Message_queue_send(LED_CIRCLE_IN, 100, 60, 0, false);
    
    //Serial.println("Main loop will now start sleep");
    //for(;;){}
    
    // Software Relay Pin Setup
    pinMode(SOFT_RELAY_PIN, OUTPUT);
    digitalWrite(SOFT_RELAY_PIN, LOW);

    
    // Loading animation
    LED_Message_queue_send(LED_LOADING, 100, 80, 0, true);
    // Connect to Wifi
    bool connectionSuccess;
    if (UseWAPEnterprise) {
        connectionSuccess = connectWifi_WAP2_Enterprise();
    }else{
        connectionSuccess = connectWifi_WAP2_Personal();
    }

    if (connectionSuccess){
        // Setup Local DNS - domain name
        if ( MDNS.begin(domainName) ) {
            Serial.println("mDNS responder started");
        } else {
            Serial.println("Error setting up mDNS responder!");
        }
    
        MDNS.addService("http", "tcp", 80);
    
        // Setup Web server callbacks:
        server.on("/", HTTP_GET, handleRoot);
        server.on("/control/click", handleClick);
        server.on("/control/motor",  handleMotor);
        server.on("/reading/emg",  updateEMGreading);
        server.on("/getsetting/threshold",  updateThresholdValue);
        server.on("/getsetting/vibration",  updateVibrationStatus);
        server.on("/set/vibration",  handleChangeVibration);  
        server.on("/set/threshold", handleChangeThreshold);
        server.on("/set/indicator", handleChangeColor);
         
        
    
        // Begin Server
        server.begin();
        Serial.println("HTTP server started \nOn Domain:\n" + String(domainName) + ".local");
    
        // setup complete animation
        LED_Message_queue_send(LED_LOAD_IN, 0, 80, 0, false);
        LED_Message_queue_send(LED_LOAD_OUT, 0, 80, 0, false);
        // Turn on normal status indicator
        LED_Message_queue_send(LED_PERSIST_STATUS_2, color_normal_r, color_normal_g, color_normal_b, false);
    }else{
        // connection unsuccessful, 
        Serial.println("[ERROR] >> Failed to establish wireless connection");
        // Display error animation
        LED_Message_queue_send(LED_FADEIN, 80, 0, 0, false);
        LED_Message_queue_send(LED_BREATHE, 80, 0, 0, true);
    }


}


/*
 * =======================================================
 * Loop Function
 * =======================================================
 */
void loop() {

    if(getEMGreading() > EMGthreshold){
        activate_Switch();
        while(getEMGreading() > EMGthreshold){
            // blocking
        }
    }
    //Serial.println(EMGreading);
    vTaskDelay(10 / portTICK_PERIOD_MS); // Delay 100 millisecond
}
