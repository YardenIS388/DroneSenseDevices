#include <RH_RF95.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RADIO_CS_PIN 18
#define RADIO_RST_PIN 23
#define RADIO_DIO0_PIN 26
#define BAND 868.0 // LoRa Frequency in MHz

int counter = 0;

RH_RF95 rf95(RADIO_CS_PIN, RADIO_DIO0_PIN);

typedef struct struct_message
{
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

  // Initialize RH_RF95 LoRa radio
  if (!rf95.init())
  {
    display.println("LoRa init failed!");
    display.display();
    while (1)
      ;
  }
  if (!rf95.setFrequency(BAND))
  {
    display.println("Frequency set failed!");
    display.display();
    while (1)
      ;
  }
  display.println("LoRa Initializing OK!");
  display.display();
}

void loop()
{

  // Periodically print the RSSI when no data is available
  if (!rf95.available())
  {
    int rssi = rf95.lastRssi();
    Serial.print("No data available. RSSI: ");
    Serial.println(rssi);
  }
  if (rf95.available())
  {
    struct_message receivedData;
    uint8_t buf[sizeof(struct_message)];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {
      if (len == sizeof(struct_message))
      {
        memcpy(&receivedData, buf, sizeof(struct_message));
        counter++;
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Received Data: Transmission #" + String(counter));
        display.printf("AX: %.2f, AY: %.2f, AZ: %.2f\n", receivedData.accelX, receivedData.accelY, receivedData.accelZ);
        display.print("RSSI: ");
        display.println(rf95.lastRssi()); // Get the RSSI value from last received packet
        display.display();

        Serial.println("Packet received!");
        Serial.printf("AX: %.2f, AY: %.2f, AZ: %.2f\n", receivedData.accelX, receivedData.accelY, receivedData.accelZ);
        Serial.print("RSSI: ");
        Serial.println(rf95.lastRssi());
      }
      else
      {
        Serial.println("Received packet of unexpected size.");
      }
    }
  }
  else
  {
    Serial.println("No packet received.");
  }
  delay(1000);
}
