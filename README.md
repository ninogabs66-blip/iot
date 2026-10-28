# ESP32 Distance Monitoring System

This project uses an ESP32, HC-SR04 ultrasonic sensor, 10 LEDs, three buttons, Wi-Fi, PHP, and MySQL.

## Repository layout
- `esp32/esp32_distance_monitor.ino` — Arduino sketch
- `esp_project/dashboard.php` — dashboard page
- `esp_project/insert.php` — HTTP POST endpoint
- `database/schema.sql` — database and table setup

## Before running
1. Set your Wi-Fi SSID and password locally in the Arduino sketch. Do not commit real credentials to this public repository.
2. Configure the PHP database connection for your own MySQL setup.
3. Put the `esp_project` folder under your Apache web root so the endpoint matches `/esp_project/insert.php`.
4. Import `database/schema.sql` into MySQL.
5. Make sure the ESP32 and server are on the same reachable network. Update `SERVER_URL` to the server's current LAN IP.

The PHP endpoint accepts form fields named `distance` and `status`. The dashboard refreshes every 3 seconds.