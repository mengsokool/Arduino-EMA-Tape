#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// กำหนดขาของ HC-SR04
#define TRIG_PIN 9
#define ECHO_PIN 10

// กำหนดหน้าจอ LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ตัวแปรหลัก
float distance = 0.0;
float lastDistance = 0.0;
float filteredDistance = 0.0;
unsigned long lastUpdate = 0;
#define ALPHA 0.1  // กรองค่า Exponential Moving Average (ลดจาก 0.3 เป็น 0.1)

// ตั้งค่าอัตราการอัปเดตจอ LCD
#define BASE_REFRESH_RATE 1000  // ค่าเริ่มต้น (ms)
#define FAST_REFRESH_RATE 100   // ค่าเร็วสุด (ms)
unsigned long refreshRate = BASE_REFRESH_RATE;

// สำหรับ spinner animation
int spinnerIndex = 0;
char spinnerChars[4] = {'|', '/', '-', '\\'};

// สำหรับ gauge bar
const float maxDistance = 200.0; // สมมุติระยะทางสูงสุด 200 cm
const int gaugeWidth = 15;       // ใช้ 15 ช่องสำหรับ gauge bar (ช่องสุดท้ายจองไว้สำหรับ spinner)

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    showSplashScreen(); // แสดง splash screen ตอนเปิดเครื่อง
}

void loop() {
    delay(100);
    // อ่านค่าระยะด้วยการสุ่มตัวอย่างหลายครั้ง (5 ค่า)
    distance = measureDistance();

    // กรองค่าด้วย Exponential Moving Average (EMA)
    filteredDistance = (ALPHA * distance) + ((1 - ALPHA) * filteredDistance);

    // ตรวจจับความเปลี่ยนแปลงของระยะ
    float diff = abs(lastDistance - filteredDistance);

    // ปรับอัตราการอัปเดตจอ LCD ตามการเปลี่ยนแปลง
    if (diff > 50) {
        refreshRate = FAST_REFRESH_RATE;  // เปลี่ยนแปลงมาก → อัปเดตเร็วสุด
    } else if (diff > 5) {
        refreshRate = 300;                // เปลี่ยนแปลงปานกลาง
    } else {
        refreshRate = BASE_REFRESH_RATE;  // เปลี่ยนแปลงน้อย → อัปเดตช้า
    }

    // อัปเดต LCD เมื่อถึงเวลาหรือมีการเปลี่ยนแปลงมากกว่า 0.1
    unsigned long currentMillis = millis();
    if ((currentMillis - lastUpdate >= refreshRate) && (diff > 0.1)) {
        lastUpdate = currentMillis;
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Dist:");
        lcd.print(filteredDistance, 1);
        lcd.print("cm");

        // บรรทัดที่สอง: แสดง gauge bar พร้อม spinner animation
        lcd.setCursor(0, 1);
        float ratio = filteredDistance / maxDistance;
        if (ratio > 1.0) ratio = 1.0;
        int totalPixels = gaugeWidth * 5;
        int filledPixels = ratio * totalPixels;
        for (int i = 0; i < gaugeWidth; i++) {
            int cellFill = filledPixels - (i * 5);
            if (cellFill >= 5) {
                lcd.write(byte(5));
            } else if (cellFill > 0) {
                lcd.write(byte(cellFill));
            } else {
                lcd.write(byte(0));
            }
        }
        lcd.setCursor(gaugeWidth, 1);
        lcd.print(spinnerChars[spinnerIndex]);
        spinnerIndex = (spinnerIndex + 1) % 4;

        Serial.print("Measured: ");
        Serial.print(filteredDistance);
        Serial.println(" cm");

        lastDistance = filteredDistance;
    }
}

// ฟังก์ชันวัดระยะทางโดยใช้ค่าเฉลี่ยจากหลายการอ่าน
float measureDistance() {
    float sum = 0;
    int samples = 5; // อ่านค่า 5 ครั้งแล้วหาค่าเฉลี่ย
    for (int i = 0; i < samples; i++) {
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);
        long duration = pulseIn(ECHO_PIN, HIGH);
        float dist = duration * 0.034 / 2;
        
        // กรองค่าสุ่มผิดพลาด
        if (dist < 2 || dist > 200) {
            continue; // ข้ามค่าที่ผิดปกติ
        }

        sum += dist;
        delay(10);
    }
    return sum / samples; // ค่าเฉลี่ย
}

// ฟังก์ชันแสดง splash screen เท่สุดโต่ง
void showSplashScreen() {
    byte progress0[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    byte progress1[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
    byte progress2[8] = {0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18};
    byte progress3[8] = {0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C,0x1C};
    byte progress4[8] = {0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E,0x1E};
    byte progress5[8] = {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F};

    lcd.createChar(0, progress0);
    lcd.createChar(1, progress1);
    lcd.createChar(2, progress2);
    lcd.createChar(3, progress3);
    lcd.createChar(4, progress4);
    lcd.createChar(5, progress5);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Taksa-on Tape!");
    lcd.setCursor(0, 1);
    lcd.print("Booting Up...");
    delay(1500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Taksa-on Tape!");
    int barLength = 16;
    int totalSteps = barLength * 5;
    for (int progress = 0; progress <= totalSteps; progress++) {
        lcd.setCursor(0, 1);
        for (int i = 0; i < barLength; i++) {
            if (i < progress / 5) {
                lcd.write(byte(5));
            } else if (i == progress / 5) {
                int frac = progress % 5;
                lcd.write(byte(frac));
            } else {
                lcd.write(byte(0));
            }
        }
        delay(30);
    }
    delay(500);
    lcd.clear();
}
