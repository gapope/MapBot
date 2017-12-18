package com.example.arya5.mapbot;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    private BluetoothAdapter mBtAdapter;
    private String btAddress = null;
    List<BluetoothDevice> devices = new ArrayList<BluetoothDevice>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBtAdapter == null) {
            Toast.makeText(this, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            this.finish();
        }

        if (!mBtAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, 1);
        }

        // Register for broadcasts when a device is discovered
        IntentFilter filter = new IntentFilter(BluetoothDevice.ACTION_FOUND);
        this.registerReceiver(mReceiver, filter);

        // Register for broadcasts when discovery has finished
        filter = new IntentFilter(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);
        this.registerReceiver(mReceiver, filter);

        if (mBtAdapter.isDiscovering()) {
            mBtAdapter.cancelDiscovery();
        }

        int MY_PERMISSIONS_REQUEST_ACCESS_COARSE_LOCATION = 1;
        ActivityCompat.requestPermissions(this,
                new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                MY_PERMISSIONS_REQUEST_ACCESS_COARSE_LOCATION);
        mBtAdapter.startDiscovery();
        Log.d("myID", "STARTED DISCOVERY");
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == 1) {
            if (resultCode != RESULT_OK) {
                Toast.makeText(this, "Bluetooth is not enabled", Toast.LENGTH_LONG).show();
                this.finish();
            }
        }
    }

    public void onClick(View view){
        TextView message = findViewById(R.id.textView);
        message.setText(R.string.bt_loading_message);
        ProgressBar loading = findViewById(R.id.btLoading);
        loading.setVisibility(View.VISIBLE);

        if (mBtAdapter.isDiscovering()) {
            mBtAdapter.cancelDiscovery();
        }
        mBtAdapter.startDiscovery();
        Log.d("myID", "STARTED DISCOVERY");
    }

    private void isDevice(BluetoothDevice device) {
        Log.d("myID", "Device: " + device.getName());
        Log.d("myID", "Address: " + device.getAddress());
        if (device.getName() != null){
            if (device.getName().equals("mapBot")) {
                btAddress = device.getAddress();
                TextView message = findViewById(R.id.textView);
                message.setText(R.string.bt_loading_message);
                Intent startRemote = new Intent(this, RemoteActivity.class);
                startRemote.putExtra("Address", btAddress);
                startActivity(startRemote);
            }
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            // When discovery finds a device
            if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                // Get the BluetoothDevice object from the Intent
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (device.getName() != null){
                    if (device.getName().equals("mapBot")) {
                        btAddress = device.getAddress();
                        Intent startRemote = new Intent(context, RemoteActivity.class);
                        startRemote.putExtra("Address", btAddress);
                        startActivity(startRemote);
                    }
                }

                // When discovery is finished, change the Activity title
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                mBtAdapter.cancelDiscovery();
                Log.d("myID", "ENDED DISCOVERY");
                TextView message = findViewById(R.id.textView);
                message.setText(R.string.bt_message);
                ProgressBar loading = findViewById(R.id.btLoading);
                loading.setVisibility(View.INVISIBLE);
            }
        }
    };
}
