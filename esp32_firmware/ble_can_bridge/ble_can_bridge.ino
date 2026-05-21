/**
 * ESP32 BLE CAN Bridge
 * ====================
 *
 * Mostkuje magistralę CAN na BLE GATT.
 * Działa z MCP2515, kwarcem 8MHz, prędkością 250 kbps.
 *
 * Serwisy BLE:
 *   CAN:  4fafc202-1fb5-459e-8fcc-c5c9c331914b
 *     RX: beb5483f-...  (NOTIFY)  ESP32 → tablet
 *     TX: beb54840-...  (WRITE)   tablet → ESP32
 *   LED:  4fafc201-... (jak wcześniej)
 *
 * Binarny format ramki CAN (14 bajtów):
 *   [0-3] ID (32-bit, little-endian)
 *   [4]   DLC (0-8)
 *   [5]   Flags (bit0=extended)
 *   [6-13] Data (DLC bajtów)
 *
 * Pinout MCP2515 → ESP32:
 *   CS=5  INT=4  SO=19  SI=23  SCK=18
 */

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEDescriptor.h>
#include <SPI.h>

// ═══════════════════════════════════════════════════════════════════════════
// Konfiguracja
// ═══════════════════════════════════════════════════════════════════════════

#define DEVICE_NAME         "CANBridge"
#define CAN_SPEED           250E3
#define CS_PIN              5
#define INT_PIN             4
#define LED_PIN             2

// UUID dla serwisu CAN
#define CAN_SERVICE_UUID    "4fafc202-1fb5-459e-8fcc-c5c9c331914b"
#define CAN_RX_CHAR_UUID    "beb5483f-36e1-4688-b7f5-ea07361b26a8"  // ESP32 → tablet (NOTIFY)
#define CAN_TX_CHAR_UUID    "beb54840-36e1-4688-b7f5-ea07361b26a8"  // tablet → ESP32 (WRITE)

// UUID dla LED (opcjonalne)
#define LED_SERVICE_UUID    "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LED_CHAR_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ═══════════════════════════════════════════════════════════════════════════
// MCP2515 — komendy SPI i rejestry
// ═══════════════════════════════════════════════════════════════════════════

#define MCP_RESET       0xC0
#define MCP_READ        0x03
#define MCP_WRITE       0x02
#define MCP_READ_STATUS 0xA0
#define MCP_BIT_MODIFY  0x05

#define REG_CANSTAT   0x0E
#define REG_CANCTRL   0x0F
#define REG_CNF3      0x28
#define REG_CNF2      0x29
#define REG_CNF1      0x2A
#define REG_CANINTE   0x2B
#define REG_CANINTF   0x2C
#define REG_EFLG      0x2D
#define REG_TEC       0x1C
#define REG_REC       0x1D

#define REG_TXB0CTRL  0x30
#define REG_TXB0SIDH  0x31
#define REG_TXB0SIDL  0x32
#define REG_TXB0DLC   0x35
#define REG_TXB0D0    0x36

#define REG_RXB0CTRL  0x60
#define REG_RXB0SIDH  0x61
#define REG_RXB0SIDL  0x62
#define REG_RXB0DLC   0x65
#define REG_RXB0D0    0x66

#define REG_RXB1CTRL  0x70
#define REG_RXB1SIDH  0x71
#define REG_RXB1SIDL  0x72
#define REG_RXB1DLC   0x75
#define REG_RXB1D0    0x76

// ═══════════════════════════════════════════════════════════════════════════
// Ramka CAN
// ═══════════════════════════════════════════════════════════════════════════

struct CANFrame {
    uint32_t id;
    bool     extended;
    uint8_t  length;   // DLC 0-8
    uint8_t  data[8];
    uint32_t timestamp;
};

// ═══════════════════════════════════════════════════════════════════════════
// Ring-buffery
// ═══════════════════════════════════════════════════════════════════════════

template<typename T, int SIZE>
class RingBuffer {
    volatile int head = 0;
    volatile int tail = 0;
    T buffer[SIZE];
public:
    bool push(const T& item) {
        int next = (head + 1) % SIZE;
        if (next == tail) return false;
        buffer[head] = item;
        head = next;
        return true;
    }
    bool pop(T& item) {
        if (tail == head) return false;
        item = buffer[tail];
        tail = (tail + 1) % SIZE;
        return true;
    }
    int available() const { return (head - tail + SIZE) % SIZE; }
    bool isEmpty() const { return head == tail; }
    void clear() { head = tail = 0; }
};

