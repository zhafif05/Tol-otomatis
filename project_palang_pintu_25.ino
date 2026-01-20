#include <EEPROM.h>     
#include <SPI.h>       
#include <MFRC522.h>  

#include <Servo.h>
Servo myservo;

#define buzzer 5
#define led 7

int infrared = 8; 
int servo_speed = 10;
int speed = 0;
int infra_read = 0;
int posisiSekarang = myservo.read();

boolean match = false;          
boolean programMode = false;  
boolean replaceMaster = false;

uint8_t successRead;    

byte storedCard[4];   
byte readCard[4];   
byte masterCard[4];   

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN); 

//konfigurasi pin arduino
void setup() { 
  myservo.attach(6);  
  pinMode(infrared,INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW); 
  digitalWrite(buzzer, LOW); 
  Serial.begin(9600);  
  SPI.begin();          
  mfrc522.PCD_Init();   
  
  Serial.println(F("Palang Pintu Otomatis dengan RFID Multicard"));   
  ShowReaderDetails();  

  if (EEPROM.read(1) != 143) {
    Serial.println(F("Tidak Ada Master Card Tersimpan"));
    Serial.println(F("Scan Kartu RFID Untuk Dijadikan Sebagai Master Card"));
    do {
      successRead = getID();            
      digitalWrite(buzzer, HIGH);    
      delay(200);
      digitalWrite(buzzer, LOW);
      delay(200);
    }
    while (!successRead);                  
    for ( uint8_t j = 0; j < 4; j++ ) {        
      EEPROM.write( 2 + j, readCard[j] );  
    }
    EEPROM.write(1, 143);                  
    Serial.println(F("Master Card Tersimpan"));
  }
  Serial.println(F("-------------------"));
  Serial.println(F("Kartu RFID Master Card"));
  for ( uint8_t i = 0; i < 4; i++ ) {         
    masterCard[i] = EEPROM.read(2 + i);    
    Serial.print(masterCard[i], HEX);
  }
  Serial.println("");
  Serial.println(F("-------------------"));
  Serial.println(F("Sudah Siap"));
  Serial.println(F("Menunggu Kartu RFID Terbaca"));
  cycleBuzzer();    

}

void loop () {   
  do {
    sensor ();
    successRead = getID();  
    if (programMode) {
      cycleBuzzer();              
    }
    else {
      normalModeOn();     
    }
  }
  while (!successRead);   
  if (programMode) {
    if ( isMaster(readCard) ) { 
      Serial.println(F("Master Card Terbaca"));
      Serial.println(F("Keluar Program Mode"));
      Serial.println(F("-----------------------------"));
      programMode = false;
      return;
    }
    else {
      if ( findID(readCard) ) { 
        Serial.println(F("Hapus Kartu RFID"));
        deleteID(readCard);
        Serial.println("-----------------------------");
        Serial.println(F("Scan Kartu RFID Tambahkan Atau Hapus Ke EEPROM"));
      }
      else {                    
        Serial.println(F("Tambahkan Kartu RFID"));
        writeID(readCard);
        Serial.println(F("-----------------------------"));
        Serial.println(F("Scan Kartu RFID Tambahkan Atau Hapus Ke EEPROM"));
      }
    }
  }
  else {
    if ( isMaster(readCard)) {   
      programMode = true;
      Serial.println(F("Masuk Ke Program Mode"));
      uint8_t count = EEPROM.read(0);   
      Serial.print(F("Ada "));     
      Serial.print(count);
      Serial.print(F(" Data Di EEPROM"));
      Serial.println("");
      Serial.println(F("Scan Kartu RFID Tambahkan Atau Hapus Ke EEPROM"));
      Serial.println(F("Scan Master Card Lagi Untuk Keluar dari Program Mode"));
      Serial.println(F("-----------------------------"));
    }
    else {
      if ( findID(readCard)) {
        Serial.println(F("Selamat Datang, Anda Bisa Masuk"));
          diterima(5000);   
      }
      else {      
        Serial.println(F("Anda Tidak Bisa Masuk"));
        ditolak();
      }
    }
  }
 
}
// program sensor infrared
void sensor(){
  infra_read=digitalRead(infrared);
  int posisiSekarang = myservo.read();
  if (infra_read == LOW && posisiSekarang == 180){
    delay(2500);
        for (speed = 180; speed >= 90; speed -= 1) {
myservo.write(speed);
delay(30);
  }
  }   
}

//program jika kartu rfid terdeteksi
void diterima ( uint16_t setDelay) {
  digitalWrite(buzzer, LOW);   
  delay(100);
  digitalWrite(buzzer, HIGH);  
  delay(200);
  digitalWrite(buzzer, LOW);   
  delay(100);
  digitalWrite(buzzer, HIGH); 
  delay(200);
  for (speed = 90; speed <= 180; speed += 1) {
myservo.write(speed);
delay(servo_speed);}
}

