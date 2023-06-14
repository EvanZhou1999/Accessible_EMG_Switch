// ==================================================================
// Code containing LED ring control tasks and variables for FreeRTOS
// ==================================================================

// Import LED
#include <Adafruit_NeoPixel.h>

// LED pixel light Setup
#define LED_PIN 32
#define NUMPIXELS 7

// LED pattern codes
#define LED_FADEIN 100
#define LED_OFF 200
#define LED_LOADING 300
#define LED_LOADING_LONG 310
#define LED_LOAD_IN 320
#define LED_LOAD_OUT 330
#define LED_BREATHE 400
#define LED_CIRCLE_IN 500
#define LED_FLASH 600
#define LED_FLASH_FAST 610
#define LED_PERSIST_STATUS_1 700
#define LED_PERSIST_STATUS_2 701


// Struct for Queue messaging
struct LEDMessage {
    int pattern;
    int colors[3]; // R, G, B
    bool useprevcolor = false;  // whether the new pattern should use whatever color used by the previous one
    bool allowreplay;    // Is this pattern allowed to be replayed if no message from queue?
    bool playedflag = false;     // Whether this pattern has been played once
};

/*
 * =======================================================
 * Instantiate Global Objects/Devices
 * =======================================================
 */

// Setup LED ring
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_RGB + NEO_KHZ800);

// Create LED queue handler
QueueHandle_t xLedQueue = NULL;


/*
 * =======================================================
 * Functions
 * =======================================================
 */

int* RGBtoHSV(int r, int g, int b) {
    float h, s, v;
    float r_ = r / 255.0f;
    float g_ = g / 255.0f;
    float b_ = b / 255.0f;

    float cmax = std::max(std::max(r_, g_), b_);
    float cmin = std::min(std::min(r_, g_), b_);
    float delta = cmax - cmin;

    if (delta == 0) {
        h = 0;
    } else if (cmax == r_) {
        h = 60 * fmod(((g_ - b_) / delta), 6);
    } else if (cmax == g_) {
        h = 60 * (((b_ - r_) / delta) + 2);
    } else if (cmax == b_) {
        h = 60 * (((r_ - g_) / delta) + 4);
    }

    if (cmax == 0) {
        s = 0;
    } else {
        s = delta / cmax;
    }

    v = cmax;

    int* hsv = new int[3];
    hsv[0] = int(h);
    hsv[1] = int(s);
    hsv[2] = int(v);

    return hsv;
}



void setColor(int red, int green, int blue){
    for(int i=0; i<NUMPIXELS; i++) {
        // (G, R, B)
        pixels.setPixelColor(i, pixels.Color(green, red, blue));
        pixels.show();
        delay(100);
     }
}

void led_fade_in(int red, int green, int blue){

    //int* hsvValue = RGBtoHSV(red, green, blue);
    
    for(int i=0; i<120; i++) {
        //pixels.fill(pixels.ColorHSV(hsvValue[0], hsvValue[1], i));
        pixels.fill(pixels.Color(int(green/100.0*i), int(red/100.0*i), int(blue/100.0*i)));
        pixels.show();
        vTaskDelay(3 / portTICK_PERIOD_MS);
    }
    for(int i=120; i>100; i--) {
        pixels.fill(pixels.Color(int(green/100.0*i), int(red/100.0*i), int(blue/100.0*i)));
        pixels.show();
        vTaskDelay(4 / portTICK_PERIOD_MS);
    }
}

void led_off(){
    pixels.fill(pixels.Color(0, 0, 0));
    pixels.show();
}


