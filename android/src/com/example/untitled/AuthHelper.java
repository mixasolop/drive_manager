package com.example.untitled;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

import com.google.android.gms.auth.api.signin.*;
import com.google.android.gms.common.api.ApiException;
import com.google.android.gms.common.api.Scope;
import com.google.android.gms.tasks.Task;

import org.json.JSONObject;

import java.io.*;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;

public class AuthHelper {

    private static final String CLIENT_ID =
            "672191165584-lrc87gr0ip4ipra19s7k67tq8traagnm.apps.googleusercontent.com";
    private static final int RC_SIGN_IN = 100;

    private final Activity activity;
    private GoogleSignInClient googleSignInClient;

    public AuthHelper(Activity activity) {
        this.activity = activity;
        GoogleSignInOptions gso = new GoogleSignInOptions.Builder(GoogleSignInOptions.DEFAULT_SIGN_IN)
                .requestEmail()
                .requestIdToken(CLIENT_ID)
                .requestServerAuthCode(CLIENT_ID, true)
                .requestScopes(new Scope("https://www.googleapis.com/auth/drive"))
                .build();
        googleSignInClient = GoogleSignIn.getClient(activity, gso);
    }
    public void startLogin() {
        googleSignInClient.signOut().addOnCompleteListener(task -> {
            Intent signInIntent = googleSignInClient.getSignInIntent();
            activity.startActivityForResult(signInIntent, RC_SIGN_IN);
        });
    }

    public void doAuthFlow() {
        Intent signInIntent = googleSignInClient.getSignInIntent();
        activity.startActivityForResult(signInIntent, RC_SIGN_IN);
    }

    public void handleActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == RC_SIGN_IN) {
            Task<GoogleSignInAccount> task = GoogleSignIn.getSignedInAccountFromIntent(data);
            try {
                GoogleSignInAccount account = task.getResult(ApiException.class);
                String authCode = account.getServerAuthCode();
                exchangeAuthCodeForTokens(authCode);
            } catch (ApiException e) {
                Log.e("AUTH", "Sign-in failed", e);
            }
        }
    }
    private void exchangeAuthCodeForTokens(String authCode) {
        new Thread(() -> {
            try {
                Log.d("TOKEN_EXCHANGE", "authCode = " + authCode);

                String clientSecret = readClientSecretFromFile();
                if (clientSecret == null || clientSecret.isEmpty()) {
                    Log.e("TOKEN_EXCHANGE", "clientSecret is empty");
                    return;
                }

                URL url = new URL("https://oauth2.googleapis.com/token");
                HttpURLConnection conn = (HttpURLConnection) url.openConnection();
                conn.setRequestMethod("POST");
                conn.setDoOutput(true);
                conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");

                String data = "code=" + URLEncoder.encode(authCode, "UTF-8")
                        + "&client_id=" + URLEncoder.encode(CLIENT_ID, "UTF-8")
                        + "&client_secret=" + URLEncoder.encode(clientSecret, "UTF-8")
                        + "&redirect_uri="
                        + "&grant_type=authorization_code";

                try (OutputStream os = conn.getOutputStream()) {
                    os.write(data.getBytes());
                    os.flush();
                }

                int httpCode = conn.getResponseCode();
                Log.d("TOKEN_EXCHANGE", "HTTP code = " + httpCode);

                InputStream is = (httpCode >= 200 && httpCode < 300)
                        ? conn.getInputStream()
                        : conn.getErrorStream();

                BufferedReader reader = new BufferedReader(new InputStreamReader(is));
                StringBuilder sb = new StringBuilder();
                String line;
                while ((line = reader.readLine()) != null) {
                    sb.append(line);
                }
                reader.close();

                Log.d("TOKEN_EXCHANGE", "Response body: " + sb);

                if (httpCode < 200 || httpCode >= 300) {
                    return;
                }

                JSONObject json = new JSONObject(sb.toString());
                String accessToken = json.optString("access_token", null);
                String refreshToken = json.optString("refresh_token", null);

                Log.d("TOKEN_EXCHANGE", "access=" + accessToken + " refresh=" + refreshToken);

                QtBridge.setTokens(accessToken, refreshToken);
            } catch (Exception e) {
                Log.e("TOKEN_EXCHANGE", "Error exchanging code", e);
            }
        }).start();
    }

    private String readClientSecretFromFile() {
        try (InputStream is = activity.getAssets().open("secret.txt");
             BufferedReader reader = new BufferedReader(new InputStreamReader(is))) {
            return reader.readLine().trim();
        } catch (IOException e) {
            Log.e("SECRET_READ", "Failed to read client secret", e);
            return null;
        }
    }
}
