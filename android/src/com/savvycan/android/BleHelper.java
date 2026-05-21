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

public class BleHelper {
    private static final String TAG = "SavvyCAN-BLE";

    private static final String LED_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
    private static final String LED_CHAR_UUID    = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

    private static final String CAN_SERVICE_UUID = "4fafc202-1fb5-459e-8fcc-c5c9c331914b";
    private static final String CAN_RX_CHAR_UUID = "beb5483f-36e1-4688-b7f5-ea07361b26a8";
    private static final String CAN_TX_CHAR_UUID = "beb54840-36e1-4688-b7f5-ea07361b26a8";

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
    private String              lastConnectedAddress;
    private boolean             userDisconnect = false;

    public static native void onDeviceFound(String name, String address, int rssi);
    public static native void onScanFinished();
    public static native void onConnected(String address);
    public static native void onDisconnected();
    public static native void onLedStateChanged(int state);
    public static native void onCanFrameReceived(byte[] data);
    public static native void onError(String message);

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
        Log.i(TAG, "BleHelper initialized. BT enabled=" + btAdapter.isEnabled());
    }

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
            Log.i(TAG, "BLE scan stopped");
        }
    }

    private final ScanCallback scanCallback = new ScanCallback() {
        @Override
        public void onScanResult(int cbType, ScanResult result) {
            BluetoothDevice device = result.getDevice();
            String name = device.getName();
            if (name == null) name = "(unknown)";
            Log.i(TAG, "Scan found: " + name + " [" + device.getAddress() + "] RSSI=" + result.getRssi());
            onDeviceFound(name, device.getAddress(), result.getRssi());
        }
        @Override public void onBatchScanResults(List<ScanResult> r) { for (ScanResult s : r) onScanResult(0, s); }
        @Override public void onScanFailed(int code) { scanning = false; Log.e(TAG, "Scan failed: " + code); onError("Scan failed: " + code); }
    };

    public void connectToDevice(String address) {
        Log.i(TAG, "connectToDevice called: " + address);
        if (btAdapter == null) { onError("No adapter"); return; }
        userDisconnect = false;
        disconnect();
        BluetoothDevice device = btAdapter.getRemoteDevice(address);
        if (device == null) { Log.e(TAG, "getRemoteDevice returned null"); onError("Device not found"); return; }
        Log.i(TAG, "Connecting GATT to " + address + "...");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            gatt = device.connectGatt(null, false, gattCallback, BluetoothDevice.TRANSPORT_LE);
        } else {
            gatt = device.connectGatt(null, false, gattCallback);
        }
        if (gatt == null) {
            Log.e(TAG, "connectGatt returned NULL");
            onError("connectGatt returned null");
        } else {
            connectedAddress = address;
            lastConnectedAddress = address;
            Log.i(TAG, "connectGatt OK, waiting for callback...");
        }
    }

    public void disconnect() {
        Log.i(TAG, "disconnect called");
        userDisconnect = true;
        if (gatt != null) { gatt.disconnect(); gatt.close(); gatt = null; }
        ledChar = null; canRxChar = null; canTxChar = null; connectedAddress = null;
    }

    public boolean isConnected() { return gatt != null && connectedAddress != null; }

    private final BluetoothGattCallback gattCallback = new BluetoothGattCallback() {
        @Override
        public void onConnectionStateChange(BluetoothGatt g, int status, int newState) {
            Log.i(TAG, "onConnectionStateChange: status=" + status + " newState=" + newState);
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Connection state change ERROR: status=" + status);
            }
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.i(TAG, "STATE_CONNECTED — calling discoverServices()");
                gatt.discoverServices();
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.i(TAG, "STATE_DISCONNECTED");
                final String addr = connectedAddress;
                ledChar = null; canRxChar = null; canTxChar = null;
                connectedAddress = null;
                handler.post(() -> onDisconnected());
                // Auto-reconnect on unexpected disconnect
                if (!userDisconnect && addr != null && !addr.isEmpty()) {
                    Log.i(TAG, "Scheduling auto-reconnect in 3s to " + addr);
                    handler.postDelayed(() -> connectToDevice(addr), 3000);
                }
            }
        }

        @Override
        public void onServicesDiscovered(BluetoothGatt g, int status) {
            Log.i(TAG, "onServicesDiscovered: status=" + status);
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Service discovery FAILED: " + status);
                onError("Service discovery failed: " + status);
                return;
            }

            List<BluetoothGattService> svcList = gatt.getServices();
            Log.i(TAG, "Found " + svcList.size() + " services:");
            for (BluetoothGattService s : svcList) {
                Log.i(TAG, "  service: " + s.getUuid().toString() + " (instance=" + s.getInstanceId() + ")");
                for (BluetoothGattCharacteristic c : s.getCharacteristics()) {
                    Log.i(TAG, "    char: " + c.getUuid().toString() + " props=" + c.getProperties());
                }
            }

            BluetoothGattService canSvc = gatt.getService(UUID.fromString(CAN_SERVICE_UUID));
            if (canSvc == null) {
                Log.e(TAG, "CAN SERVICE NOT FOUND! UUID=" + CAN_SERVICE_UUID);
            } else {
                Log.i(TAG, "CAN service FOUND");
                canRxChar = canSvc.getCharacteristic(UUID.fromString(CAN_RX_CHAR_UUID));
                canTxChar = canSvc.getCharacteristic(UUID.fromString(CAN_TX_CHAR_UUID));

                if (canRxChar == null) {
                    Log.e(TAG, "CAN RX char NOT FOUND! UUID=" + CAN_RX_CHAR_UUID);
                } else {
                    Log.i(TAG, "CAN RX char found, props=" + canRxChar.getProperties());
                    gatt.setCharacteristicNotification(canRxChar, true);
                    Log.i(TAG, "setCharacteristicNotification called");

                    BluetoothGattDescriptor desc = canRxChar.getDescriptor(
                        UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"));
                    if (desc == null) {
                        Log.e(TAG, "CCCD descriptor NOT FOUND!");
                    } else {
                        Log.i(TAG, "CCCD descriptor found, writing enable...");
                        desc.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
                        boolean ok = gatt.writeDescriptor(desc);
                        Log.i(TAG, "writeDescriptor returned: " + ok);
                    }
                }
                if (canTxChar != null) Log.i(TAG, "CAN TX char found");
                else Log.w(TAG, "CAN TX char NOT FOUND");
            }

            BluetoothGattService ledSvc = gatt.getService(UUID.fromString(LED_SERVICE_UUID));
            if (ledSvc != null) {
                ledChar = ledSvc.getCharacteristic(UUID.fromString(LED_CHAR_UUID));
                if (ledChar != null) Log.i(TAG, "LED char found");
            }

            handler.post(() -> onConnected(gatt.getDevice().getAddress()));
        }

        @Override
        public void onCharacteristicRead(BluetoothGatt g, BluetoothGattCharacteristic c, int status) {
            if (status == BluetoothGatt.GATT_SUCCESS && c.getUuid().equals(UUID.fromString(LED_CHAR_UUID))) {
                byte[] data = c.getValue();
                if (data != null && data.length > 0) {
                    final int state = (data[0] == '1') ? 1 : 0;
                    handler.post(() -> onLedStateChanged(state));
                }
            }
        }

        @Override
        public void onCharacteristicWrite(BluetoothGatt g, BluetoothGattCharacteristic c, int status) {
            Log.i(TAG, "onCharacteristicWrite: uuid=" + c.getUuid() + " status=" + status);
            if (status == BluetoothGatt.GATT_SUCCESS && ledChar != null
                    && c.getUuid().equals(UUID.fromString(LED_CHAR_UUID))) {
                gatt.readCharacteristic(ledChar);
            }
        }

        @Override
        public void onCharacteristicChanged(BluetoothGatt g, BluetoothGattCharacteristic c) {
            Log.i(TAG, "onCharacteristicChanged: uuid=" + c.getUuid());
            if (c.getUuid().equals(UUID.fromString(CAN_RX_CHAR_UUID))) {
                byte[] data = c.getValue();
                Log.i(TAG, "CAN notification: " + (data != null ? data.length : 0) + " bytes");
                if (data != null && data.length >= 6) {
                    final byte[] copy = data.clone();
                    handler.post(() -> onCanFrameReceived(copy));
                }
            }
        }

        @Override
        public void onDescriptorWrite(BluetoothGatt g, BluetoothGattDescriptor d, int status) {
            Log.i(TAG, "onDescriptorWrite: desc=" + d.getUuid() + " char=" + d.getCharacteristic().getUuid() + " status=" + status);
            if (status == BluetoothGatt.GATT_SUCCESS &&
                d.getCharacteristic().getUuid().equals(UUID.fromString(CAN_RX_CHAR_UUID))) {
                Log.i(TAG, "CAN NOTIFICATIONS ENABLED SUCCESSFULLY");
            } else if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.e(TAG, "Descriptor write FAILED: status=" + status);
            }
        }
    };

    public void writeLed(boolean on) {
        if (ledChar == null || gatt == null) { Log.w(TAG, "Cannot write LED: not ready"); return; }
        ledChar.setValue(on ? "1" : "0");
        gatt.writeCharacteristic(ledChar);
    }

    public void readLed() {
        if (ledChar == null || gatt == null) return;
        gatt.readCharacteristic(ledChar);
    }

    public void sendCanFrame(int id, boolean extended, int dlc, byte[] data) {
        if (canTxChar == null || gatt == null) {
            Log.w(TAG, "CAN TX not ready (char=" + (canTxChar!=null) + " gatt=" + (gatt!=null) + ")");
            return;
        }
        if (dlc > 8) dlc = 8;
        byte[] frame = new byte[6 + dlc];
        frame[0] = (byte)(id & 0xFF);
        frame[1] = (byte)((id >> 8) & 0xFF);
        frame[2] = (byte)((id >> 16) & 0xFF);
        frame[3] = (byte)((id >> 24) & 0xFF);
        frame[4] = (byte)dlc;
        frame[5] = (byte)(extended ? 0x01 : 0x00);
        for (int i = 0; i < dlc && i < data.length; i++) frame[6+i] = data[i];
        canTxChar.setValue(frame);
        gatt.writeCharacteristic(canTxChar);
        Log.i(TAG, "CAN TX: ID=" + Integer.toHexString(id) + (extended?" ext":" std") + " DLC=" + dlc);
    }
}