RingBuffer<CANFrame, 128> canRxBuf;   // CAN → BLE (do tabletu)
RingBuffer<CANFrame, 64>  bleTxBuf;   // BLE → CAN (z tabletu)

// ═══════════════════════════════════════════════════════════════════════════
// MCP2515 — niskopoziomowe SPI
// ═══════════════════════════════════════════════════════════════════════════

uint8_t mcp_read(uint8_t reg) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_READ);
    SPI.transfer(reg);
    uint8_t r = SPI.transfer(0x00);
    digitalWrite(CS_PIN, HIGH);
    return r;
}

void mcp_write(uint8_t reg, uint8_t val) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_WRITE);
    SPI.transfer(reg);
    SPI.transfer(val);
    digitalWrite(CS_PIN, HIGH);
}

void mcp_bitModify(uint8_t reg, uint8_t mask, uint8_t data) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_BIT_MODIFY);
    SPI.transfer(reg);
    SPI.transfer(mask);
    SPI.transfer(data);
    digitalWrite(CS_PIN, HIGH);
}

uint8_t mcp_readStatus() {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_READ_STATUS);
    uint8_t r = SPI.transfer(0x00);
    digitalWrite(CS_PIN, HIGH);
    return r;
}

void mcp_reset() {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_RESET);
    digitalWrite(CS_PIN, HIGH);
    delay(10);
}

// ═══════════════════════════════════════════════════════════════════════════
// MCP2515 — inicjalizacja (250 kbps @ 8 MHz)
// ═══════════════════════════════════════════════════════════════════════════

