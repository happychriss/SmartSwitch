//
// Created by development on 01.07.18.
// Working version uploaded to ESP8266 Development
//



//#define MYDEBUG

#ifdef MYDEBUG
#define DP(x)     Serial.print (x)
    #define DPD(x)     Serial.print (x, DEC)
    #define DPL(x)  Serial.println (x)
#else
#define DP(x)
#define DPD(x)
#define DPL(x)
#endif

#include "esp8266.h"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define ESP8266_LED 5
#define SW1 12
#define SW2 13


const char WiFiSSID[] = "xxxx";
const char WiFiPSK[] = "xxxx";
WiFiClient client1, client2;

IPAddress server(192, 168, 1, 100);

// The ESP8266 RTC memory is arranged into blocks of 4 bytes. The access methods read and write 4 bytes at a time,
// so the RTC data structure should be padded to a 4-byte multiple.
struct {
    uint32_t crc32;   // 4 bytes
    uint8_t channel;  // 1 byte,   5 in total
    uint8_t bssid[6]; // 6 bytes, 11 in total
    uint8_t padding;  // 1 byte,  12 in total
} rtcData;


uint32_t calculateCRC32( const uint8_t *data, size_t length );
// ////////////////////////////// Main Loop, executed after sleep
void setup()
{
    pinMode(SW1, INPUT_PULLUP);
    pinMode(SW2, INPUT_PULLUP);
    pinMode(ESP8266_LED, OUTPUT);

#ifdef MYDEBUG
    Serial.begin(74880);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB
    }
#endif


    digitalWrite(ESP8266_LED, LOW);


    DPL("Start");
    delay(100);
    WiFi.disconnect();
    delay( 30 );
    WiFi.forceSleepBegin(); //this sequence is magically needed
    delay( 50 );
    WiFi.forceSleepWake();
    delay( 50 ); //this value must be long enough, otherwise it will not work

    // Try to read WiFi settings from RTC memory

    bool rtcValid = false;

    if( ESP.rtcUserMemoryRead( 0, (uint32_t*)&rtcData, sizeof( rtcData ) ) ) {
        // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
        uint32_t crc = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
        if( crc == rtcData.crc32 ) {
            rtcValid = true;
            DPL("Valid RTC");
            DPL(crc);

        } else {
            DPL("InValid RTC");
        }
    }


//    WiFi.persistent(false); // Bugfix connectivity to RP
//    WiFi.mode(WIFI_OFF);   // this is a temporary line, to be removed after SDK update to 1.5.4
    WiFi.mode(WIFI_STA);


    if( rtcValid ) {
        DPL("Quick connection");
        // The RTC data was good, make a quick connection
        WiFi.begin( WiFiSSID, WiFiPSK, rtcData.channel, rtcData.bssid, true );
        DPL("Channel");
        DPL(rtcData.channel);

    }
    else {
        DPL("Regular connection");
        // The RTC data was not valid, so make a regular connection
        WiFi.begin( WiFiSSID, WiFiPSK );
    }

    WiFi.config(
            IPAddress(192,168,1,107),
            IPAddress(192,168,1,1),
            IPAddress(255,255,255,0),
            IPAddress(192,168,1,1));



    int retries = 0;
    int wifiStatus = WiFi.status();
    while( wifiStatus != WL_CONNECTED ) {
        retries++;
        DP(".");
        if( retries == 100) {
            DPL("Not working, try regular");
            // Quick connect is not working, reset WiFi and try regular connection
            WiFi.disconnect();
            delay( 10 );
            WiFi.forceSleepBegin();
            delay( 10 );
            WiFi.forceSleepWake();
            delay( 10 );
            WiFi.begin( WiFiSSID, WiFiPSK );
        }
        if( retries == 600 ) {
            DPL("Not working long time, deep sleep");
            // Giving up after 30 seconds and going back to sleep
            WiFi.disconnect( true );
            delay( 1 );
            WiFi.mode( WIFI_OFF );
            ESP.deepSleep( 0, WAKE_RF_DISABLED );
            return; // Not expecting this to be called, the previous call will never return.
        }
        delay( 50 );
        wifiStatus = WiFi.status();
    }

    digitalWrite(ESP8266_LED, HIGH); // Write LED high/low

    // Write current connection info back to RTC

    rtcData.channel = WiFi.channel();
    memcpy( rtcData.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
    rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    ESP.rtcUserMemoryWrite( 0, (uint32_t*)&rtcData, sizeof( rtcData ) );

    DPL("DONE IP Address");
    DPL(WiFi.localIP());

    String PostData;

    IPAddress server(192, 168, 1, 106);

    if (client1.connect(server, 8080)) {

        if (digitalRead(SW1)) {
            PostData = "ON";
            DPL("Read: SW1 - ON");
        } else {
            PostData = "OFF";
            DPL("Read: SW1 - OFF");
        }


        DPL("Connected to Server, send request SW1");
        client1.println("POST /rest/items/SmartSwitch1 HTTP/1.1");
        client1.println("Host: 192.168.1.106");
        client1.println("Content-Type: text/plain");
        client1.println("Accept: application/json");
        client1.println("Connection: close");
        client1.print("Content-Length: ");
        client1.println(PostData.length());
        client1.println();
        client1.println(PostData);
        DPL("Request sent");
        delay(100);

    } else {
        DPL("Connection Error Client1  to Openhabian");
    }


    if (client2.connect(server, 8080)) {

        if (digitalRead(SW2)) {
            PostData = "ON";
            DPL("Read: SW2 - ON");
        } else {
            PostData = "OFF";
            DPL("Read: SW2 - OFF");
        }


        DPL("Connected to Server, send request SW2");
        client2.println("POST /rest/items/SmartSwitch2 HTTP/1.1");
        client2.println("Host: 192.168.1.106");
        client2.println("Content-Type: text/plain");
        client2.println("Accept: application/json");
        client2.println("Connection: close");
        client2.print("Content-Length: ");
        client2.println(PostData.length());
        client2.println();
        client2.println(PostData);
        DPL("Request sent");
        delay(100);

    } else {
        DPL("Connection Error Client2  to Openhabian");
    }



    DPL("End of requests send");


    client1.stop();
    client2.stop();

    ESP.deepSleep( 0, WAKE_RF_DISABLED );

}

void loop()
{

 }


uint32_t calculateCRC32( const uint8_t *data, size_t length ) {
    uint32_t crc = 0xffffffff;
    while( length-- ) {
        uint8_t c = *data++;
        for( uint32_t i = 0x80; i > 0; i >>= 1 ) {
            bool bit = crc & 0x80000000;
            if( c & i ) {
                bit = !bit;
            }

            crc <<= 1;
            if( bit ) {
                crc ^= 0x04c11db7;
            }
        }
    }

    return crc;
}