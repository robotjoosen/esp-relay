#include <Regexp.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define RELAY_PORT 0

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);

const char* SSID = "your-ssid";
const char* PASSWORD = "your-password";
const String device_name = "ESP8266-Relay";

void handleRoot() {
    int d = 10;
    float amount = 0;

    String t;
    for ( uint8_t i = 0; i < server.args(); i++ ) {

        if(server.argName(i) == "text") {
            t = server.arg(i);
        }
    }

    char result[16];
    MatchState ms;
    ms.Target((char*)t.c_str());
    if(ms.Match("(%d+,%d+)")) {
        ms.GetMatch(result);
        String normalized = result;
        normalized.replace(",",".");
        amount = normalized.toFloat();
        if(amount < 1) {
            //
        } else if(amount < 5) {
            d = 50;
        } else if(amount < 10) {
            d = 100;
        } else if(amount < 20) {
            d = 250;
        } else if(amount < 50) {
            d = 500;
        } else {
            d = 1500;
        }
    }

    char msg[400];
    snprintf(msg, 400, "{\"success\" : 1, \"message\" : \"Index\", \"delay\": \"%02d\", \"amount\": \"%02f\"}", d, amount);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", msg);


    if (amount > 9.101 && amount < 9.111) {
        Serial.println("911 triggered");
        digitalWrite(RELAY_PORT, LOW);
        delay(500);
        digitalWrite(RELAY_PORT, HIGH);
        delay(2000);
        digitalWrite(RELAY_PORT, LOW);
        delay(500);
        digitalWrite(RELAY_PORT, HIGH);

    } else {
        Serial.println("Delay trigger warning!!!");
        digitalWrite(RELAY_PORT, LOW);
        delay(d);
        digitalWrite(RELAY_PORT, HIGH);
    }
    
}

void handleNotFound() {
    digitalWrite(LED_BUILTIN, HIGH);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(404, "text/plain", "{\"success\" : 0, \"message\" : \"Page not found\"}");
    digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW); 

    pinMode(RELAY_PORT, OUTPUT);
    digitalWrite(RELAY_PORT, HIGH); 

    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(SSID, PASSWORD);

    if (MDNS.begin("esp8266-relay-1")) {
        Serial.println("MDNS responder started");
    }

    Serial.print("connecting");
    while (WiFiMulti.run() != WL_CONNECTED ) {
		delay(500);
		Serial.print ( "." );
	}
    Serial.println("");

    if ( MDNS.begin (device_name) ) {
		Serial.println("MDNS responder started");
	}

    Serial.println("");
	Serial.print("Connected to ");
	Serial.println (SSID);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();
}

void loop() {
    server.handleClient();
}
