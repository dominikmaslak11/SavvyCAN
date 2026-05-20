package com.savvycan.android;

import android.bluetooth.*;
import android.bluetooth.le.*;
import android.content.Context;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import java.util.List;
import java.util.UUID;

/**
 * BLE Helper — zarządza skanowaniem BLE, połączeniami GATT,
 * sterowaniem LED i mostkiem CAN przez BLE.
 */
public class BleHelper {
    private static final String TAG = "SavvyCAN-BLE";

    // Serwis LED
    private static final String LED_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    private static final String LED_CHAR_UUID    = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

    // Serwis CAN
    private static final String CAN_SERVICE_UUID = "4fafc202-1fb5-459e-8fcc-c5c9c331914b";
    private static final String CAN_RX_CHAR_UUID = "beb5483f-36e1-4688-b7f5-ea07361b26a8";  // NOTIFY
    private static final String CAN_TX_CHAR_UUID = "beb54840-36e1-4688-b7f5-ea07361b26a8";  // WRITE

    private BluetoothManager    btManager;
    private BluetoothAdapter    btAdapter;
    private BluetoothLeScanner  leScanner;
    private BluetoothGatt       gatt;
    private BluetoothGattCharacteristic ledChar;
    private BluetoothGattCharacteristic canRxChar;
    private BluetoothGattCharacteristic canTxChar;
    private Handler             handler;
    private boolean             scanning;
    private String              connectedAddress;

    // ── Native methods ──────────────────────────────────────────────────
    public static native void onDeviceFound(String name, String address, int rssi);
    public static native void onScanFinished();
    public static native void onConnected(String address);
    public static native void onDisconnected();
    public static native void onLedStateChanged(int state);
    public static native void onCanFrameReceived(byte[] data);
    public static native void onError(String message);

    // ── Singleton ───────────────────────────────────────────────────────
    private static BleHelper instance;
    public static BleHelper getInstance() { return instance; }

    public BleHelper(Context ctx) {
        instance = this;
        handler = new Handler(Looper.getMainLooper());
        btManager = (BluetoothManager) ctx.getSystemService(Context.BLUETOOTH_SERVICE);
        if (btManager == null) { Log.e(TAG, "BluetoothManager unavailable"); return; }
        btAdapter = btManager.getAdapter();
        if (btAdapter == null) { Log.e(TAG, "BluetoothAdapter unavailable"); return; }
        leScanner = btAdapter.getBluetoothLeScanner();
        Log.i(TAG, "BleHelper initialized");
    }

    // ── Skanowanie ──────────────────────────────────────────────────────
    public void startScan() {
        if (leScanner == null) { onError("BLE scanner not available"); return; }
        ScanSettings settings = new ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();
        scanning = true;
        leScanner.startScan(null, settings, scanCallback);
        Log.i(TAG, "BLE scan started");
    }

