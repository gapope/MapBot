package com.example.arya5.mapbot;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.content.Intent;
import android.view.View;

public class ChoiceActivity extends AppCompatActivity {

    String address = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_choice);

        Intent intent = getIntent();
        address = intent.getStringExtra("Address");
    }

    public void onClick (View view) {
        if (view.getId() == R.id.manual) {
            Intent startRemote = new Intent(this, RemoteActivity.class);
            startRemote.putExtra("Address", address);
            startActivity(startRemote);
        } else {
            Intent startAuto = new Intent(this, AutoActivity.class);
            startAuto.putExtra("Address", address);
            startActivity(startAuto);
        }
    };
}

