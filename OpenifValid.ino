#include <WiFi.h>
#include <WebSocketsServer.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Firebase_ESP_Client.h>
#include <ESP32Servo.h>

// 🔥 Firebase Credentials
#define FIREBASE_PROJECT_ID "rit24safaai"
#define FIREBASE_API_KEY "AIzaSyCtKl0jmLRZe3FVzPumMsEdvFB61sqBSDM"
#define USER_EMAIL "nakul977822@gmail.com"
#define USER_PASSWORD "Nakul@2002"

// WiFi Credentials
const char* ssid = "AK";
const char* password = "12345678";

// NFC Module Setup
#define SS_PIN  21  
#define RST_PIN 22  
MFRC522 mfrc522(SS_PIN, RST_PIN);

// WebSocket Setup
WebSocketsServer webSocket(81);

// Firebase Setup
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Servo Setup
Servo myServo;
#define SERVO_PIN 15  // Connect the servo signal wire to pin 15

MFRC522::MIFARE_Key key;

// Timer for Firestore checks
unsigned long lastFirestoreCheck = 0;
const unsigned long firestoreCheckInterval = 5000;  // Check Firestore every 5 seconds

void setup() {
    Serial.begin(9600);
    WiFi.begin(ssid, password);
    SPI.begin();
    mfrc522.PCD_Init();

    // ✅ Set the authentication key
    byte customKey[6] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7};
    memcpy(key.keyByte, customKey, 6);

    // Wait for WiFi connection
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected");

    // Print ESP32 IP Address
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    // Initialize Firebase
    config.api_key = FIREBASE_API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    updateFirestoreIP();

    // Attach servo to pin
    myServo.attach(SERVO_PIN);

    // Initialize WebSocket
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    Serial.println("Setup Complete");
}

void loop() {
    webSocket.loop();
    checkNFC();

    // Check Firestore frequently
    if (millis() - lastFirestoreCheck >= firestoreCheckInterval) {
        lastFirestoreCheck = millis();
        checkFirestoreFlag();
    }
}

// Function to check and read NFC data
void checkNFC() {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        Serial.println("NFC Card Detected!");

        String email = readNFCData(5) + readNFCData(6);  // Read email from Block 1 & 2
        String password = readNFCData(9) + readNFCData(10);  // Read password from Block 5 & 6

        email.trim(); // Remove unwanted spaces or null chars
        password.trim();

        if (email.length() > 0 && password.length() > 0) {
            String json = "{\"email\":\"" + email + "\", \"password\":\"" + password + "\"}";
            Serial.println("Sending NFC Data: " + json);
            webSocket.broadcastTXT(json);  // Send data to app
        } else {
            Serial.println("Error: Could not read email/password from NFC card!");
        }

        // Halt PICC to avoid repeated reading
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
    }
}

// Function to authenticate and read stored data from NFC card
String readNFCData(byte block) {
    byte buffer[18];
    byte bufferSize = sizeof(buffer);

    // Authenticate sector before reading
    byte sector = block / 4 * 4;
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
        MFRC522::PICC_CMD_MF_AUTH_KEY_A, sector, &key, &(mfrc522.uid)
    );

    if (status != MFRC522::STATUS_OK) {
        Serial.print("Authentication failed for block ");
        Serial.print(block);
        Serial.print(": ");
        Serial.println(mfrc522.GetStatusCodeName(status));
        return "";
    }

    // Read data
    status = mfrc522.MIFARE_Read(block, buffer, &bufferSize);
    if (status != MFRC522::STATUS_OK) {
        Serial.println("Failed to read NFC data");
        return "";
    }

    // Convert bytes to string
    String data = "";
    for (int i = 0; i < 16; i++) {
        if (buffer[i] == 0x00) break;
        data += (char)buffer[i];
    }

    return data;
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    if (type == WStype_CONNECTED) {
        Serial.println("Client Connected");
    }
}

// Check Firestore Flag
void checkFirestoreFlag() {
    String documentPath = "bin/bin1";
    if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str())) {
        Serial.println("Firestore Data Retrieved");
        processFirestoreData(fbdo.payload().c_str());
    } else {
        Serial.println("Failed to retrieve Firestore data");
        Serial.println(fbdo.errorReason());
    }
}

// Process Firestore Data
void processFirestoreData(const char* json) {
    FirebaseJson payload;
    payload.setJsonData(json);
    FirebaseJsonData flagData;
    payload.get(flagData, "fields/flag/integerValue");

    if (flagData.success) {
        int flag = flagData.intValue;
        Serial.print("Flag Value: ");
        Serial.println(flag);

        if (flag == 1) {
            myServo.write(180);  // Turn servo to 180 degrees
        } else if (flag == 0) {
            myServo.write(0);    // Turn servo back to 0 degrees
        }
    } else {
        Serial.println("Failed to parse flag value");
    }
}
void updateFirestoreIP() {
    String documentPath = "bin/bin1";
    String espIP = WiFi.localIP().toString();  // Get ESP32's IP address

    FirebaseJson content;
    content.set("fields/ip/stringValue", espIP);  // Update "ip" field with the ESP32's IP

    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), content.raw(), "ip")) {
        Serial.println("IP address updated in Firestore: " + espIP);
    } else {
        Serial.println("Failed to update Firestore IP: " + fbdo.errorReason());
    }
}
