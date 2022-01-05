#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char *ssid = "HUAWEI nova 5T";
const char *pass = "leppangang1";

const char *mqttBroker = "test.mosquitto.org";
const int port = 1883;
const char *topic = "wsn-tempat-sampah";

WiFiClient client;
PubSubClient mqtt(client);

typedef struct pesan
{
  int node;
  float tinggi;
  float berat;
} pesan;

pesan pesanMasuk;

void connectWifi();
void onRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void connectMqtt();
void sendMqtt();

void setup()
{
  Serial.begin(9600);

  connectWifi();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Terjadi masalah inisialisasi ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(onRecv);

  mqtt.setServer(mqttBroker, port);
}

void loop()
{
  if (!mqtt.connected())
  {
    connectMqtt();
  }

  mqtt.loop();
}

void connectWifi()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pass);

  Serial.print("Menghubungkan ke Wifi ..");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("Berhasil terhubung ke Wifi");
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
}

void onRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&pesanMasuk, incomingData, sizeof(pesanMasuk));

  Serial.print("Node: ");
  Serial.println(pesanMasuk.node);
  Serial.print("Tinggi: ");
  Serial.print(pesanMasuk.tinggi);
  Serial.println(" cm");
  Serial.print("Berat: ");
  Serial.print(pesanMasuk.berat);
  Serial.println(" kg");
  Serial.println("-------------------------------------");

  sendMqtt();
}

void connectMqtt()
{
  Serial.print("Menghubungkan ke MQTT ..");
  while (!mqtt.connected())
  {
    Serial.print(".");

    if (mqtt.connect("wsn-gateway"))
    {
      mqtt.subscribe(topic);
    }
  }
  Serial.println();
  Serial.println("Berhasil terhubung ke MQTT");
}

void sendMqtt()
{
  DynamicJsonDocument doc(256);
  char buffer[256];

  doc["node"] = pesanMasuk.node;
  doc["tinggi"] = pesanMasuk.tinggi;
  doc["berat"] = pesanMasuk.berat;

  size_t n = serializeJson(doc, buffer);

  mqtt.publish(topic, buffer, n);
}