#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BAND 868E6 // LoRa Frequency, set to your region's frequency
#define RADIO_CS_PIN 18
#define RADIO_DIO0_PIN 26
#define RADIO_RST_PIN 23

typedef struct struct_message {
  float accelX;
  float accelY;
  float accelZ;
} struct_message;

void setup()
{
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Initialize the OLED display
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("LoRa Receiver");
  display.display();

  // Initialize LoRa using the frequency defined above
  LoRa.setPins(RADIO_CS_PIN, RADIO_RST_PIN, RADIO_DIO0_PIN);
  if (!LoRa.begin(BAND))
  {
    display.println("Starting LoRa failed!");
    display.display();
    while (1)
      ;
  }
  display.println("LoRa Initializing OK!");
  display.display();
}

void loop()
{
      int packetSize = LoRa.parsePacket();
      Serial.println(packetSize);
    if (packetSize) {
        Serial.println("Packet received!");

        // Check if the packet size matches the expected size of struct_message
        if (packetSize == sizeof(struct_message)) {
            struct_message receivedData;
            // Read the packet into the struct
            uint8_t *dataPtr = (uint8_t *)&receivedData;
            for (int i = 0; i < sizeof(struct_message); i++) {
                dataPtr[i] = LoRa.read();
            }

            // Print and display the received data
            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Received Data:");
            display.printf("AX: %.2f, AY: %.2f, AZ: %.2f\n", receivedData.accelX, receivedData.accelY, receivedData.accelZ);
            display.print("RSSI: ");
            display.println(LoRa.packetRssi());
            display.display();
        } else {
            Serial.println("Received packet of unexpected size.");
        }
    } else {
        Serial.println("No packet received.");
    }
}
