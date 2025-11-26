package com.example.untitled;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

public class QtBridge {

    private static String accessToken = "";
    private static String refreshToken = "";

    private static AuthHelper authHelper;

    public static void init(Activity activity) {
        authHelper = new AuthHelper(activity);
        Log.d("BRIDGE", "AuthHelper created");
    }

    public static void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (authHelper != null) {
            authHelper.handleActivityResult(requestCode, resultCode, data);
        } else {
            Log.e("BRIDGE", "authHelper is null in onActivityResult");
        }
    }

    public static void startLogin() {
        if (authHelper != null) {
            authHelper.startLogin();
        } else {
            Log.e("BRIDGE", "authHelper is null in startLogin");
        }
    }

    public static void setTokens(String access, String refresh) {
        accessToken  = access  != null ? access  : "";
        refreshToken = refresh != null ? refresh : "";
        Log.d("BRIDGE", "Tokens set: access=" + accessToken + ", refresh=" + refreshToken);
    }

    public static String getAccessToken() {
        return accessToken;
    }

    public static String getRefreshToken() {
        return refreshToken;
    }
}
