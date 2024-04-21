#include <Wire.h>
#include <RH_RF95.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3C for most screens

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RH_RF95 rf95;

void setup() {
    Serial.begin(9600);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("LoRa Sender Init");
    display.display();
    
    if (!rf95.init()) {
        display.println("LoRa init failed");
        display.display();
        while (1);
    }
    if (!rf95.setFrequency(915.0)) {
        display.println("Set freq failed");
        display.display();
        while (1);
    }
    rf95.setTxPower(18, true);
    display.println("LoRa Ready!");
    display.display();
}

void loop() {
    const char *msg = "Hello, LoRa!";
    rf95.send((uint8_t *)msg, strlen(msg));
    rf95.waitPacketSent();

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Sending message:");
    display.println(msg);
    display.display();

    Serial.println("Message sent");
    delay(1000);
}