    public void stopScan() {
        if (leScanner != null && scanning) {
            leScanner.stopScan(scanCallback);
            scanning = false;
        }
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int cbType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            String name = device.getName();
            if (name == null) name = "Unknown";
            onDeviceFound(name, device.getAddress(), result.getRssi());
        }
        @Override
        public void onBatchScanResults(List<ScanResult> results) {
            for (ScanResult r : results) onScanResult(0, r);
        }
        @Override
        public void onScanFailed(int errorCode) {
            scanning = false;
            onError("Scan failed: code " + errorCode);
        }
    };

    // ── Połączenie GATT ─────────────────────────────────────────────────
    public void connectToDevice(String address) {
        if (btAdapter == null) { onError("No adapter"); return; }
        disconnect();
        BluetoothDevice device = btAdapter.getRemoteDevice(address);
        if (device == null) { onError("Device not found: " + address); return; }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            gatt = device.connectGatt(null, false, gattCallback, BluetoothDevice.TRANSPORT_LE);
        } else {
            gatt = device.connectGatt(null, false, gattCallback);
        }
        if (gatt == null) {
            onError("connectGatt returned null");
        } else {
            connectedAddress = address;
        }
    }

    public void disconnect() {
        if (gatt != null) { gatt.disconnect(); gatt.close(); gatt = null; }
        ledChar = null;
        canRxChar = null;
        canTxChar = null;
        connectedAddress = null;
    }

    public boolean isConnected() {
        return gatt != null && connectedAddress != null;
    }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "GATT connected — discovering services");
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "GATT disconnected");
                ledChar = null; canRxChar = null; canTxChar = null;
                handler.post(() -> onDisconnected());
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) {
                onError("Service discovery failed: " + status);
                return;
            }
            Log.i(TAG, "Services discovered");

            // ── LED service ──────────────────────────────────────────
            BluetoothGattService ledSvc = gatt.getService(UUID.fromString(LED_SERVICE_UUID));
            if (ledSvc != null) {
                ledChar = ledSvc.getCharacteristic(UUID.fromString(LED_CHAR_UUID));
                if (ledChar != null) Log.i(TAG, "LED char found");
            }

            // ── CAN service ──────────────────────────────────────────
            BluetoothGattService canSvc = gatt.getService(UUID.fromString(CAN_SERVICE_UUID));
            if (canSvc != null) {
                canRxChar = canSvc.getCharacteristic(UUID.fromString(CAN_RX_CHAR_UUID));
                canTxChar = canSvc.getCharacteristic(UUID.fromString(CAN_TX_CHAR_UUID));

                if (canRxChar != null) {
                    // Włącz notyfikacje
                    gatt.setCharacteristicNotification(canRxChar, true);
                    BluetoothGattDescriptor desc = canRxChar.getDescriptor(
                        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
                    if (desc != null) {
                        desc.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        gatt.writeDescriptor(desc);
                    }
                    Log.i(TAG, "CAN RX char found, notifications enabled");
                }
                if (canTxChar != null) Log.i(TAG, "CAN TX char found");
            }

            handler.post(() -> onConnected(gatt.getDevice().getAddress()));

            // Odczytaj stan LED jeśli dostępny
            if (ledChar != null) gatt.readCharacteristic(ledChar);
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt gatt,
                BluetoothGattCharacteristic c, int status) {
            if (status != BluetoothGatt.GATT_SUCCESS) return;
            if (c.getUuid().equals(UUID.fromString(LED_CHAR_UUID))) {
                byte[] data = c.getValue();
                if (data != null && data.length > 0) {
                    final int state = (data[0] == '1') ? 1 : 0;
                    handler.post(() -> onLedStateChanged(state));
                }
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt gatt,
                BluetoothGattCharacteristic c, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS && ledChar != null
                    && c.getUuid().equals(UUID.fromString(LED_CHAR_UUID))) {
                gatt.readCharacteristic(ledChar);
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt gatt,
                BluetoothGattCharacteristic c) {
            // CAN RX notification
            if (c.getUuid().equals(UUID.fromString(CAN_RX_CHAR_UUID))) {
                byte[] data = c.getValue();
                if (data != null && data.length >= 6) {
                    final byte[] copy = data.clone();
                    handler.post(() -> onCanFrameReceived(copy));
                }
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt gatt,
                BluetoothGattDescriptor desc, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS &&
                desc.getCharacteristic().getUuid().equals(UUID.fromString(CAN_RX_CHAR_UUID))) {
                Log.i(TAG, "CAN notifications enabled");
            }
        }
    };

    // ── LED ─────────────────────────────────────────────────────────────
    public void writeLed(boolean on) {
        if (ledChar == null || gatt == null) return;
        ledChar.setValue(on ? "1" : "0");
        gatt.writeCharacteristic(ledChar);
    }

    public void readLed() {
        if (ledChar == null || gatt == null) return;
        gatt.readCharacteristic(ledChar);
    }

    // ── CAN ─────────────────────────────────────────────────────────────
    /**
     * Wysyła ramkę CAN na magistralę przez ESP32.
     *
     * @param id       CAN ID (11-bit standard lub 29-bit extended)
     * @param extended true jeśli extended frame
     * @param dlc      długość danych (0-8)
     * @param data     8 bajtów danych (tylko dlc pierwszych jest używane)
     */
    public void sendCanFrame(int id, boolean extended, int dlc, byte[] data) {
        if (canTxChar == null || gatt == null) {
            Log.w(TAG, "CAN not ready");
            return;
        }

        if (dlc > 8) dlc = 8;
        int frameLen = 6 + dlc;
        byte[] frame = new byte[frameLen];

        // ID little-endian
        frame[0] = (byte)(id & 0xFF);
        frame[1] = (byte)((id >> 8) & 0xFF);
        frame[2] = (byte)((id >> 16) & 0xFF);
        frame[3] = (byte)((id >> 24) & 0xFF);
        // DLC
        frame[4] = (byte)dlc;
        // Flags
        frame[5] = (byte)(extended ? 0x01 : 0x00);
        // Data
        for (int i = 0; i < dlc && i < data.length; i++) {
            frame[6 + i] = data[i];
        }

        canTxChar.setValue(frame);
        gatt.writeCharacteristic(canTxChar);
        Log.i(TAG, "CAN frame sent: ID=" + Integer.toHexString(id) +
              (extended ? " ext" : " std") + " DLC=" + dlc);
    }
}
