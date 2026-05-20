/**
 * MCP2515 SPI Scanner — diagnostyka dla ESP32
 * ============================================
 *
 * Sprawdza czy MCP2515 jest poprawnie podłączony do ESP32 przez SPI.
 * Odczytuje rejestry kontrolne i raportuje stan układu.
 *
 * Połączenia:
 *   MCP2515  →  ESP32
 *   VCC      →  3.3V
 *   GND      →  GND
 *   CS       →  GPIO5  (D5)
 *   SO/MISO  →  GPIO19 (D19)
 *   SI/MOSI  →  GPIO23 (D23)
 *   SCK      →  GPIO18 (D18)
 *   INT      →  GPIO4  (D4)
 *
 * Wyjście: Serial Monitor @ 115200 baud
 */

#include <SPI.h>

// ── Pinout ───────────────────────────────────────────────────────────────
const int CS_PIN  = 5;    // D5  — Chip Select
const int INT_PIN = 4;    // D4  — Interrupt (do odczytu stanu)

// ── MCP2515 SPI Commands ─────────────────────────────────────────────────
#define MCP_CMD_RESET       0xC0
#define MCP_CMD_READ        0x03
#define MCP_CMD_WRITE       0x02
#define MCP_CMD_READ_STATUS 0xA0
#define MCP_CMD_RX_STATUS   0xB0
#define MCP_CMD_BIT_MODIFY  0x05

// ── MCP2515 Registers ────────────────────────────────────────────────────
#define REG_RXF0SIDH  0x00
#define REG_RXF0SIDL  0x01
#define REG_CANSTAT   0x0E
#define REG_CANCTRL   0x0F
#define REG_CANINTE   0x2B
#define REG_CANINTF   0x2C
#define REG_EFLG      0x2D
#define REG_TEC       0x1C   // Transmit Error Counter
#define REG_REC       0x1D   // Receive Error Counter
#define REG_CNF3      0x28
#define REG_CNF2      0x29
#define REG_CNF1      0x2A

// ── Helpers ──────────────────────────────────────────────────────────────
uint8_t mcp2515_readRegister(uint8_t reg) {
    uint8_t result;
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_CMD_READ);
    SPI.transfer(reg);
    result = SPI.transfer(0x00);  // dummy byte to clock out data
    digitalWrite(CS_PIN, HIGH);
    return result;
}

void mcp2515_writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_CMD_WRITE);
    SPI.transfer(reg);
    SPI.transfer(value);
    digitalWrite(CS_PIN, HIGH);
}

void mcp2515_reset() {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_CMD_RESET);
    digitalWrite(CS_PIN, HIGH);
    delay(10);  // MCP2515 needs ~10ms after reset
}

uint8_t mcp2515_readStatus() {
    uint8_t result;
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_CMD_READ_STATUS);
    result = SPI.transfer(0x00);
    digitalWrite(CS_PIN, HIGH);
    return result;
}

void mcp2515_bitModify(uint8_t reg, uint8_t mask, uint8_t data) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(MCP_CMD_BIT_MODIFY);
    SPI.transfer(reg);
    SPI.transfer(mask);
    SPI.transfer(data);
    digitalWrite(CS_PIN, HIGH);
}

// ── Register name lookup ─────────────────────────────────────────────────
struct RegInfo {
    uint8_t addr;
    const char *name;
    const char *desc;
};

const RegInfo regs[] = {
    {0x0E, "CANSTAT",  "CAN status (0x80=config mode after reset)"},
    {0x0F, "CANCTRL",  "CAN control"},
    {0x2B, "CANINTE",  "Interrupt enable"},
    {0x2C, "CANINTF",  "Interrupt flags"},
    {0x2D, "EFLG",     "Error flags"},
    {0x1C, "TEC",      "Transmit error counter"},
    {0x1D, "REC",      "Receive error counter"},
    {0x28, "CNF3",     "Configuration 3"},
    {0x29, "CNF2",     "Configuration 2"},
    {0x2A, "CNF1",     "Configuration 1 (baud rate)"},
    {0x00, "RXF0SIDH", "RX buffer 0 — standard ID high"},
    {0x01, "RXF0SIDL", "RX buffer 0 — standard ID low"},
};