//program jika kartu rfid tidak terdeteksi
void ditolak() {
  digitalWrite(buzzer, HIGH);   
  delay(500);
  digitalWrite(buzzer, LOW);
  delay(100);
}

//program mendapatkan ID Kartu RFID
uint8_t getID() { 
  if ( ! mfrc522.PICC_IsNewCardPresent()) { 
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   
    return 0;
  }
  Serial.println(F("Scan ID Kartu RFID:"));
  for ( uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");
  mfrc522.PICC_HaltA(); 
  return 1;
}

void ShowReaderDetails() {
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown),probably a chinese clone?"));
  Serial.println("");
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    Serial.println(F("SYSTEM HALTED: Check connections."));
    digitalWrite(buzzer, LOW);  
    while (true);
  }
}

//indikator masuk ke program mode
void cycleBuzzer() {
  digitalWrite(led, LOW);   
  delay(100);
  digitalWrite(led, HIGH); 
  delay(100);
delay(200);
}

//indikator masuk ke normal mode
void normalModeOn () {
  digitalWrite(buzzer, LOW);
}

//program membaca ID Kartu RFID dari EEPROM
void readID( uint8_t number ) {
  uint8_t start = (number * 4 ) + 2;    
  for ( uint8_t i = 0; i < 4; i++ ) {     
    storedCard[i] = EEPROM.read(start + i);  
  }
}

//program menambahkan ID Kartu RFID ke EEPROM
void writeID( byte a[] ) {
  if ( !findID( a ) ) {     
    uint8_t num = EEPROM.read(0);     
    uint8_t start = ( num * 4 ) + 6;  
    num++;                
    EEPROM.write( 0, num );    
    for ( uint8_t j = 0; j < 4; j++ ) {   
      EEPROM.write( start + j, a[j] );  
    }
    successWrite();
    Serial.println(F("Berhasil Menambahkan ID Kartu RFID ke EEPROM"));
  }
  else {
    failedWrite();
    Serial.println(F("Gagal Memasukkan ID Kartu RFID ke EEPROM"));
  }
}

void deleteID( byte a[] ) {
  if ( !findID( a ) ) {     
    failedWrite();     
    Serial.println(F("Gagal Memasukkan ID Kartu RFID ke EEPROM"));
  }
  else {
    uint8_t num = EEPROM.read(0);  
    uint8_t slot;       
    uint8_t start;      
    uint8_t looping;    
    uint8_t j;
    uint8_t count = EEPROM.read(0); 
    slot = findIDSLOT( a );   
    start = (slot * 4) + 2;
    looping = ((num - slot) * 4);
    num--;      
    EEPROM.write( 0, num );   
    for ( j = 0; j < looping; j++ ) {         
      EEPROM.write( start + j, EEPROM.read(start + 4 + j));   
    }
    for ( uint8_t k = 0; k < 4; k++ ) {        
      EEPROM.write( start + j + k, 0);
    }
    successDelete();
    Serial.println(F("Berhasil Menghapus ID Kartu RFID dari EEPROM"));
  }
}

boolean checkTwo ( byte a[], byte b[] ) {
  if ( a[0] != 0 )      
    match = true;       
  for ( uint8_t k = 0; k < 4; k++ ) {   
    if ( a[k] != b[k] )     
      match = false;
  }
  if ( match ) {      
    return true;      
  }
  else  {
    return false;       
  }
}

uint8_t findIDSLOT( byte find[] ) {
  uint8_t count = EEPROM.read(0);       
  for ( uint8_t i = 1; i <= count; i++ ) {    
    readID(i);                
    if ( checkTwo( find, storedCard ) ) {   
     
      return i;         
      break;          
    }
  }
}
//program temukan id kartu RFID dari EEPROM
boolean findID( byte find[] ) {
  uint8_t count = EEPROM.read(0);     
  for ( uint8_t i = 1; i <= count; i++ ) {    
    readID(i);          
    if ( checkTwo( find, storedCard ) ) {   
      return true;
      break;  
    }
    else {    
    }
  }
  return false;
}
// program indikator jika berhasil dikirim ke EEPROM
void successWrite() {
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);  
  delay(100);
    digitalWrite(buzzer, HIGH);
  delay(100);
    digitalWrite(buzzer, LOW);   
  delay(100);
}
// program indikator jika gagal dikirim ke EEPROM
void failedWrite() {
  digitalWrite(buzzer, LOW);   
}
// program indikator jika berhasil dihapus dari EEPROM
void successDelete() {
    digitalWrite(buzzer, HIGH);
  delay(500);
  digitalWrite(buzzer, LOW);  
  delay(100);
      digitalWrite(buzzer, HIGH);
  delay(500);
    digitalWrite(buzzer, LOW);   
  delay(100);
}
//program untuk check apakah kartu RFID sebagai Master Card
boolean isMaster( byte test[] ) {
  if ( checkTwo( test, masterCard ) )
    return true;
  else
    return false;
}