void led_loading(int red, int green, int blue){
    led_off();
    // loading trailing lnegth: 1 leds [alow 15 spacing]
    // one trail in front and one trail at the end, 3 active leds
    // led2[100], led3[200], ...... , led7[600]
    float trailing = 150.0;
    for(int t=-50; t<760; t++) {
        for (int i=1; i<NUMPIXELS; i++){
            float brightness = 100 - abs(i*100-t)*100/trailing;
            if (brightness < 0){
                brightness = 0;
            }
            pixels.setPixelColor(i, pixels.Color(int(green/100.0*brightness), int(red/100.0*brightness), int(blue/100.0*brightness)));
        }
        pixels.show();
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void led_loading_long(int red, int green, int blue){
    led_off();
    // loading trailing lnegth: 1 leds [alow 15 spacing]
    // one trail in front and one trail at the end, 3 active leds
    // led2[100], led3[200], ...... , led7[600]
    float trailing = 500.0;
    for(int t=-600; t<1300; t++) {
        for (int i=1; i<NUMPIXELS; i++){
            float brightness = 100 - abs(i*100-t)*100/trailing;
            if (brightness < 0){
                brightness = 0;
            }
            pixels.setPixelColor(i, pixels.Color(int(green/100.0*brightness), int(red/100.0*brightness), int(blue/100.0*brightness)));
        }
        pixels.show();
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void led_load_in(int red, int green, int blue){
    led_off();
    // loading trailing lnegth: 1 leds [alow 15 spacing]
    // one trail in front and one trail at the end, 3 active leds
    // led2[100], led3[200], ...... , led7[600]
    float trailing = 150.0;
    for(int t=-150; t<760; t++) {
        for (int i=0; i<NUMPIXELS; i++){
            float brightness = 100 - abs(i*100-t)*100/trailing;
            if (brightness < 0){
                brightness = 0;
            }
            if (i*100 > t){
                pixels.setPixelColor(i, pixels.Color(int(green/100.0*brightness), int(red/100.0*brightness), int(blue/100.0*brightness)));
            }
        }
        pixels.show();
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

void led_load_out(int red, int green, int blue){
    pixels.fill(pixels.Color(green, red, blue));
    // loading trailing lnegth: 1 leds [alow 15 spacing]
    // one trail in front and one trail at the end, 3 active leds
    // led2[100], led3[200], ...... , led7[600]
    float trailing = 150.0;
    for(int t=-50; t<760; t++) {
        for (int i=0; i<NUMPIXELS; i++){
            float brightness = 100 - abs(i*100-t)*100/trailing;
            if (brightness < 0){
                brightness = 0;
            }
            if (i*100 < t){
                pixels.setPixelColor(i, pixels.Color(int(green/100.0*brightness), int(red/100.0*brightness), int(blue/100.0*brightness)));
            }
        }
        pixels.show();
        vTaskDelay(2 / portTICK_PERIOD_MS);
    }
}

// good idea but might not use this one
void led_breathe_1(int red, int green, int blue){
    float centerintensity = 60;
    float restintensity = 60;
    
    // Decrease brightness (breathe out)
    for(int i=50; i<140; i++) {
        // center intensity raises from 50 - 100
        // rest intensity raises from 90 - 140
        if (i < 100) centerintensity -= 1;
        if (i > 90) restintensity -= 1;
        
        pixels.fill(pixels.Color(int(green/100.0*restintensity), int(red/100.0*restintensity), int(blue/100.0*restintensity)));
        pixels.setPixelColor(0,pixels.Color(int(green/100.0*centerintensity), int(red/100.0*centerintensity), int(blue/100.0*centerintensity)));
        pixels.show();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    // Increase brightness (breathe in)
    for(int i=50; i<140; i++) {
        // center intensity raises from 50 - 100
        // rest intensity raises from 80 - 130
        if (i < 100) centerintensity += 1;
        if (i > 90) restintensity += 1;
        
        pixels.fill(pixels.Color(int(green/100.0*restintensity), int(red/100.0*restintensity), int(blue/100.0*restintensity)));
        pixels.setPixelColor(0,pixels.Color(int(green/100.0*centerintensity), int(red/100.0*centerintensity), int(blue/100.0*centerintensity)));
        pixels.show();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    
}

void led_breathe_2(int red, int green, int blue){
    float centerintensity = 100;
    float restintensity = 100;
    
    // Decrease brightness (breathe out)
    for(int i=0; i<65; i++) {
        // center intensity raises from 50 - 100
        // rest intensity raises from 90 - 140
        centerintensity -= 1;
        if (i > 20) restintensity -= 2;
        
        pixels.fill(pixels.Color(int(green/100.0*restintensity), int(red/100.0*restintensity), int(blue/100.0*restintensity)));
        pixels.setPixelColor(0,pixels.Color(int(green/100.0*centerintensity), int(red/100.0*centerintensity), int(blue/100.0*centerintensity)));
        pixels.show();
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }

    // Increase brightness (breathe in)
    for(int i=0; i<65; i++) {
        centerintensity += 1;
        if (i > 20) restintensity += 2;
        
        pixels.fill(pixels.Color(int(green/100.0*restintensity), int(red/100.0*restintensity), int(blue/100.0*restintensity)));
        pixels.setPixelColor(0,pixels.Color(int(green/100.0*centerintensity), int(red/100.0*centerintensity), int(blue/100.0*centerintensity)));
        pixels.show();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    vTaskDelay(400 / portTICK_PERIOD_MS);
    
}


void led_circle_in(int red, int green, int blue){
    led_off();
    // loading trailing lnegth: 1 leds [alow 15 spacing]
    // one trail in front and one trail at the end, 3 active leds
    // led2[100], led3[200], ...... , led7[600]
    float trailing = 0.0001; // this is to prevent division by zero
    for(int t=0; t<600; t++) {
        for (int i=1; i<NUMPIXELS; i++){
            // dist - shortest distance from led to variable t. from either direction (on a circle)
            float dist = 0.0;
            if (i*100 <= t){
                dist = min( t-(i*100), 600-t+(i*100) );
            }else{
                dist = min( (i*100)-t, 600-(i*100)+t );
            }
            float brightness = 100 - dist*100/trailing;
            if (brightness < 0){
                brightness = 0;
            }
            pixels.setPixelColor(i, pixels.Color(int(green/100.0*brightness), int(red/100.0*brightness), int(blue/100.0*brightness)));
        }
        trailing += 0.5;
        pixels.show();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    for(int t=0; t<600; t++) {
        for (int i=1; i<NUMPIXELS; i++){
            // dist - shortest distance from led to variable t. from either direction (on a circle)
            float dist = 0.0;
            if (i*100 <= t){
                dist = min( t-(i*100), 600-t+(i*100) );
            }else{
                dist = min( (i*100)-t, 600-(i*100)+t );
            }
            float brightness = 100 - dist*100/trailing;
            if (brightness < 0){
                brightness = 0;
            }
            pixels.setPixelColor(i, pixels.Color(int(green/100.0*brightness), int(red/100.0*brightness), int(blue/100.0*brightness)));
        }
        pixels.setPixelColor(0, pixels.Color(int(green*(t/600.0)), int(red*(t/600.0)), int(blue*(t/600.0))));
        trailing += 0.5;
        pixels.show();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}


void led_flash(int red, int green, int blue){
    // Flash Twice per call
    for(int i=0; i<2; i++) {
        pixels.fill(pixels.Color(green, red, blue));
        pixels.show();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        pixels.fill(pixels.Color(0, 0, 0));
        pixels.show();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void led_flash_fast(int red, int green, int blue){
    // Flash Twice per call
    for(int i=0; i<2; i++) {
        pixels.fill(pixels.Color(green, red, blue));
        pixels.show();
        vTaskDelay(200 / portTICK_PERIOD_MS);
        pixels.fill(pixels.Color(0, 0, 0));
        pixels.show();
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void led_persist_status_1(int red, int green, int blue){
    float centerintensity = 0;
    float restintensity = 0;

    for(int i=0; i<100; i++) {
        centerintensity += 1;
        if (i > 90) restintensity += 1;
        
        pixels.fill(pixels.Color(int(green/100.0*restintensity), int(red/100.0*restintensity), int(blue/100.0*restintensity)));
        pixels.setPixelColor(0,pixels.Color(int(green/100.0*centerintensity), int(red/100.0*centerintensity), int(blue/100.0*centerintensity)));
        pixels.show();
        vTaskDelay(8 / portTICK_PERIOD_MS);
    }
}

void led_persist_status_2(int red, int green, int blue){
    led_off();
    for(int i=0; i<100; i++) {
        pixels.setPixelColor(0,pixels.Color(int(green/100.0*i), int(red/100.0*i), int(blue/100.0*i)));
        pixels.show();
        vTaskDelay(8 / portTICK_PERIOD_MS);
    }
}


void playPattern(int pattern, int red, int green, int blue){
    switch(pattern){
        case(LED_FADEIN):           led_fade_in(red, green, blue); break;
        case(LED_OFF):              led_off(); break;
        case(LED_LOADING):          led_loading(red, green, blue); break;
        case(LED_LOADING_LONG):     led_loading_long(red, green, blue); break;
        case(LED_LOAD_IN):          led_load_in(red, green, blue); break;
        case(LED_LOAD_OUT):         led_load_out(red, green, blue); break;
        case(LED_BREATHE):          led_breathe_2(red, green, blue); break;
        case(LED_CIRCLE_IN):        led_circle_in(red, green, blue); break;
        case(LED_FLASH):            led_flash(red, green, blue); break;
        case(LED_FLASH_FAST):       led_flash_fast(red, green, blue); break;
        case(LED_PERSIST_STATUS_1): led_persist_status_1(red, green, blue); break;
        case(LED_PERSIST_STATUS_2): led_persist_status_2(red, green, blue); break;
        default:
            Serial.println("Unrecognized LED Pattern");
    }
}


/*
 * =======================================================
 * LED ring Main Task loop
 * =======================================================
 * Setup Tasks:
 * - Begin RGB Pixel LED
 * - Initialize Previous Color variable
 * - Initialize LEDmessage struct
 * Loop Tasks:
 * - Receive LEDMessage from the Queue
 * - If there is a mesage, play the new pattern with color defined by useprevcolor flag
 *      - Only check and update previous color in this step
 * - If there is no message, play previous pattern if allowreplay flag, with previous color
 */

void LED_ring_task(void * parameter) {
    // Pixel LED Start
    pixels.begin();
    pixels.clear();
    // Variable to store previous color
    int previousColor[3] = {0, 0, 0};
    // Instantiate LED message
    struct LEDMessage ledmessage;
    ledmessage.pattern = LED_OFF;
    ledmessage.colors[0] = 0;
    ledmessage.colors[1] = 0;
    ledmessage.colors[2] = 0;
    ledmessage.allowreplay = false;

    
    // enter loop and processing queue message
    for(;;){
        
        if( xLedQueue != NULL ) {
            if( xQueueReceive( xLedQueue,&( ledmessage ),( TickType_t ) 10 ) == pdPASS ){
                // received new message from the queue
                if(ledmessage.useprevcolor) {
                    // if the message indicates the use of previous color
                    playPattern(ledmessage.pattern, previousColor[0], previousColor[1], previousColor[2]);
                }else{
                    // if the message did not indicate use of previous color
                    // then use the color defined in the message
                    playPattern(ledmessage.pattern, ledmessage.colors[0], ledmessage.colors[1], ledmessage.colors[2]);
                    // and update precious color variable with current one.
                    previousColor[0] = ledmessage.colors[0];
                    previousColor[1] = ledmessage.colors[1];
                    previousColor[2] = ledmessage.colors[2];
                }
            }else {
                // did not receive new message from the queue
                // check if we should replay current one
                if (ledmessage.allowreplay){
                    // then play the pattern with (maybe already updated) previousColor.
                    playPattern(ledmessage.pattern, previousColor[0], previousColor[1], previousColor[2]);
                }
                // if not the led will simply remain current display status.
            }
        }else{
            Serial.println("[ERROR] >>> xLedQueue NULL ");
        }
        
    } // infinite for loop bracket
}













 
