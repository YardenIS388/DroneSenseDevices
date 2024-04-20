#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

// MPU6050 sensor instance
Adafruit_MPU6050 mpu;

// Structure to send data
typedef struct struct_message
{
  float accelX;
  float accelY;
  float accelZ;
} struct_message;

struct_message myData;
// Register peer
uint8_t peerAddress[] = {0x08, 0xF9, 0xE0, 0xD0, 0x20, 0x9C};

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus)
{
  Serial.print("Send Status: ");
  if (sendStatus == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Delivery success");
  }
  else
  {
    Serial.println("Delivery fail");
  }
}

void setup()
{
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set device in STA mode to initiate ESP-NOW communication
  Serial.println(WiFi.macAddress());

  // Initialize MPU6050
  if (!mpu.begin())
  {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(1000);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  Serial.println("MPU6050 initialized!");

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback for sending data
  esp_now_register_send_cb(OnDataSent);

  // Replace with actual MAC Address of the T-Beam
  esp_now_peer_info_t peerInfo = {};
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, peerAddress, 6);
  peerInfo.channel = 0; // use the current channel
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Print accelerometer values
  Serial.print("Accelerometer: X=");
  Serial.print(a.acceleration.x);
  Serial.print(" Y=");
  Serial.print(a.acceleration.y);
  Serial.print(" Z=");
  Serial.println(a.acceleration.z);

  // Assign data to the structure
  myData.accelX = a.acceleration.x;
  myData.accelY = a.acceleration.y;
  myData.accelZ = a.acceleration.z;

  // Send data
  esp_now_send(peerAddress, (uint8_t *)&myData, sizeof(struct_message)); // Send to specific peer
  delay(20000);                                                           // Send every 2 seconds
}
