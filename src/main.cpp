#include <WiFi.h>
#include <esp_now.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define RADIO_CS_PIN 18
#define RADIO_DIO0_PIN 26
#define RADIO_RST_PIN 23

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

typedef struct struct_message {
    float accelX;
    float accelY;
    float accelZ;
} struct_message;

volatile struct_message incomingData;
volatile bool loraSendDataFlag = false;

void OnDataRecv(const uint8_t *mac, const uint8_t *incoming, int len) {
    if (len == sizeof(struct_message)) {
        memcpy((void*)&incomingData, incoming, sizeof(struct_message));
        loraSendDataFlag = true; // Signal that new data is ready to be sent via LoRa
    }
}

void initDisplay() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.setTextSize(1);              // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.setCursor(0, 0);             // Start at top-left corner
    display.cp437(true);                 // Use full 256 char 'Code Page 437' font
}

void setup() {
    Serial.begin(115200);
    // Initialize LoRa
    
    WiFi.mode(WIFI_STA); // Set device in STA mode to initiate ESP-NOW communication
    Serial.println(WiFi.macAddress());

    initDisplay();
    display.println("Setup done!");
    display.display();

    LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
    if (!LoRa.begin(915E6)) {
        Serial.println("Starting LoRa failed!");
        display.println("LoRa start failed!");
        display.display();
        while (1);
    } else {
        Serial.println("LoRa initialized successfully.");
        display.println("LoRa OK!");
        display.display();
    }

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed!");
        return;
    }
    esp_now_register_recv_cb(OnDataRecv);

    
}

void loop() {
    if (loraSendDataFlag) {
        // Display the received data on OLED
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Received Data:");
        display.print("AX: ");
        display.print(incomingData.accelX);
        display.print(" AY: ");
        display.print(incomingData.accelY);
        display.print(" AZ: ");
        display.println(incomingData.accelZ);
        display.display();

        // Log received data to Serial
        Serial.println("Received Data:");
        Serial.print("AX: ");
        Serial.print(incomingData.accelX);
        Serial.print(", AY: ");
        Serial.print(incomingData.accelY);
        Serial.print(", AZ: ");
        Serial.println(incomingData.accelZ);

        // Send the data via LoRa
        Serial.println("Sending data via LoRa");
        LoRa.beginPacket();
        LoRa.write((uint8_t*)&incomingData, sizeof(struct_message));
        int result = LoRa.endPacket();
        Serial.print("Send result: ");
        Serial.println(result ? "SUCCESS" : "FAILURE");

        if (result) {
            Serial.println("Data sent via LoRa");
        } else {
            Serial.println("LoRa send failed");
        }
        
        loraSendDataFlag = false;  // Reset the flag after handling
    }

    delay(100); // Just a small delay to keep loop under control
}
