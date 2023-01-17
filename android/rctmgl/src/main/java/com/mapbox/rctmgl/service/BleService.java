package com.mapbox.rctmgl.service;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.ParcelUuid;
import android.util.Log;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;

import com.facebook.react.bridge.Arguments;
import com.facebook.react.bridge.WritableMap;
import com.polidea.rxandroidble3.RxBleClient;
import com.polidea.rxandroidble3.RxBleConnection;
import com.polidea.rxandroidble3.RxBleDevice;
import com.polidea.rxandroidble3.scan.ScanResult;
import com.polidea.rxandroidble3.scan.ScanSettings;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.UUID;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.disposables.CompositeDisposable;
import io.reactivex.rxjava3.disposables.Disposable;

public class BleService extends Service {
    final String BLE_GLOSA_UUID = "9bb78031-77eb-4ed9-b7d2-48ba9bb5304d";
    final String BLE_CAM_UUID = "9bb78032-77eb-4ed9-b7d2-48ba9bb5304d";
    final String BLE_DENM_UUID = "9bb78033-77eb-4ed9-b7d2-48ba9bb5304d";
    final String OBU_POS_UUID = "9bb78030-77eb-4ed9-b7d2-48ba9bb5304d";
    private final String[] BLE_CHARA_UUIDS = {BLE_CAM_UUID, BLE_DENM_UUID, BLE_GLOSA_UUID, OBU_POS_UUID};

    private final IBinder binder = new BleBinder();
    private BleServiceListener listener;
    private Disposable scanDisposable;
    private CompositeDisposable connectionDisposable = new CompositeDisposable();

    RxBleClient rxBleClient;


    @Override
    public void onCreate() {
        super.onCreate();

        rxBleClient = RxBleClient.create(getApplicationContext());
    }

    public void scan() {
        if (!scanDisposable.isDisposed()) scanDisposable.dispose();
        scanDisposable = rxBleClient.scanBleDevices(
                new ScanSettings.Builder()
                        .setScanMode(ScanSettings.SCAN_MODE_LOW_POWER)
                        .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                        .build()
        ).filter(scanResult ->
                scanResult.getScanRecord().getServiceUuids().contains(ParcelUuid.fromString("9bb78000-77eb-4ed9-b7d2-48ba9bb5304d"))
        ).subscribe(scanResult -> {
                    if (listener != null) {
                        WritableMap params = Arguments.createMap();
                        params.putString("deviceName", scanResult.getBleDevice().getName());
                        params.putString("deviceAddress", scanResult.getBleDevice().getMacAddress());
                        params.putInt("rssi", scanResult.getRssi());
                        listener.sendEvent("DEVICE_FOUND", params);
                    }
                },
                throwable ->
                        Log.d("InjectedMaps", "Error during scanning: " + throwable.getMessage())
        );
    }

    public void connect(String deviceAddress) {
        RxBleDevice device = rxBleClient.getBleDevice(deviceAddress);
        connectionDisposable.add(device.establishConnection(false).subscribe(
                rxBleConnection -> {
                    connectionDisposable.add(
                            device.observeConnectionStateChanges()
                                    .filter(state ->
                                            state == RxBleConnection.RxBleConnectionState.DISCONNECTED
                                    )
                                    .subscribe(disconnectEvent -> {
                                        if (listener != null) {
                                            WritableMap params = Arguments.createMap();
                                            params.putString("deviceName", device.getName());
                                            params.putString("deviceAddress", device.getMacAddress());
                                            listener.sendEvent("DEVICE_DISCONNECTED", params);
                                        }
                                    })
                    );
                    for (String uuid : BLE_CHARA_UUIDS) {
                        connectionDisposable.add(
                                rxBleConnection.setupNotification(UUID.fromString(uuid))
                                        .doOnNext(notificationObservable -> {
                                            if (listener != null) {
                                                WritableMap params = Arguments.createMap();
                                                params.putString("deviceName", device.getName());
                                                params.putString("deviceAddress", device.getMacAddress());
                                                params.putString("characteristic", uuid);
                                                listener.sendEvent("DEVICE_SUBSCRIBED", params);
                                            }
                                        })
                                        .flatMap(notificationObservable -> notificationObservable)
                                        .subscribe(
                                                bytes -> {
                                                    if (listener != null) {
                                                        listener.updateChara(uuid, bytes);
                                                    }
                                                },
                                                throwable ->
                                                    Log.d("InjectedMaps", "Error during scanning: " + throwable.getMessage())

                                        ));
                    }
                },
                throwable ->
                        Log.d("InjectedMaps", "Error during connecting: " + throwable.getMessage())
        ));
    }

    public void disconnect() {
        if (!connectionDisposable.isDisposed()) {
            connectionDisposable.dispose();
            connectionDisposable = new CompositeDisposable();
        }
        if (scanDisposable != null && !scanDisposable.isDisposed()) scanDisposable.dispose();
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return this.binder;
    }

    public class BleBinder extends Binder {
        public BleService getService() {
            return BleService.this;
        }

        public void setListener(BleServiceListener listener) {
            BleService.this.listener = listener;
        }
    }

    public interface BleServiceListener {
        void sendEvent(String eventName, @Nullable WritableMap params);
        void updateChara(String characteristic, byte[] payload);
    }
}