// ── Print binary ─────────────────────────────────────────────────────────
void printBinary(uint8_t val) {
    for (int i = 7; i >= 0; i--) {
        Serial.print((val >> i) & 1);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// Setup
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n");
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║   MCP2515 SPI Scanner for ESP32          ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    pinMode(INT_PIN, INPUT_PULLUP);

    // ── Initialize SPI ────────────────────────────────────────────────
    SPI.begin(18, 19, 23, CS_PIN);  // SCK, MISO, MOSI, SS
    SPI.setFrequency(8000000);       // 8 MHz (safe for MCP2515)
    SPI.setDataMode(SPI_MODE0);      // CPOL=0, CPHA=0
    SPI.setBitOrder(MSBFIRST);

    Serial.println("[1] SPI initialized");
    Serial.printf("    SCK=18  MISO=19  MOSI=23  CS=%d  INT=%d\n", CS_PIN, INT_PIN);
    Serial.printf("    Frequency: 8 MHz, Mode 0\n\n");

    // ── Test INT pin ──────────────────────────────────────────────────
    Serial.println("[2] Testing INT pin (GPIO4)...");
    int intState = digitalRead(INT_PIN);
    Serial.printf("    INT pin state: %s (pull-up enabled)\n",
                  intState == HIGH ? "HIGH ✓" : "LOW (check wiring!)");

    // ── RESET MCP2515 ─────────────────────────────────────────────────
    Serial.println("\n[3] Resetting MCP2515...");
    mcp2515_reset();
    delay(20);
    Serial.println("    Reset command sent.");

    // ── Read status register ──────────────────────────────────────────
    Serial.println("\n[4] Reading status register (READ_STATUS command)...");
    uint8_t status = mcp2515_readStatus();
    Serial.printf("    Status: 0x%02X  ", status);
    printBinary(status);
    Serial.println();

    // Decode status bits
    Serial.printf("    RX0IF:%d  RX1IF:%d  TX0REQ:%d  TX0IF:%d  TX1REQ:%d  TX1IF:%d  TX2REQ:%d  TX2IF:%d\n",
                  (status>>0)&1, (status>>1)&1, (status>>2)&1, (status>>3)&1,
                  (status>>4)&1, (status>>5)&1, (status>>6)&1, (status>>7)&1);

    // ── Read all key registers ────────────────────────────────────────
    Serial.println("\n[5] Reading all control registers...");
    Serial.println("    ┌────────┬──────────┬─────────────────────────────┐");
    Serial.println("    │ Reg    │ Value    │ Binary                      │");
    Serial.println("    ├────────┼──────────┼─────────────────────────────┤");

    bool chipResponding = false;
    bool canstatOK = false;

    for (const RegInfo &r : regs) {
        uint8_t val = mcp2515_readRegister(r.addr);
        Serial.printf("    │ 0x%02X   │ 0x%02X     │ ", r.addr, val);
        printBinary(val);
        Serial.printf(" │ %-27s │\n", r.name);

        // Check if data looks valid (not all 0x00 or 0xFF which would indicate no response)
        if (val != 0x00 && val != 0xFF) {
            chipResponding = true;
        }

        // CANSTAT should be 0x80 after reset (configuration mode)
        if (r.addr == 0x0E) {
            if ((val & 0xE0) == 0x80) {
                canstatOK = true;
            }
        }
    }
    Serial.println("    └────────┴──────────┴─────────────────────────────┘");

    // ── Write-then-read test ──────────────────────────────────────────
    Serial.println("\n[6] Write/Read-back test (CANCTRL register)...");
    uint8_t originalCtrl = mcp2515_readRegister(REG_CANCTRL);
    Serial.printf("    Original CANCTRL: 0x%02X\n", originalCtrl);

    // Write a test value (set CLKEN bit, which is harmless)
    mcp2515_writeRegister(REG_CANCTRL, 0x84);
    delay(5);
    uint8_t readback = mcp2515_readRegister(REG_CANCTRL);
    Serial.printf("    Written: 0x84, Read-back: 0x%02X", readback);

    bool writeOK = (readback == 0x84);
    Serial.printf(" — %s\n", writeOK ? "MATCH ✓" : "MISMATCH ✗");

    // Restore original
    mcp2515_writeRegister(REG_CANCTRL, originalCtrl);

    // ── Bit-modify test ──────────────────────────────────────────────
    Serial.println("\n[7] Bit-modify test (CANINTE register)...");
    uint8_t origIntE = mcp2515_readRegister(REG_CANINTE);
    Serial.printf("    Original CANINTE: 0x%02X\n", origIntE);

    mcp2515_bitModify(REG_CANINTE, 0x01, 0x01);  // Set RX0IE bit
    delay(5);
    uint8_t modVal = mcp2515_readRegister(REG_CANINTE);
    Serial.printf("    After bit-modify (set RX0IE): 0x%02X", modVal);
    bool bitModOK = (modVal & 0x01);
    Serial.printf(" — %s\n", bitModOK ? "OK ✓" : "FAIL ✗");

    mcp2515_bitModify(REG_CANINTE, 0x01, 0x00);  // Clear it
    delay(5);

    // ── Read INT pin after operations ─────────────────────────────────
    Serial.println("\n[8] Final INT pin check...");
    intState = digitalRead(INT_PIN);
    Serial.printf("    INT pin: %s\n", intState == HIGH ? "HIGH (inactive)" : "LOW (active/interrupt pending)");

    // ── Summary ───────────────────────────────────────────────────────
    Serial.println("\n╔══════════════════════════════════════════╗");
    Serial.println("║              DIAGNOSIS                    ║");
    Serial.println("╠══════════════════════════════════════════╣");

    if (!chipResponding) {
        Serial.println("║ ✗ MCP2515 NOT RESPONDING                ║");
        Serial.println("║   Check:                                ║");
        Serial.println("║   - VCC (3.3V) and GND connections       ║");
        Serial.println("║   - CS  → GPIO5  (D5)                   ║");
        Serial.println("║   - SO  → GPIO19 (D19 / MISO)           ║");
        Serial.println("║   - SI  → GPIO23 (D23 / MOSI)           ║");
        Serial.println("║   - SCK → GPIO18 (D18)                  ║");
        Serial.println("║   - INT → GPIO4  (D4)                   ║");
    } else if (!canstatOK) {
        Serial.printf("║ ⚠ MCP2515 responding but CANSTAT=0x%02X ║\n",
                     mcp2515_readRegister(REG_CANSTAT));
        Serial.println("║   Expected 0x80 (config mode)           ║");
        Serial.println("║   Try power-cycling the MCP2515         ║");
    } else if (!writeOK) {
        Serial.println("║ ⚠ SPI read works but write fails       ║");
        Serial.println("║   Check MOSI (GPIO23) connection        ║");
    } else {
        Serial.println("║ ✓ MCP2515 OK — all tests passed         ║");
        Serial.println("║   SPI: working                          ║");
        Serial.println("║   Registers: readable & writable        ║");
        Serial.println("║   CANSTAT: configuration mode (0x80)    ║");
    }
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println();
    Serial.println("Scan complete. Reset ESP32 to re-run.");
}

void loop() {
    // Nothing — one-shot diagnostic
    delay(1000);
}
