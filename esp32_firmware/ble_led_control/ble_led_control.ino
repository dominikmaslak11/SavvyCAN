/**
 * ESP32 BLE LED Control
 * =====================
 *
 * Serwis BLE GATT do sterowania wbudowaną diodą LED (GPIO 2).
 * Działa równolegle z mostkiem CAN (nie koliduje).
 *
 * BLE 4.2 — kompatybilny z Android 5.0+ (API 21+).
 *
 * Serwis:   4fafc201-1fb5-459e-8fcc-c5c9c331914b
 * Char LED: beb5483e-36e1-4688-b7f5-ea07361b26a8  (READ, WRITE)
 *
 * Użycie:
 *   1. Wgraj na ESP32
 *   2. Z Android: znajdź "ESP32_LED" w skanerze BLE
 *   3. Połącz, zapisz "1" na charakterystykę → LED ON
 *                     "0" → LED OFF
 *                     Odczyt → "0" lub "1"
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// ═══════════════════════════════════════════════════════════════════════════
// Konfiguracja
// ═══════════════════════════════════════════════════════════════════════════

#define LED_PIN            2          // Wbudowana dioda (GPIO2 na większości płytek)
#define DEVICE_NAME        "ESP32_LED"
#define SERVICE_UUID       "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ═══════════════════════════════════════════════════════════════════════════
// Stan globalny
// ═══════════════════════════════════════════════════════════════════════════

BLEServer        *pServer        = nullptr;
BLEService       *pService       = nullptr;
BLECharacteristic *pLedChar      = nullptr;
bool              ledState       = false;
bool              deviceConnected = false;

// ═══════════════════════════════════════════════════════════════════════════
// Callback dla zdarzeń serwera BLE
// ═══════════════════════════════════════════════════════════════════════════

class LedServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
        Serial.println("[BLE] Klient podłączony");
    }

    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
        Serial.println("[BLE] Klient rozłączony — restartowanie reklamowania...");
        // Po rozłączeniu zacznij ponownie reklamować
        BLEDevice::startAdvertising();
        Serial.println("[BLE] Reklamowanie wznowione");
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Callback dla zapisu/odczytu charakterystyki LED
// ═══════════════════════════════════════════════════════════════════════════

class LedCharCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        String value = pCharacteristic->getValue();

        Serial.printf("[BLE] Otrzymano zapis: '%s' (długość: %d)\n",
                      value.c_str(), value.length());

        if (value.length() == 0) return;

        // Akceptowane komendy: "1", "0", "ON", "OFF", "on", "off", "TOGGLE"
        char cmd = value.charAt(0);
        bool newState = ledState;

        if (cmd == '1' || cmd == 'O' || cmd == 'o') {
            newState = true;
        } else if (cmd == '0' || cmd == 'F' || cmd == 'f') {
            newState = false;
        } else if (cmd == 'T' || cmd == 't') {
            newState = !ledState;
        } else {
            Serial.printf("[BLE] Nieznana komenda: '%s'\n", value.c_str());
            return;
        }

        ledState = newState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        Serial.printf("[BLE] LED: %s (GPIO %d)\n",
                      ledState ? "ON" : "OFF", LED_PIN);

        // Aktualizuj wartość charakterystyki dla przyszłych odczytów
        pLedChar->setValue(ledState ? "1" : "0");
    }

    void onRead(BLECharacteristic *pCharacteristic) {
        pLedChar->setValue(ledState ? "1" : "0");
        Serial.printf("[BLE] Odczyt stanu LED: %s\n", ledState ? "ON" : "OFF");
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Setup
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println("\n=== ESP32 BLE LED Control ===");
    Serial.printf("Nazwa:    %s\n", DEVICE_NAME);
    Serial.printf("Serwis:   %s\n", SERVICE_UUID);
    Serial.printf("Char:     %s\n", CHARACTERISTIC_UUID);
    Serial.printf("PIN LED:  GPIO %d\n\n", LED_PIN);

    // ── LED ───────────────────────────────────────────────────────────
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    ledState = false;

    // ── BLE ───────────────────────────────────────────────────────────
    BLEDevice::init(DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new LedServerCallbacks());

    // Tworzenie serwisu LED
    pService = pServer->createService(SERVICE_UUID);

    // Tworzenie charakterystyki LED
    pLedChar = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    pLedChar->setCallbacks(new LedCharCallbacks());
    pLedChar->setValue("0");  // Początkowo wyłączona

    // Uruchomienie serwisu
    pService->start();

    // ── Reklamowanie ──────────────────────────────────────────────────
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);   // Minimalny interwał reklamowania
    pAdvertising->setMinPreferred(0x12);   // Maksymalny interwał
    BLEDevice::startAdvertising();

    Serial.println("BLE: Reklamowanie rozpoczęte");
    Serial.println("Gotowy. Czekam na połączenie...\n");
}

// ═══════════════════════════════════════════════════════════════════════════
// Loop
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
    // Nic do roboty — wszystko obsługiwane przez callbacki BLE
    delay(100);
}