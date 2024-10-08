#include <Arduino.h>
#include <TFT_eSPI.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#define DHT_PIN 18  // 18번 핀 사용
#define DHT_TYPE DHT22  // DHT 센서 타입
#define RELAY_PIN 17 // 릴레이 17번 핀 사용

const int pulseA = 44;
const int pulseB = 43;

volatile int lastEncoded = 0;
volatile long encoderValue = 0;

TFT_eSPI tft = TFT_eSPI();  // 디스플레이 설정 
DHT temperatureSensor(DHT_PIN, DHT_TYPE);  

unsigned long previousMillis = 0;  
int interval = 2000;  // 2초 간격

IRAM_ATTR void handleRotary() {
    
    int MSB = digitalRead(pulseA); //MSB = most significant bit
    int LSB = digitalRead(pulseB); //LSB = least significant bit
    int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
    int sum = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
    if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue ++;
    if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue --;
    lastEncoded = encoded; //store this value for next time
    if (encoderValue > 60) {
        encoderValue = 60;
    } else if (encoderValue < 0 ) {
        encoderValue = 0;
    }
}



void setup() {
    Serial.begin(115200);  
    
    pinMode(pulseA, INPUT_PULLUP);
    pinMode(pulseB, INPUT_PULLUP);
    attachInterrupt(pulseA, handleRotary, CHANGE);
    attachInterrupt(pulseB, handleRotary, CHANGE); 

    pinMode(RELAY_PIN, OUTPUT);  
    digitalWrite(RELAY_PIN, LOW);

    tft.init();  // TFT 디스플레이 초기화
    tft.setRotation(3);  // 화면 회전 설정
    tft.fillScreen(TFT_BLACK);  // 화면을 검은색으로 초기화

    temperatureSensor.begin();  // DHT 센서 초기화
}

void loop() {
    unsigned long currentMillis = millis();  // 현재 시간 가져오기

    // 지정된 간격마다 센서 데이터를 읽기
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;  // 시간 기록 업데이트

       
        float temperature = temperatureSensor.readTemperature();

        //디스플레이에 데이터 출력
        tft.fillScreen(TFT_BLACK);  
        tft.setCursor(10, 10);  
        tft.setTextColor(TFT_WHITE);  
        tft.setTextSize(3.5);  
        tft.println("Thermostat");
        tft.println("");
        tft.print("Temp: "); tft.println(temperature); 
        tft.println("");
        tft.print("Set : "); tft.print(encoderValue);

        // 시리얼 모니터에 데이터 출력
        Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" C ");
        Serial.print("Set : "); Serial.print(encoderValue);
        Serial.println(encoderValue);

        if ( temperature < encoderValue) {
           digitalWrite(RELAY_PIN, HIGH);
        } else {
           digitalWrite(RELAY_PIN, LOW);
        }
    }
}
