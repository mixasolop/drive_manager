# Drive Manager – Google Drive Client (C++ / Qt)

Drive Manager is an Android application built with Qt (C++/QML).
It allows a user to connect multiple Google Drive accounts and manage files
from one place.

The project focuses mainly on backend logic and integration, not only UI.

## How it works
- C++ backend written using Qt 6
- JNI bridge between C++ and Android Java code
- Google Drive REST API
- Local SQLite database for storage

## Main technical parts

### OAuth 2.0 handling
- Access and refresh tokens are stored locally
- Access token is refreshed automatically when it expires

### File listing
- Files are loaded using paginated API requests
- This avoids loading large file lists into memory at once

### JNI integration
- C++ backend communicates with Android Java using JNI

### Local storage
- SQLite database used to store users, accounts and tokens
- Basic secure storage patterns applied

## Technologies used
- C++ (Qt 6, QML)
- Java (Android)
- SQLite
- Google Drive API
- OAuth 2.0