bool mcp2515_init() {
    SPI.begin(18, 19, 23, CS_PIN);
    SPI.setFrequency(8000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(INT_PIN, INPUT_PULLUP);

    mcp_reset();
    delay(20);

    uint8_t canstat = mcp_read(REG_CANSTAT);
    if ((canstat & 0xE0) != 0x80) {
        Serial.printf("MCP2515 CANSTAT=0x%02X (expected 0x80)\n", canstat);
        return false;
    }

    // 250kbps @ 8MHz: BRP=1 → TQ=500ns, bit=4000ns=8TQ
    // Sync=1 Prop=1 PS1=4 PS2=2 SJW=1
    mcp_write(REG_CNF1, 0x01);   // SJW=1, BRP=1
    mcp_write(REG_CNF2, 0x98);   // BTLMODE=1, PHSEG1=3, PRSEG=0
    mcp_write(REG_CNF3, 0x01);   // PHSEG2=1

    mcp_write(REG_CANINTE, 0x03);  // RX0IE + RX1IE

    // Odbieraj wszystko
    mcp_write(REG_RXB0CTRL, 0x60);
    mcp_write(REG_RXB1CTRL, 0x60);

    // Normal mode
    mcp_bitModify(REG_CANCTRL, 0xE0, 0x00);
    delay(10);

    canstat = mcp_read(REG_CANSTAT);
    if (((canstat >> 5) & 0x07) != 0) {
        Serial.printf("MCP2515 mode=%d (expected 0)\n", (canstat >> 5) & 0x07);
        return false;
    }

    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// MCP2515 — odbiór ramki CAN
// ═══════════════════════════════════════════════════════════════════════════

bool mcp2515_readFrame(CANFrame &frame) {
    uint8_t status = mcp_readStatus();
    uint8_t rxBase;

    if (status & 0x01) {
        rxBase = REG_RXB0SIDH;
    } else if (status & 0x02) {
        rxBase = REG_RXB1SIDH;
    } else {
        return false;
    }

    uint8_t sidh = mcp_read(rxBase);
    uint8_t sidl = mcp_read(rxBase + 1);
    frame.extended = (sidl & 0x08) != 0;

    if (frame.extended) {
        uint8_t eid8 = mcp_read(rxBase + 2);
        uint8_t eid0 = mcp_read(rxBase + 3);
        frame.id = ((uint32_t)sidh << 3) | (sidl >> 5);
        frame.id = (frame.id << 18) | ((uint32_t)(sidl & 0x03) << 16) | ((uint32_t)eid8 << 8) | eid0;
    } else {
        frame.id = ((uint32_t)sidh << 3) | (sidl >> 5);
    }

    uint8_t dlc = mcp_read(rxBase + 4) & 0x0F;
    frame.length = (dlc > 8) ? 8 : dlc;

    for (int i = 0; i < frame.length; i++) {
        frame.data[i] = mcp_read(rxBase + 5 + i);
    }

    frame.timestamp = micros();

    if (status & 0x01) mcp_bitModify(REG_CANINTF, 0x01, 0x00);
    if (status & 0x02) mcp_bitModify(REG_CANINTF, 0x02, 0x00);

    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// MCP2515 — wysyłanie ramki CAN
// ═══════════════════════════════════════════════════════════════════════════

bool mcp2515_sendFrame(const CANFrame &frame) {
    uint8_t status = mcp_readStatus();

    uint8_t txBase;
    if (!(status & 0x04)) {
        txBase = REG_TXB0SIDH;
    } else if (!(status & 0x10)) {
        txBase = REG_TXB0SIDH + 16;
    } else if (!(status & 0x40)) {
        txBase = REG_TXB0SIDH + 32;
    } else {
        return false;
    }

    if (frame.extended) {
        // Extended 29-bit ID: SID10:SID3 | SID2:SID0/EXIDE/EID17:EID16 | EID15:EID8 | EID7:EID0
        mcp_write(txBase,     (frame.id >> 21) & 0xFF);
        mcp_write(txBase + 1, ((frame.id >> 18) & 0x07) << 5 | 0x08 | ((frame.id >> 16) & 0x03));
        mcp_write(txBase + 2, (frame.id >> 8) & 0xFF);
        mcp_write(txBase + 3, frame.id & 0xFF);
    } else {
        // Standard 11-bit ID: SID10:SID3 | SID2:SID0
        mcp_write(txBase,     (frame.id >> 3) & 0xFF);
        mcp_write(txBase + 1, (frame.id & 0x07) << 5);
    }
    mcp_write(txBase + 4, frame.length);
    for (int i = 0; i < frame.length; i++) {
        mcp_write(txBase + 5 + i, frame.data[i]);
    }
    mcp_write(txBase - 1, 0x0B);  // TXBxCTRL
    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
// Serializacja ramki CAN do formatu binarnego (14 bajtów)
// ═══════════════════════════════════════════════════════════════════════════

void frameToBytes(const CANFrame &frame, uint8_t *buf) {
    // ID (little-endian)
    buf[0] = frame.id & 0xFF;
    buf[1] = (frame.id >> 8) & 0xFF;
    buf[2] = (frame.id >> 16) & 0xFF;
    buf[3] = (frame.id >> 24) & 0xFF;
    // DLC
    buf[4] = frame.length;
    // Flags: bit0 = extended
    buf[5] = frame.extended ? 0x01 : 0x00;
    // Data
    memcpy(buf + 6, frame.data, frame.length);
}

void bytesToFrame(const uint8_t *buf, int len, CANFrame &frame) {
    if (len < 6) return;

    frame.id = buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    frame.length = buf[4];
    if (frame.length > 8) frame.length = 8;
    frame.extended = (buf[5] & 0x01) != 0;

    int dataLen = len - 6;
    if (dataLen > frame.length) dataLen = frame.length;
    for (int i = 0; i < dataLen; i++) {
        frame.data[i] = buf[6 + i];
    }
    frame.timestamp = micros();
}

// ═══════════════════════════════════════════════════════════════════════════
// BLE — globalne obiekty
// ═══════════════════════════════════════════════════════════════════════════

BLEServer          *pServer      = nullptr;
BLEService         *pCanService  = nullptr;
BLECharacteristic  *pCanRxChar   = nullptr;  // NOTIFY: ESP32 → tablet
BLECharacteristic  *pCanTxChar   = nullptr;  // WRITE:  tablet → ESP32
bool                deviceConnected = false;
bool                ledOn = false;

// ═══════════════════════════════════════════════════════════════════════════
// BLE Callbacki
// ═══════════════════════════════════════════════════════════════════════════

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("[BLE] Połączony");
    }
    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
        digitalWrite(LED_PIN, LOW);
        Serial.println("[BLE] Rozłączony — restart reklamowania...");
        BLEDevice::startAdvertising();
    }
};

class CanTxCharCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pChar) {
        String val = pChar->getValue();
        if (val.length() < 6) return;

        CANFrame frame;
        bytesToFrame((const uint8_t*)val.c_str(), val.length(), frame);

        if (!bleTxBuf.push(frame)) {
            Serial.println("[CAN] BUFOR TX PEŁNY — odrzucono ramkę");
        }
    }
};

// ═══════════════════════════════════════════════════════════════════════════
// Setup
// ═══════════════════════════════════════════════════════════════════════════

unsigned long lastStatus = 0;
unsigned long canRxCount = 0;
unsigned long canTxCount = 0;
unsigned long bleRxDrops = 0;

