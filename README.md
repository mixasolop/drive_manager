# Google Drive Manager (Qt / C++ / Android)

Google Drive Manager is a **native Android app built with Qt (C++/QML)** that lets users
connect and manage **multiple Google Drive accounts** from one place.

This project was built to gain hands-on experience with **C++ application logic,
Android integration, OAuth authentication, and local storisation**.

---

## Features

- Connect multiple Google Drive accounts
- Google OAuth 2.0 authentication with access token refresh
- File listing with pagination (to handle large accounts efficiently)
- Upload local files to selected Drive accounts
- Local data persistence using SQLite

---

## Tech stack

- **C++ / QML (Qt 6)**
- **Java (Android, JNI integration)**
- **SQLite**
- **Google Drive API & OAuth 2.0**

---

## Notes

- Application logic is implemented in C++ (Qt backend)
- Android-specific functionality is accessed via a Qt–Java JNI bridge
- Token lifecycle management is handled automatically

---

## What I learned

- Integrating C++ code with Android via JNI  
- Working with OAuth flows and REST APIs  
- Designing simple but scalable application logic
