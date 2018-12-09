#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;

ESP8266WebServer http_rest_server(80);

WiFiEventHandler stationConnectedHandler;

IPAddress local_IP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

String macToString(const unsigned char *mac)
{
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected &evt)
{
  Serial.print("Station connected: ");
  Serial.println(macToString(evt.mac));
}

void setup()
{

  Serial.begin(9600);

  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  delay(200);

  ESP.eraseConfig();
  delay(200);

  WiFi.setAutoConnect(false);
  delay(200);

  WiFi.mode(WIFI_AP);
  delay(200);

  Serial.println(WiFi.enableAP(true) ? "AP Enabled" : "AP Enable fail!");
  delay(200);

  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "APConfig Ready" : "Failed!");
  delay(200);

  Serial.println(WiFi.softAP("TEST ESP8266") ? "AP Ready" : "Failed!");
  Serial.println("Wait 200 ms for AP_START...");
  delay(200);

  Serial.println(WiFi.softAPIP());
  delay(200);

  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);

  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);

  // start DNS server for a specific domain name
  dnsServer.start(DNS_PORT, "www.myesptest.io", local_IP);

  //Handle every request
  http_rest_server.onNotFound([]() {
    String message = "Hello, I'm Alive\n\n";
    message += "URI: ";
    message += http_rest_server.uri();

    http_rest_server.send(200, "text/plain", message);
  });

  http_rest_server.begin();
  Serial.println("HTTP REST Server Started");
}

void loop()
{
  dnsServer.processNextRequest();
  http_rest_server.handleClient();
}