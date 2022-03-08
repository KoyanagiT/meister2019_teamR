#include <Adafruit_Fingerprint.h>
#include <Arduino.h>
//displayを使う時もう一個必要

// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
// uncomment this line:
// #define mySerial Serial1

//モーター関係
#define IN1 12
#define IN2 13

// For UNO and others without hardware serial, we must use software serial...
// pin #4 is IN from sensor (GREEN wire) : TX
// pin #5 is OUT from arduino  (WHITE wire) : RX
// comment these two lines if using hardware serial
SoftwareSerial mySerial(4, 5);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t sys=2;

uint8_t id;

uint8_t check=-1;

// rock関係
int userID;
int trig=0;
int mpin=3;
// rock関係

//displayを使う時
//TM1637 disp(CLK,DIO);

uint8_t readnumber(void) {
  uint8_t num = 0;
  
  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void setup()  
{
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  Serial.println("\n\n指紋センサーに接続します．．．");

  //displayを使う時
  //disp.set(7);
  //disp.init(D4056A);
  delay(5);
  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("指紋センサーを発見!");
  } else {
    Serial.println("ごめん．．．");
    while (1) { delay(1); }
  }

  //ドアしまったかチェック
  pinMode(mpin,INPUT);

  //モーター関係
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  finger.getTemplateCount();
  Serial.print("現在，"); Serial.print(finger.templateCount); Serial.println("個の指紋が登録されています．");
  Serial.print("enroll or test? : 1/2\n\n");
  //while((sys!=1)&&(sys!=2)){
  //  sys=readnumber();
  //}
  //常時開錠モード
  sys=2;
  if(sys==2) Serial.print("指置いて待ってな\n");
}

//モーター関係

void closeLock(){
  //モーターを回す信号(逆転します)
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  
  //なにかしらで鍵が閉まるまで回す(delayは仮)
  delay(2000);

  //ブレーキ
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);
  delay(1);

  //スタンバイ
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

void openLock(){
  //モーターを回す信号(正転します)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  
  //なにかしらで鍵が開くまで回すで(delayは仮)
  delay(2000);
  
  //ブレーキ
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, HIGH);

  //スタンバイ
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  //ドア閉まるの待ち
  while(trig==1){
    if(!digitalRead(mpin)) trig=0;
  }
  
  closeLock();
}

//モーター関係

void loop()                     // run over and over again
{
  //スタンバイ
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  //登録モードの時
  if(sys==1){
    Serial.println("Ready to enroll a fingerprint!");
    Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
    id = readnumber();
    if (id == 0) {// ID #0 not allowed, try again!
      return;
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);
  
    while (!  getFingerprintEnroll() );
  }

  //開錠モードの時
  else if(sys==2){
    getFingerprintIDez();
    //check=getFingerprintIDez();
    delay(50);            //don't ned to run this at full speed.
    if(!digitalRead(mpin)) trig=0;
    if(trig==1){
      openLock();
    }
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  Serial.print(finger.fingerID); Serial.print("さんを認証しました");
  Serial.print("　:精度#"); Serial.println(finger.confidence); 
  trig=1;
  userID=(int)finger.fingerID;
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;
  
  // found a match!
  Serial.print(finger.fingerID); Serial.print("さんを認証しました");
  Serial.print("　:精度#"); Serial.println(finger.confidence);
  trig=1;
  userID=(int)finger.fingerID;
  return finger.fingerID; 
}


// enroll

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);
  
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
  
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }   
}
