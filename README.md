# ระบบวัดระยะอัจฉริยะด้วย HC-SR04 และจอ LCD I2C พร้อมการกรองสัญญาณ

สวัสดีครับเพื่อน ๆ นักพัฒนา Arduino ทุกคน! วันนี้เรามาดูกันว่าเราจะสร้างระบบวัดระยะทางสุดเจ๋งด้วย Arduino, เซ็นเซอร์ HC-SR04 และจอ LCD I2C ได้อย่างไร โดยเพิ่มความแม่นยำด้วยการกรองสัญญาณแบบ Real-time กันเลย! 🚀

เว็บบทความ : https://geworn.cloud/blog/rabbwadraya-acchchriyadwy-hc-sr04-aelacch-lcd-i2c-phr-mkaarkr-ngsayyaan/

## 🌟 ทำไมต้องใช้ระบบวัดระยะอัจฉริยะ?

ในยุคที่เทคโนโลยี IoT กำลังมาแรง การวัดระยะทางที่แม่นยำและรวดเร็วเป็นสิ่งสำคัญมาก ๆ ไม่ว่าจะเป็นการทำหุ่นยนต์หลบสิ่งกีดขวาง, ระบบเตือนภัยอัจฉริยะ หรือแม้แต่การทำเครื่องวัดส่วนสูงดิจิทัล ก็ล้วนต้องการระบบวัดระยะที่เชื่อถือได้ทั้งนั้น

แต่ปัญหาคือ... การวัดระยะด้วยเซ็นเซอร์อัลตราโซนิกอย่าง HC-SR04 มักจะมีสัญญาณรบกวนเยอะ ทำให้ค่าที่วัดได้กระโดดไปมา 😵‍💫 แล้วเราจะแก้ปัญหานี้ยังไงดีล่ะ?

## 💡 วิธีการแก้ปัญหา: กรองสัญญาณแบบ Real-time!

เราจะใช้เทคนิคการกรองสัญญาณที่เรียกว่า Exponential Moving Average (EMA) ซึ่งเป็นวิธีที่ง่ายแต่มีประสิทธิภาพสูงในการลดสัญญาณรบกวน โดยไม่ทำให้ระบบตอบสนองช้าเกินไป

### 🔍 หลักการทำงานของ EMA

EMA จะคำนวณค่าเฉลี่ยแบบถ่วงน้ำหนัก โดยให้น้ำหนักกับค่าล่าสุดมากกว่าค่าเก่า ๆ ทำให้ระบบตอบสนองต่อการเปลี่ยนแปลงได้เร็ว แต่ก็ยังคงความเรียบของข้อมูลไว้ได้

สูตรคำนวณ EMA คือ:

`EMA = α * ค่าปัจจุบัน + (1 - α) * EMA ก่อนหน้า`

โดย α (alpha) คือค่าระหว่าง 0 ถึง 1 ที่กำหนดว่าจะให้น้ำหนักกับค่าใหม่มากแค่ไหน

ในโค้ดของเรา เราใช้ `ALPHA = 0.1` ซึ่งให้การกรองที่นุ่มนวลพอดี ไม่ช้าเกินไปและไม่ไวเกินไป

## 🛠️ มาดูโค้ดกันเลย!

```cpp
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define TRIG_PIN 9
#define ECHO_PIN 10
LiquidCrystal_I2C lcd(0x27, 16, 2);

float distance = 0.0;
float lastDistance = 0.0;
float filteredDistance = 0.0;
unsigned long lastUpdate = 0;
#define ALPHA 0.1

#define BASE_REFRESH_RATE 1000
#define FAST_REFRESH_RATE 100
unsigned long refreshRate = BASE_REFRESH_RATE;

int spinnerIndex = 0;
char spinnerChars[4] = {'|', '/', '-', '\\'};

const float maxDistance = 200.0;
const int gaugeWidth = 15;

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    showSplashScreen();
}

void loop() {
    delay(100);
    distance = measureDistance();
    filteredDistance = (ALPHA * distance) + ((1 - ALPHA) * filteredDistance);
    
    float diff = abs(lastDistance - filteredDistance);
    if (diff > 50) {
        refreshRate = FAST_REFRESH_RATE;
    } else if (diff > 5) {
        refreshRate = 300;
    } else {
        refreshRate = BASE_REFRESH_RATE;
    }
    
    unsigned long currentMillis = millis();
    if ((currentMillis - lastUpdate >= refreshRate) && (diff > 0.1)) {
        lastUpdate = currentMillis;
        updateLCD();
        lastDistance = filteredDistance;
    }
}

// ฟังก์ชันอื่น ๆ เช่น measureDistance(), updateLCD(), showSplashScreen() ตามโค้ดต้นฉบับ
```

### 🔬 การทำงานของโค้ด

