/*
  doublechest, 2025
*/

#include <Arduino.h>
#include <SettingsGyver.h>
#include <GyverMotor.h>

#include <ESP32Servo.h>
#include <ESP32Encoder.h>

const char* ssid = "ESP32";
const char* password = "12345678";

#define MOTOR_TEST 0

// FL
#define RX2 16 // Вперед
#define TX2 17

// BL
#define D18 18 // Вперед
#define D4 4

// BR
#define D21 21 // Вперед
#define D19 19

// FR
#define D23 23 // Вперед
#define D22 22

#define BLINK_PIN 2
#define BLINK_INTERVAL_MS 500

SettingsGyver sett("Mecanum");
sets::Pos joystick1;
sets::Pos joystick2;

#define MOTOR1_A RX2
#define MOTOR1_B TX2  // ШИМ!
#define MOTOR2_A D21
#define MOTOR2_B D19  // ШИМ!
#define MOTOR3_A D18
#define MOTOR3_B D4  // ШИМ!
#define MOTOR4_A D23
#define MOTOR4_B D22  // ШИМ!

#define D35 35
#define ENCODER_FL_PIN D35
ESP32Encoder encoderFL;
GMotor motorFL(DRIVER2WIRE, MOTOR1_A, MOTOR1_B, HIGH);
#define D5 5
#define ENCODER_BR_PIN D5
ESP32Encoder encoderBR;
GMotor motorBR(DRIVER2WIRE, MOTOR2_A, MOTOR2_B, HIGH);
#define D15 15
#define ENCODER_BL_PIN D15
ESP32Encoder encoderBL;
GMotor motorBL(DRIVER2WIRE, MOTOR3_A, MOTOR3_B, HIGH);
#define D34 34
#define ENCODER_FR_PIN D34
ESP32Encoder encoderFR;
GMotor motorFR(DRIVER2WIRE, MOTOR4_A, MOTOR4_B, HIGH);

#define D27 27
#define CAM_SERVO_PIN D27
Servo camServo;
int camServoAngle = 0;

int maxSpeed = 170;
bool engineEnabled = true;

void stopMotors() {
    motorFR.smoothTick(0);
    motorBR.smoothTick(0);
    motorFL.smoothTick(0);
    motorBL.smoothTick(0);
}

// Интерфейс

