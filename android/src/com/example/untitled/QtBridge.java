package com.example.untitled;

import android.app.Activity;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import android.database.Cursor;
import android.provider.OpenableColumns;
import android.content.ContentResolver;
import java.io.InputStream;
import java.io.ByteArrayOutputStream;
import android.database.Cursor;
import android.provider.OpenableColumns;


public class QtBridge {

    private static final int RC_PICK_FILES = 3333;

    private static String accessToken = "";
    private static String refreshToken = "";
    public static String userName = "";
    public static String userEmail = "";

    private static AuthHelper authHelper;
    private static Activity activity;

    public static void init(Activity act) {
        activity = act;
        authHelper = new AuthHelper(act);
        Log.d("BRIDGE", "AuthHelper created");
    }

    public static void startLogin() {
        if (authHelper != null) {
            authHelper.startLogin();
        }
    }

    public static void openFilePicker() {
        pickFiles();
    }

    public static void pickFiles() {
        if (activity == null) {
            Log.e("BRIDGE", "Activity is null in pickFiles()");
            return;
        }
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        activity.startActivityForResult(intent, RC_PICK_FILES);
    }

    public static void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode == RC_PICK_FILES && resultCode == Activity.RESULT_OK) {
            if (data == null) return;

            if (data.getClipData() != null) {
                int count = data.getClipData().getItemCount();
                String[] result = new String[count];
                for (int i = 0; i < count; i++) {
                    Uri uri = data.getClipData().getItemAt(i).getUri();
                    result[i] = getPathFromUri(uri);
                }
                onFilesSelected(result);
            } else if (data.getData() != null) {
                Uri uri = data.getData();
                String[] result = new String[]{ getPathFromUri(uri) };
                onFilesSelected(result);
            }
            return;
        }

        if (authHelper != null) {
            authHelper.handleActivityResult(requestCode, resultCode, data);
        }
    }

    private static String getPathFromUri(Uri uri) {
        return uri.toString();
    }

    public static void setTokens(String access, String refresh) {
        accessToken = access != null ? access : "";
        refreshToken = refresh != null ? refresh : "";
        Log.d("BRIDGE", "Tokens set: access=" + accessToken + ", refresh=" + refreshToken);
    }

    public static String getAccessToken() {
        return accessToken;
    }

    public static String getRefreshToken() {
        return refreshToken;
    }

    public static native void onFilesSelected(String[] paths);


    public static byte[] readFileBytes(String uriString) {
        try {
            Uri uri = Uri.parse(uriString);
            InputStream is = activity.getContentResolver().openInputStream(uri);

            if (is == null) return null;

            ByteArrayOutputStream buffer = new ByteArrayOutputStream();
            byte[] data = new byte[4096];
            int nRead;

            while ((nRead = is.read(data, 0, data.length)) != -1) {
                buffer.write(data, 0, nRead);
            }
            is.close();
            return buffer.toByteArray();

        } catch (Exception e) {
            Log.e("BRIDGE", "readFileBytes ERROR:", e);
            return null;
        }
    }


    public static void setUserInfo(String name, String email) {
        userName = name;
        userEmail = email;
    }

    public static String getUserName() { return userName; }
    public static String getUserEmail() { return userEmail; }

    public static String getFileName(String uriString) {
        Uri uri = Uri.parse(uriString);
        String result = null;

        Cursor cursor = activity.getContentResolver()
                .query(uri, null, null, null, null);

        if (cursor != null) {
            int nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
            if (cursor.moveToFirst()) {
                result = cursor.getString(nameIndex);
            }
            cursor.close();
        }
        return result != null ? result : "unknown";
    }

    public static String getMimeType(String uriString) {
        Uri uri = Uri.parse(uriString);
        return activity.getContentResolver().getType(uri);
    }

    public static native void onLoginFinished();


}
