# ESP32 Smart MPPT Solar Charge Controller

A feature-rich, open-source Maximum Power Point Tracking (MPPT) solar charge controller built on the ESP32 microcontroller. This project combines professional-grade solar charging algorithms with IoT capabilities for remote monitoring and control.

## 🌟 Key Features

### Power Management
- **Hybrid Charging Algorithms**: Supports both MPPT + CC-CV and CC-CV only modes
- **Dual Operation Modes**: Charger mode for batteries or PSU mode for direct power supply
- **Automatic MPPT Tracking**: Real-time power point optimization for maximum solar harvesting
- **Predictive PWM Control**: Prevents backflow current and ensures stable operation
- **Multi-stage Charging**: Constant Current (CC) and Constant Voltage (CV) profiles

### Monitoring & Telemetry
- **Real-time LCD Display**: 16x2 I2C LCD with multi-page data views
- **Web Dashboard**: Built-in web server for remote monitoring and control
- **Telegram Bot**: Remote commands and notifications via Telegram
- **Data Logging**: Historical data storage with 1-minute resolution
- **Serial Telemetry**: Configurable serial output for debugging

### Protection Systems
- **Comprehensive Fault Detection**: Over-temperature, over-current, over-voltage protection
- **Backflow Prevention**: MOSFET-based input/output isolation
- **Battery Health Monitoring**: SOC calculation and low-voltage disconnect
- **Error Counting**: Persistent error tracking with configurable limits

### IoT & Connectivity
- **WiFi Management**: WiFiManager for easy network configuration
- **Dual WiFi Modes**: Station + Access Point simultaneous operation
- **REST API**: Full control via HTTP endpoints
- **Remote Access**: Compatible with ngrok and port forwarding
- **Network Info Display**: IP addresses and connection status on LCD

### Hardware Compatibility
- **ADS1115/ADS1015**: Support for both 16-bit and 12-bit external ADCs
- **Temperature Sensing**: NTC thermistor support with Steinhart-Hart equation
- **PWM Fan Control**: Temperature-based cooling management
- **Load Control**: Manual, automatic, and remote load switching
- **Button Interface**: 4-button navigation for local control

## 📊 Technical Specifications

| Parameter | Range | Resolution |
|-----------|-------|------------|
| Input Voltage | 0-50V | 0.1V |
| Output Voltage | 0-50V | 0.01V |
| Charging Current | 0-30A | 0.01A |
| Temperature | 0-100°C | 0.1°C |
| PWM Frequency | 39kHz | 11-bit |
| Data Logging | 5 days | 1 minute |

## 🖥️ Web Dashboard Features

- Real-time sensor data display (voltage, current, power)
- Interactive charts for voltage, current, and power trends
- Historical data analysis with selectable time ranges
- Control panel for:
  - Switching between MPPT+CC-CV and CC-CV modes
  - Toggling Charger/PSU operation modes
  - Adjusting battery voltage limits (max/min)
  - Setting charging current limits
  - Configuring fan trigger temperature
  - Remote load ON/OFF control
  - Factory reset option

## 📱 Telegram Bot Commands
/status - Complete system status
/solar - Solar power details
/battery - Battery status
/stats - Quick statistics
/ip - Network information
/loadon - Turn load ON
/loadoff - Turn load OFF
/settemp - Set fan temperature
/help - Show all commands

text

## 🔧 Hardware Requirements

- ESP32 Development Board
- ADS1115 or ADS1015 ADC Module
- 16x2 I2C LCD Display
- Voltage dividers (20.75:1 input, 21.25:1 output)
- Current sensor (0.066V/A sensitivity)
- NTC thermistor (8kΩ)
- IR2104 MOSFET driver
- Backflow protection MOSFET
- Cooling fan (optional)
- 4x Push buttons for navigation

## 📁 Code Structure
├── Esp32_MPPT_FIRMWARE_V24updatetelegra2.ino # Main file
├── 2_Read_Sensors.ino # Sensor reading & calibration
├── 3_Device_Protection.ino # Fault detection & protection
├── 4_Charging_Algorithm.ino # MPPT & CC-CV algorithms
├── 5_System_Processes.ino # Background processes
├── 6_Onboard_Telemetry.ino # Serial output
├── 7_Wireless_Telemetry.ino # Web server & dashboard
├── 8_LCD_Menu.ino # LCD display & menu
├── 9_Load.ino # Load control
├── 10_Remote_Access.ino # Remote access utilities
└── 12_Telegram_Bot.ino # Telegram integration

text

## 🚀 Getting Started

1. **Clone the repository**
   ```bash
   git clone https://github.com/yourusername/ESP32-MPPT-Solar-Charger.git
Install required libraries

Adafruit ADS1X15

LiquidCrystal I2C

WiFiManager

ArduinoJson

UniversalTelegramBot

TimeLib

Configure hardware connections as per pin definitions in the code

Upload the code to your ESP32 using Arduino IDE or PlatformIO

First boot: Connect to the "ESP32_MPPT_PRO" hotspot (password: 12345678) or configure WiFi via the captive portal

Access the dashboard: Navigate to the ESP32's IP address in your browser

⚙️ Configuration Options
The system offers extensive configuration via the LCD menu or web interface:

Charging Algorithm: MPPT+CC-CV or CC-CV only

Operation Mode: Charger or PSU

Battery Voltage Limits: Max and min voltage thresholds

Charging Current: Maximum current limit

Fan Settings: Enable/disable and trigger temperature

WiFi: Enable/disable wireless features

LCD Backlight: Configurable sleep timer (10s to 1 month)

Factory Reset: Restore all settings to defaults

📈 Data Logging
The system automatically logs data every minute and stores up to 5 days of historical data in EEPROM. Logged parameters include:

Solar voltage, current, and power

Battery voltage and current

Temperature

Timestamp for each data point

🔒 Safety Features
Over-temperature shutdown: Automatically disables charging at critical temperatures

Over-current protection: Both input and output current limits

Over-voltage protection: Prevents battery overcharging

Low voltage disconnect: Protects battery from deep discharge

Backflow prevention: MOSFET-based isolation when conditions aren't met

Error counting: Persistent errors trigger system pauses

🌐 Remote Access Options
Local Network: Access via ESP32's IP address

Hotspot: Connect directly to ESP32's WiFi (192.168.4.1)

Port Forwarding: Forward port 80 on your router

ngrok: Create secure tunnel for internet access

Telegram Bot: Control via Telegram messages

📝 License
This project is open-source and available under the MIT License.

🤝 Contributing
Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.