*   **การวัดระยะ:** ใช้ฟังก์ชัน `measureDistance()` วัดระยะหลาย ๆ ครั้งแล้วหาค่าเฉลี่ย
*   **การกรองสัญญาณ:** ใช้ EMA กรองค่าที่วัดได้ให้เรียบขึ้น
*   **การปรับ Refresh Rate:** ปรับความถี่ในการอัพเดทจอ LCD ตามการเปลี่ยนแปลงของระยะทาง
*   **การแสดงผล:** ใช้ Gauge Bar และ Spinner Animation เพื่อให้การแสดงผลน่าสนใจยิ่งขึ้น

## 🎨 การแสดงผลสุดเจ๋ง!

เราไม่ได้แค่แสดงตัวเลขธรรมดา ๆ นะ แต่เรามี:

*   **Gauge Bar:** แสดงระยะทางเป็นแถบกราฟิก ทำให้เห็นภาพได้ง่ายขึ้น
*   **Spinner Animation:** แสดงว่าระบบยังทำงานอยู่ ไม่ได้ค้างหรือหยุดทำงาน
*   **Dynamic Refresh Rate:** ปรับความเร็วในการอัพเดทจอตามการเปลี่ยนแปลงของระยะทาง

## 🚀 ประโยชน์ที่ได้รับ

*   **ความแม่นยำสูง:** ด้วยการกรองสัญญาณ EMA ทำให้ค่าที่วัดได้มีความเสถียรมากขึ้น
*   **การแสดงผลที่น่าสนใจ:** Gauge Bar และ Spinner ทำให้ผู้ใช้เข้าใจข้อมูลได้ง่ายขึ้น
*   **ประหยัดพลังงาน:** การปรับ Refresh Rate ช่วยลดการทำงานของ LCD เมื่อไม่จำเป็น
*   **เหมาะกับงานหลากหลาย:** ใช้ได้ทั้งในโปรเจค IoT, หุ่นยนต์ หรือระบบอัตโนมัติต่าง ๆ

## 💡 ไอเดียการประยุกต์ใช้

*   **หุ่นยนต์หลบสิ่งกีดขวาง:** ใช้วัดระยะห่างจากสิ่งกีดขวางได้อย่างแม่นยำ
*   **ระบบเตือนภัยอัจฉริยะ:** ตรวจจับการเคลื่อนไหวหรือการบุกรุก
*   **เครื่องวัดส่วนสูงดิจิทัล:** สร้างเครื่องวัดส่วนสูงที่แม่นยำและอ่านค่าง่าย
*   **ระบบจอดรถอัจฉริยะ:** ช่วยในการจอดรถโดยบอกระยะห่างจากกำแพงหรือรถคันอื่น

## 🏆 สรุป

ด้วยการผสมผสานระหว่างเซ็นเซอร์ HC-SR04, จอ LCD I2C และเทคนิคการกรองสัญญาณ EMA เราสามารถสร้างระบบวัดระยะที่ทั้งแม่นยำ น่าใช้ และยืดหยุ่นสูง เหมาะสำหรับนำไปต่อยอดในโปรเจคต่าง ๆ ได้อย่างหลากหลาย

ลองเอาไปทดลองใช้กันดูนะครับ แล้วอย่าลืมแชร์ไอเดียเจ๋ง ๆ ของคุณในคอมเมนต์ด้วยล่ะ! 😉

## 📢 Call to Action

ชอบบทความนี้ไหมครับ? ถ้าใช่ อย่าลืมแชร์ให้เพื่อน ๆ ที่สนใจ Arduino ด้วยนะครับ! และถ้าคุณได้ลองทำตามแล้ว อย่าลืมแท็ก `#ArduinoThailand` `#DistanceSensorTH` บนโซเชียลมีเดียด้วยล่ะ เราอยากเห็นผลงานของคุณมาก ๆ เลย! 🌟

## แหล่งข้อมูล

*   "Arduino - Home." [Arduino](https://www.arduino.cc/)
*   "HC-SR04 Ultrasonic Sensor." [Components101](https://components101.com/sensors/ultrasonic-sensor-working-pinout-datasheet)
*   "I2C LCD with Arduino." [Last Minute Engineers](https://lastminuteengineers.com/i2c-lcd-arduino-tutorial/)
*   "Exponential Moving Average." [Investopedia](https://www.investopedia.com/terms/e/ema.asp)

## แฮชแท็ก

`#Arduino` `#HCSR04` `#UltrasonicSensor` `#DistanceMeasurement` `#LCDI2C` `#ExponentialMovingAverage` `#RealTimeDataFiltering` `#GaugeBar` `#DynamicRefreshRate` `#SpinnerAnimation` `#EmbeddedSystems` `#SerialMonitor` `#IoTApplications` `#ArduinoThailand` `#DistanceSensorTH`