void build(sets::Builder& b) {
    if (b.beginRow("", sets::DivType::Line)) {
        b.Label("encoder_fl"_h, "FL");
        b.Label("encoder_fr"_h, "FR");
        b.Label("encoder_bl"_h, "BL");
        b.Label("encoder_br"_h, "BR");
        b.endRow();
    }

    b.HTML("", "<div style='text-align:center;'><img src='http://192.168.4.1:81/stream' style='width:100%; max-width:600px; border-radius:10px; border: 2px solid #333;'></div>");

    if (b.Slider("Camera Servo", 0, 180, 1, "", &camServoAngle))
    {
        camServo.write(camServoAngle);
    }

    if (b.Slider("Max Speed", 30, 255, 1, "", &maxSpeed))
        stopMotors();

    b.Switch("Двигатели", &engineEnabled);

    if (b.beginRow("", sets::DivType::Line)) {
        if (b.ButtonHold("↖"))
        {
            if (b.build.pressed()){
                joystick1.x = -255;
                joystick1.y = 255;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        if (b.ButtonHold("⬆"))
        {
            if (b.build.pressed()){
                joystick1.x = 0;
                joystick1.y = 255;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        if (b.ButtonHold("↗"))
        {
            if (b.build.pressed()){
                joystick1.x = 255;
                joystick1.y = 255;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        b.endRow();
    }

    if (b.beginRow("", sets::DivType::Line)) {
        if (b.ButtonHold("⬅"))
        {
            if (b.build.pressed()){
                joystick1.x = -255;
                joystick1.y = 0;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        if (b.ButtonHold("⟲"))
        {
            if (b.build.pressed()){
                joystick2.x = 255;
                joystick2.y = 0;
            }
            else {
                joystick2.x = 0;
                joystick2.y = 0;
            }
        };
        if (b.ButtonHold("⟳"))
        {
            if (b.build.pressed()){
                joystick2.x = -255;
                joystick2.y = 0;
            }
            else {
                joystick2.x = 0;
                joystick2.y = 0;
            }
        };
        if (b.ButtonHold("➡"))
        {
            if (b.build.pressed()){
                joystick1.x = 255;
                joystick1.y = 0;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        b.endRow();
    }

    if (b.beginRow("", sets::DivType::Line)) {
        if (b.ButtonHold("↙"))
        {
            if (b.build.pressed()){
                joystick1.x = -255;
                joystick1.y = -255;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        if (b.ButtonHold("⬇"))
        {
            if (b.build.pressed()){
                joystick1.x = 0;
                joystick1.y = -255;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        if (b.ButtonHold("↘"))
        {
            if (b.build.pressed()){
                joystick1.x = 255;
                joystick1.y = -255;
            }
            else {
                joystick1.x = 0;
                joystick1.y = 0;
            }
        };
        b.endRow();
    }

    if (b.beginRow("", sets::DivType::Line)) {
        b.Joystick(joystick1, true);
        b.Joystick(joystick2, true);
        b.endRow();
    }
}

void update(sets::Updater& upd) {
    upd.update("encoder_fl"_h, encoderFL.getCount()/20);
    upd.update("encoder_fr"_h, encoderFR.getCount()/20);
    upd.update("encoder_bl"_h, encoderBL.getCount()/20);
    upd.update("encoder_br"_h, encoderBR.getCount()/20);
}


void driveRobot() {
    int valLX = map(joystick1.x, -255, 255, -maxSpeed, maxSpeed);
    int valLY = map(joystick1.y, 255, -255, -maxSpeed, maxSpeed);
    int valRX = map(joystick2.x, -255, 255, -maxSpeed, maxSpeed);
    int valRY = map(joystick2.y, 255, -255, -maxSpeed, maxSpeed);

    int dutyFR = valLY + valLX + valRY - valRX;
    int dutyFL = valLY - valLX + valRY + valRX;
    int dutyBR = valLY - valLX + valRY - valRX;
    int dutyBL = valLY + valLX + valRY + valRX;

    motorFR.smoothTick(dutyFR);
    motorBR.smoothTick(dutyBR);
    motorFL.smoothTick(dutyFL);
    motorBL.smoothTick(dutyBL);
}

void setup() {
    Serial.begin(115200);

    pinMode(BLINK_PIN, OUTPUT);

    digitalWrite(BLINK_PIN, HIGH);
    delay(1000);
    digitalWrite(BLINK_PIN, LOW);

    // 1. Устанавливаем режим клиента (Station)
    WiFi.mode(WIFI_STA); 

    // 2. Начинаем процесс подключения
    WiFi.begin(ssid, password);

    Serial.println("Подключение к WiFi...");

    // 3. Ждем завершения подключения
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nПодключено!");
    Serial.print("IP-адрес: ");
    Serial.println(WiFi.localIP()); // Выводим полученный адрес

    digitalWrite(BLINK_PIN, HIGH);
    delay(100);
    digitalWrite(BLINK_PIN, LOW);
    delay(100);
    digitalWrite(BLINK_PIN, HIGH);
    delay(100);
    digitalWrite(BLINK_PIN, LOW);

    camServo.attach(CAM_SERVO_PIN); 
    camServo.write(camServoAngle);

    // WiFi.softAP(ssid, password);

    sett.begin(false, "mecanum");
    sett.onBuild(build);
    sett.onUpdate(update);
    sett.setPass("123");

  #if (MOTOR_TEST == 1)
    Serial.println("front left");
    motorFL.run(FORWARD, 255);
    delay(3000);
    motorFL.run(STOP);
    delay(1000);
    Serial.println("front right");
    motorFR.run(FORWARD, 255);
    delay(3000);
    motorFR.run(STOP);
    delay(1000);
    Serial.println("back left");
    motorBL.run(FORWARD, 255);
    delay(3000);
    motorBL.run(STOP);
    delay(1000);
    Serial.println("back right");
    motorBR.run(FORWARD, 255);
    delay(3000);
    motorBR.run(STOP);
  #endif
    // минимальный сигнал на мотор
    motorFR.setMinDuty(30);
    motorBR.setMinDuty(30);
    motorFL.setMinDuty(30);
    motorBL.setMinDuty(30);

    // режим мотора в АВТО
    motorFR.setMode(AUTO);
    motorBR.setMode(AUTO);
    motorFL.setMode(AUTO);
    motorBL.setMode(AUTO);

    // скорость плавности
    motorFR.setSmoothSpeed(60);
    motorBR.setSmoothSpeed(60);
    motorFL.setSmoothSpeed(60);
    motorBL.setSmoothSpeed(60);

    encoderFL.attachSingleEdge(ENCODER_FL_PIN, -1); 
    encoderFL.setCount(0);
    encoderBL.attachSingleEdge(ENCODER_BL_PIN, -1); 
    encoderBL.setCount(0);
    encoderBR.attachSingleEdge(ENCODER_BR_PIN, -1); 
    encoderBR.setCount(0);
    encoderFR.attachSingleEdge(ENCODER_FR_PIN, -1); 
    encoderFR.setCount(0);
}

void loop() {
    sett.tick();

    static unsigned long _lastBlink = 0;
    static bool _ledState = false;
    if (millis() - _lastBlink >= BLINK_INTERVAL_MS) {
        _lastBlink = millis();
        _ledState = !_ledState;
        digitalWrite(BLINK_PIN, _ledState ? HIGH : LOW);
    }

    if (engineEnabled)
        driveRobot();
    else
        stopMotors();
}