#include <SPI.h>
#include <Wire.h>
#include <RFID.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include <Servo.h> 

Servo myservo1;  

#define sda 10
#define rst 9

int IR1 = 2; 
int IR2 = 3; 

int flag1 = 0;
int flag2 = 0;

RFID rfid(sda, rst);

byte validID[] = {195, 37, 76, 254, 84}; 

int saldo = 10000;
int tarif = 5000; 

void setup() {
  Serial.begin(9600);
  SPI.begin(); 
  rfid.init();

  lcd.begin();
  lcd.backlight();

  // Pesan awal di LCD
  lcd.setCursor(1,0);
  lcd.print("Project Palang ");
  lcd.setCursor(1,1);
  lcd.print("Pintu TOL");
  delay(3000);
  lcd.clear();

  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  
  myservo1.attach(7);
  myservo1.write(100);
}

void loop() { 
  if (digitalRead(IR1) == LOW && flag1 == 0) {
    flag1 = 1; // Set flag untuk mendeteksi mobil
    myservo1.write(100); // Palang tetap tertutup
    
    // Minta pengemudi menempelkan kartu
    lcd.setCursor(0,0);
    lcd.print("Tempelkan kartu");
    delay(3000);
    lcd.clear();
  }

  if (flag1 == 1 && cekRFID()) {  
    if (saldo >= tarif) {
      bayarTol();
      myservo1.write(0);
      lcd.setCursor(0, 0);
      lcd.print("   Terimakasih");
      delay(3000);
      lcd.clear();
      flag1 = 0; 
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Saldo tidak cukup");
      delay(3000);
      lcd.clear();
      flag1 = 0; // Reset flag
    }
  }
  else {
      lcd.setCursor(0, 0);
      lcd.print("Silakan");
      delay(1000);
      lcd.clear();
  }

  if (digitalRead(IR2) == LOW && flag2 == 0) {
          lcd.setCursor(0, 0);
      lcd.print("Selamat Jalan");
    flag2 = 1;
    delay(2000); 
    myservo1.write(100); 
    flag2 = 0; 
  }
}

bool cekRFID() {
  if (rfid.isCard()) {
    if (rfid.readCardSerial()) {
      bool isValid = true;
      
      for (int i = 0; i < 5; i++) {
        if (rfid.serNum[i] != validID[i]) {
          isValid = false;
          break;
        }
      }
      return isValid;
    }
  }
  return false;
}

void bayarTol() {
  saldo -= tarif; 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pembayaran sukses");
  lcd.setCursor(0, 1);
  lcd.print("Sisa saldo: ");
  lcd.print(saldo);
  delay(3000);
  lcd.clear();
}
