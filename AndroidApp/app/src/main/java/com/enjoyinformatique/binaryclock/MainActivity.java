package com.enjoyinformatique.binaryclock;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.ScrollView;
import android.widget.SeekBar;
import android.widget.TextView;

import java.time.Clock;
import java.util.Set;

public class MainActivity extends AppCompatActivity {
    private TextView logTextView;
    private boolean isServiceBound;
    private ClockService clockService;
    private SeekBar r, g, b;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        logTextView = findViewById(R.id.logTextView);
        logTextView.append("Launched\n");
        findViewById(R.id.updateTimeButton).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (isServiceBound) {
                    clockService.updateDate();
                }
            }
        });
        r = findViewById(R.id.r);
        g = findViewById(R.id.g);
        b = findViewById(R.id.b);
        SeekBar.OnSeekBarChangeListener colorChangeListener = new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    if (isServiceBound) {
                        clockService.setColor(r.getProgress(), g.getProgress(), b.getProgress());
                    }
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        };
        r.setOnSeekBarChangeListener(colorChangeListener);
        g.setOnSeekBarChangeListener(colorChangeListener);
        b.setOnSeekBarChangeListener(colorChangeListener);
    }

    @Override
    protected void onStart() {
        super.onStart();
        LocalBroadcastManager.getInstance(this).registerReceiver(broadcastReceiver, new IntentFilter(ClockService.INTENT_KEY));
        bindService(new Intent(this, ClockService.class), serviceConnection, Context.BIND_AUTO_CREATE);
    }

    @Override
    protected void onStop() {
        super.onStop();
        if (isServiceBound) {
            unbindService(serviceConnection);
            isServiceBound = false;
        }
    }

    private ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            ClockService.ClockServiceBinder binder = (ClockService.ClockServiceBinder) service;
            clockService = binder.getService();
            isServiceBound = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            isServiceBound = false;
        }
    };

    private BroadcastReceiver broadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String line = intent.getStringExtra("line");
            logTextView.append(line + "\n");
            final ScrollView scrollView = findViewById(R.id.scrollView2);
            scrollView.postDelayed(new Runnable() {
                @Override
                public void run() {
                    scrollView.fullScroll(ScrollView.FOCUS_DOWN);
                }
            }, 50);
        }
    };
}