void setup() {
    Serial.begin(115200);
    delay(300);

    Serial.println("\n=== ESP32 BLE CAN Bridge ===");
    Serial.printf("CAN: %d kbps @ 8MHz\n", (int)(CAN_SPEED / 1000));
    Serial.printf("BLE: %s\n", DEVICE_NAME);

    // ── LED ───────────────────────────────────────────────────────────
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // ── MCP2515 ───────────────────────────────────────────────────────
    Serial.print("MCP2515... ");
    if (!mcp2515_init()) {
        Serial.println("BŁĄD! Sprawdź połączenia.");
        while (1) {
            digitalWrite(LED_PIN, millis() % 500 < 250 ? HIGH : LOW);
            delay(100);
        }
    }
    Serial.println("OK (Normal mode, 250kbps)");

    // ── BLE ───────────────────────────────────────────────────────────
    Serial.print("BLE... ");
    BLEDevice::init(DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    // Serwis CAN
    pCanService = pServer->createService(CAN_SERVICE_UUID);

    // Charakterystyka RX (NOTIFY: ESP32 → tablet)
    pCanRxChar = pCanService->createCharacteristic(
        CAN_RX_CHAR_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    // CCCD descriptor — wymagany przez Android do notyfikacji
    BLEDescriptor *cccd = new BLEDescriptor("2902", 2);
    uint8_t init[2] = {0, 0};
    cccd->setValue(init, 2);
    pCanRxChar->addDescriptor(cccd);

    // Charakterystyka TX (WRITE: tablet → ESP32)
    pCanTxChar = pCanService->createCharacteristic(
        CAN_TX_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCanTxChar->setCallbacks(new CanTxCharCallbacks());

    pCanService->start();

    // Reklamowanie
    BLEAdvertising *pAdv = BLEDevice::getAdvertising();
    pAdv->addServiceUUID(CAN_SERVICE_UUID);
    pAdv->setScanResponse(true);
    BLEDevice::startAdvertising();

    Serial.println("OK");
    Serial.printf("  CAN service: %s\n", CAN_SERVICE_UUID);
    Serial.printf("  CAN RX char: %s (NOTIFY)\n", CAN_RX_CHAR_UUID);
    Serial.printf("  CAN TX char: %s (WRITE)\n", CAN_TX_CHAR_UUID);
    Serial.println("\nGotowy.\n");

    lastStatus = millis();
}

// ═══════════════════════════════════════════════════════════════════════════
// Loop
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
    // ── 1. Odbiór CAN → bufor Rx ──────────────────────────────────────
    CANFrame frame;
    while (mcp2515_readFrame(frame)) {
        canRxCount++;
        if (!canRxBuf.push(frame)) {
            bleRxDrops++;
        }
    }

    // ── 2. Wysyłanie z bufora → BLE NOTIFY ────────────────────────────
    if (deviceConnected) {
        while (canRxBuf.pop(frame)) {
            uint8_t buf[14];
            frameToBytes(frame, buf);
            pCanRxChar->setValue(buf, 6 + frame.length);
            pCanRxChar->notify();
            canTxCount++;
        }
    } else {
        // Nie połączony — opróżnij bufor by nie zalegał
        while (canRxBuf.pop(frame)) { /* discard */ }
    }

    // ── 3. Odbiór z BLE → bufor Tx → CAN ──────────────────────────────
    CANFrame txFrame;
    while (bleTxBuf.pop(txFrame)) {
        mcp2515_sendFrame(txFrame);
    }

    // ── 3b. CAN bus-off recovery ─────────────────────────────────
    uint8_t tec = mcp_read(REG_TEC);
    if (tec > 255) {
        Serial.println("[CAN] BUS-OFF detected — restarting MCP2515...");
        mcp2515_init();
    }

    // ── 5. Status co 5s ────────────────────────────────────────────────
    if (millis() - lastStatus > 5000) {
        lastStatus = millis();
        uint8_t tec = mcp_read(REG_TEC);
        uint8_t rec = mcp_read(REG_REC);
        uint8_t eflg = mcp_read(REG_EFLG);

        Serial.printf("[status] CAN RX:%lu  BLE TX:%lu  Buf: rx=%d/128 tx=%d/64  Drops:%lu  Err: TEC=%d REC=%d EFLG=0x%02X  BLE:%s\n",
                      canRxCount, canTxCount,
                      canRxBuf.available(), bleTxBuf.available(),
                      bleRxDrops,
                      tec, rec, eflg,
                      deviceConnected ? "OK" : "--");
    }

    delay(1);
}