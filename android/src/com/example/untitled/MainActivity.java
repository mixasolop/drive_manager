package com.example.untitled;

import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import org.qtproject.qt.android.bindings.QtActivity;

public class MainActivity extends QtActivity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        QtBridge.init(this);
        Log.d("MAIN", "MainActivity onCreate");
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        QtBridge.onActivityResult(requestCode, resultCode, data);
    }
}

