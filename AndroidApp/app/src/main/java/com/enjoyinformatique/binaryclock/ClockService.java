package com.enjoyinformatique.binaryclock;

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;

import java.io.IOException;
import java.util.Set;
import java.util.UUID;

public class ClockService extends Service {
    public static final String INTENT_KEY = "com.enjoyinformatique.broadcast.LOG";
    private static final String BINARY_CLOCK_BLUETOOTH_NAME = "LEDPANEL0001";
    private ClockServiceBinder bind = new ClockServiceBinder();
    private BluetoothAdapter bluetoothAdapter;
    // SPP UUID service
    private static final UUID SERIAL_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    private boolean wasBluetoothEnabled;
    private BluetoothDevice device;
    private BluetoothSocket connectedSocket;

    public ClockService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        this.bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        return this.bind;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (connectedSocket != null && connectedSocket.isConnected()) {
            try {
                connectedSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public class ClockServiceBinder extends Binder {
        ClockService getService() {
            return ClockService.this;
        }
    }

    private void pre() throws IOException {
        wasBluetoothEnabled = bluetoothAdapter.isEnabled();
        if (!wasBluetoothEnabled) {
            log("Bluetooth enabled");
            bluetoothAdapter.enable();
        } else {
            log("Bluetooth was enabled");
        }
        device = this.getBinaryClockDevice();

        if (connectedSocket == null || !connectedSocket.isConnected())
            connectedSocket = getConnectedSocket();
    }

    private void post() {
    }

    public void updateDate() {
        try {
            pre();
            connectedSocket.getOutputStream().write(("T>"+ (System.currentTimeMillis()/1000) + ";").getBytes());
            log("Sent current time");
            log("Disconnected");
        } catch (Exception e) {
            log("ERROR: " + e.getMessage());
            e.printStackTrace();
        } finally {
            post();
        }
    }


    public void setColor(int r, int g, int b) {
        try {
            pre();
            connectedSocket.getOutputStream().write(("C>" + String.valueOf(r) + String.valueOf(g) + String.valueOf(b) + ";").getBytes());
        } catch(Exception e) {

        } finally {
            post();
        }
    }

    private BluetoothSocket getConnectedSocket() throws IOException {
        BluetoothSocket socket = device.createInsecureRfcommSocketToServiceRecord(SERIAL_UUID);
        socket.connect();
        log("Connected to serial socket on " + device.getName());
        return socket;
    }


    protected BluetoothDevice getBinaryClockDevice() {
        Set<BluetoothDevice> pairedDevices = this.bluetoothAdapter.getBondedDevices();
        BluetoothDevice device = null;
        log("Listing bluetooth devices:");
        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice d: pairedDevices) {
                if (BINARY_CLOCK_BLUETOOTH_NAME.equals(d.getName())) {
                    device = d;
                }
            }
        }
        return device;
    }

    protected void log(String line) {
        Intent intent = new Intent(INTENT_KEY);
        intent.putExtra("line", line);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);
    }

}
