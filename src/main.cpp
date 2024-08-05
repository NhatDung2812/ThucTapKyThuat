#include <Arduino.h>

#include <Keypad.h>
#include <EEPROM.h>

#include <SPI.h>
#include <MFRC522.h>

#include <ESP32Servo.h>

#define SS_PIN 5
#define RST_PIN 15
#define PIN_SG90 4


unsigned char index_t = 0;
unsigned char error_in = 0;

// init keypad
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns

char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {13, 12, 14, 27}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


char str9[] = "Hello";
char str10[] = "Hello";

char passwordDefault[5] = {'1', '2', '3', '4', '5'};
char passwordDefault2[5] = {'A', 'B', 'C', 'D', '#'};
char passwordDefault3[5] = {'0', '0', '0', '0', '0'}; //Option Change Password
char passwordDefault4[5] = {'#', '#', '#', '#', '#'}; 


char inputKeyboard[5];
char inputKeyboard2[5];
int indec = 0;
int indec2 = 0;
int indec3 = 0;
int state = 0;
int option = 0;
int change = 0;


//rfid

int checkRFID = 0;

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
byte nuidPICC[4] = {0x63, 0xC9, 0x6F, 0x2F};
byte nuidPICC_1[4] = {0x60, 0x31, 0x48, 0x12};
Servo sg90;

void printHex(byte *buffer, byte bufferSize)
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void printDec(byte *buffer, byte bufferSize)
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], DEC);
    }
}

void motor()
{
  for (int pos = 0; pos <= 90; pos += 1) {
    sg90.write(pos);
    delay(10);
  }

  delay(3000);

  // Rotation from 180° to 0
  for (int pos = 90; pos >= 0; pos -= 1) {
    sg90.write(pos);
    delay(10);
  }
}




char a[5];

void writeEpprom(char data[]) {
  unsigned char i = 0;
  for (i = 0; i < 5; i++) {
  EEPROM.write(i, data[i]);
  }
  EEPROM.commit();
}

void readEpprom() {
  unsigned char i = 0;
  for (i = 0; i < 5; i++) {
    a[i]  = EEPROM.read(i);
    // Serial.print(a[i]);
    // delay(1500);
  }
}

bool compare(char data1[], char data2[]) {
  unsigned char i = 0;
  for (i = 0; i < 5; i++) {
    if (data1[i] != data2[i]) {
      // Serial.println(" False ");
      return false;
    }
  }
    // Serial.print(" True");
    return true;
}


void setup () {
  Serial.begin(9600);
  EEPROM.begin(46);

  SPI.begin();
  rfid.PCD_Init();
  sg90.setPeriodHertz(50);         
  sg90.attach(PIN_SG90, 500, 2400);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  // Serial.println();
  // Serial.print("Vui lòng quét thẻ của bạn");

  // Serial.println(F("This code scan the MIFARE Classsic NUID."));
  // Serial.print(F("Using the following key:"));
  // printHex(key.keyByte, MFRC522::MF_KEY_SIZE);


  sg90.setPeriodHertz(50);          // PWM frequency for SG90
  sg90.attach(PIN_SG90, 500, 2400);
  sg90.write(0);
}



void loop() {

  // char myData[5] = {'1', '2', '3', '4', '5'};
  // writeEpprom(myData);

  if (checkRFID == 0) {
    Serial.println();
    Serial.println("Vui lòng quét thẻ của bạn");
    checkRFID = 1;
  }

  if (rfid.PICC_IsNewCardPresent() == true && rfid.PICC_ReadCardSerial() == true && checkRFID == 1) {

    // Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    // Serial.println(rfid.PICC_GetTypeName(piccType));
    
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K)
    {
      Serial.println(F("Your tag is not of type MIFARE Classic."));
      return;
    }

    if (rfid.uid.uidByte[0] == nuidPICC[0] &&
        rfid.uid.uidByte[1] == nuidPICC[1] &&
        rfid.uid.uidByte[2] == nuidPICC[2] &&
        rfid.uid.uidByte[3] == nuidPICC[3])
    {   
      checkRFID = 2;

      // Serial.println(F("Open the door."));

      // Serial.println(F("The NUID tag is:"));
      // Serial.print(F("In hex: "));
      // printHex(rfid.uid.uidByte, rfid.uid.size);
      // Serial.println();
      // Serial.print(F("In dec: "));
      // printDec(rfid.uid.uidByte, rfid.uid.size);
      // Serial.println(); 
      Serial.println("Quét thẻ thành công");
    }
    else {
      Serial.println("Thẻ không chính xác");
      checkRFID = 0;
    }
        
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1(); 
  } else if (checkRFID == 2) {

    if (state == 0) {
    Serial.println();
    Serial.print("Vui lòng nhập mật khẩu");
    state = 1;
    }

    char key = keypad.getKey(); 

    if (key) {

      if (indec < 4 && state == 1) {

        Serial.println();
        Serial.print(key);
        inputKeyboard[indec] = key;
        indec++;
        Serial.print("   State_1");
      } else if (indec == 4 && state == 1) {

        Serial.println();
        Serial.print(key);
        inputKeyboard[indec] = key;
        indec++;
        Serial.print("   State_1");

          // Đã nhập đủ 5 ký tự, thực hiện các xử lý tiếp theo
        Serial.println();
        Serial.print("Mật khẩu đã nhập: ");
        Serial.print(inputKeyboard);

        readEpprom();

      if (compare(inputKeyboard, a) == true) {
        Serial.println();
        Serial.println("Mật khẩu đúng");
        state = 2;
        Serial.println();
        Serial.print("Vui lòng nhập lệnh");

      } else {
        Serial.println();
        Serial.println("Nhập sai mật khẩu");
        indec = 0;
        state = 0;
      }
      // indec = 0; // Reset chỉ số
      } else if (indec == 5 && state == 2) {

        if (indec == 5 && state == 2) {
          Serial.println();
          Serial.print(key);
          Serial.print("   ");
          inputKeyboard[indec2] = key;      //inputKeyboard2  //indec2
          indec2++;
          Serial.print("State_2");
        }

        if (indec2 == 5 && option == 0) {

          Serial.println();
          Serial.print("Lệnh đã nhập: ");
          Serial.println(inputKeyboard);

        
          if (compare(inputKeyboard, passwordDefault2) == true) {
            Serial.println();
            Serial.println("MỞ CỬA THÀNH CÔNG");


            motor();
            //...

            // Reset chỉ số
            indec = 0;
            indec2 = 0;
            state = 0;
            checkRFID = 0;


          }  else if (compare(inputKeyboard, passwordDefault3) == true) {

            option = 1;
            Serial.println();
            Serial.print("Nhập mật khẩu mới ");
            indec = 5;
            indec2 = 0;
            state = 2;

          } else if (compare(inputKeyboard, passwordDefault4) == true) {
            Serial.println();
            Serial.println("Đọc MK");
            readEpprom();
            indec = 0;
            indec2 = 0;
            state = 0;
            checkRFID = 0;
          }
        
        
          else {
            indec2 = 0;
            state = 2;
            Serial.println();
            Serial.println("SAI LỆNH");
            Serial.println();
            Serial.print("Vui lòng nhập lệnh");

          }
        } else if (indec2 == 5 && option == 1) {
        
          writeEpprom(inputKeyboard);
          Serial.println();
          Serial.println();
          Serial.print("Nhập lại mật khẩu");
          // indec = 0;
          // indec2 = 0;
          // state = 0;
          // option = 0;

          indec = 5;
          indec2 = 0;
          state = 2;
          option = 2;
        } else if (indec2 == 5 && option == 2) {
          readEpprom();

          if (compare(inputKeyboard, a) == true) {
            Serial.println();
            Serial.println("Mật khẩu xác nhận chính xác");
            Serial.println("Đổi mật khẩu thành công");
            indec = 0;
            indec2 = 0;
            state = 0;
            option = 0;
            checkRFID = 0;
          } else {

            Serial.println();
            Serial.println("Sai mật khẩu xác nhận");
            Serial.println();
            Serial.print("Nhập lại mật khẩu xác nhận");
            indec = 5;
            indec2 = 0;
            state = 2;
            option = 2;
          }
        }
      }
    }
  }


  // if (state == 0) {
  //   Serial.println();
  //   Serial.print("Vui lòng nhập mật khẩu");
  //   state = 1;
  // }

  // char key = keypad.getKey(); 

  // if (key) {
  //   if (indec < 4 && state == 1) {
  //   Serial.println();
  //   Serial.print(key);
  //   inputKeyboard[indec] = key;
  //   indec++;
  //   Serial.print("   State_1");

  // } else if (indec == 4 && state == 1) {

  //   Serial.println();
  //   Serial.print(key);
  //   inputKeyboard[indec] = key;
  //   indec++;
  //   Serial.print("   State_1");

  //     // Đã nhập đủ 5 ký tự, thực hiện các xử lý tiếp theo
  //   Serial.println();
  //   Serial.print("Mật khẩu đã nhập: ");
  //   Serial.print(inputKeyboard);

  //   readEpprom();

  //   if (compare(inputKeyboard, a) == true) {
  //     Serial.println();
  //     Serial.println("Mật khẩu đúng");
  //     state = 2;
  //     Serial.println();
  //     Serial.print("Vui lòng nhập lệnh");

  //   } else {
  //     Serial.println();
  //     Serial.println("Nhập sai mật khẩu");
  //     indec = 0;
  //     state = 0;
  //   }
  //   // indec = 0; // Reset chỉ số
  // } else if (indec == 5 && state == 2) {

  //   if (indec == 5 && state == 2) {
  //     Serial.println();
  //     Serial.print(key);
  //     Serial.print("   ");
  //     inputKeyboard[indec2] = key;      //inputKeyboard2  //indec2
  //     indec2++;
  //     Serial.print("State_2");
  //   }

  //   if (indec2 == 5 && option == 0) {

  //     Serial.println();
  //     Serial.print("Lệnh đã nhập: ");
  //     Serial.println(inputKeyboard);

      
  //     if (compare(inputKeyboard, passwordDefault2) == true) {
  //       Serial.println();
  //       Serial.println("MỞ CỬA THÀNH CÔNG");
  //       //...

  //       // Reset chỉ số
  //       indec = 0;
  //       indec2 = 0;
  //       state = 0;


  //     }  else if (compare(inputKeyboard, passwordDefault3) == true) {

  //       option = 1;
  //       Serial.println();
  //       Serial.print("Nhập mật khẩu mới ");
  //       indec = 5;
  //       indec2 = 0;
  //       state = 2;

  //     } else if (compare(inputKeyboard, passwordDefault4) == true) {
  //       Serial.println();
  //       Serial.println("Đọc MK");
  //       readEpprom();
  //       indec = 0;
  //       indec2 = 0;
  //       state = 0;

  //     }
      
      
  //     else {
  //       indec2 = 0;
  //       state = 2;
  //       Serial.println();
  //       Serial.println("SAI LỆNH");
  //       Serial.println();
  //       Serial.print("Vui lòng nhập lệnh");

  //     }
  //   } else if (indec2 == 5 && option == 1) {
      
  //     writeEpprom(inputKeyboard);
  //     Serial.println();
  //     Serial.println();
  //     Serial.print("Nhập lại mật khẩu");
  //     // indec = 0;
  //     // indec2 = 0;
  //     // state = 0;
  //     // option = 0;

  //     indec = 5;
  //     indec2 = 0;
  //     state = 2;
  //     option = 2;
  //   } else if (indec2 == 5 && option == 2) {
  //     readEpprom();

  //     if (compare(inputKeyboard, a) == true) {
  //       Serial.println();
  //       Serial.println("Mật khẩu xác nhận chính xác");
  //       Serial.println("Đổi mật khẩu thành công");
  //       indec = 0;
  //       indec2 = 0;
  //       state = 0;
  //       option = 0;
  //     } else {

  //       Serial.println();
  //       Serial.println("Sai mật khẩu xác nhận");
  //       Serial.println();
  //       Serial.print("Nhập lại mật khẩu xác nhận");
  //       indec = 5;
  //       indec2 = 0;
  //       state = 2;
  //       option = 2;
  //     }
  //     }
  //   }
  // }

}


//END








// void loop() {
//   // char myData[5] = {'1', '2', '3', '4', '5'};
//   // writeEpprom(myData);
//   // // giatri1 = EEPROM.read(0);
//   // // giatri2 = EEPROM.read(1);
//   // // giatri3 = EEPROM.read(2);
//   // // giatri4 = EEPROM.read(3);

//   // // Serial.print(giatri1);
//   // // Serial.print(giatri2);
//   // // Serial.print(giatri3);
//   // // Serial.print(giatri4);
//   // // Serial.println();
//   // // delay(1000);


//   // readEpprom();

//   // int kq = strcmp(str9, str10);
//   // Serial.print(kq);
//   // delay(1000);
//   if (state == 0) {
//     Serial.println("   ");
//     Serial.println("Vui lòng nhập mật khẩu");
//     state = 1;
//   }

//   char key = keypad.getKey(); 

//   if (key) {
//     if (indec < 4 && state == 1) {
//     Serial.println("   ");
//     Serial.print(key);
//     Serial.print("   ");
//     inputKeyboard[indec] = key;
//     indec++;
//     Serial.print("State_1");

//   } else if (indec == 4 && state == 1) {

//     Serial.println("   ");
//     Serial.print(key);
//     Serial.print("   ");
//     inputKeyboard[indec] = key;
//     indec++;
//     Serial.print("State_1");

//       // Đã nhập đủ 5 ký tự, thực hiện các xử lý tiếp theo
//     Serial.println(" ");
//     Serial.print("Mật khẩu đã nhập: ");
//     Serial.print(inputKeyboard);


//     if (compare(inputKeyboard, passwordDefault) == true) {
//       Serial.print("_OK");
//       Serial.println("   ");
//       state = 2;
//     } else {
//       Serial.println("   ");
//       indec = 0;
//       state = 1;
//     }
//     // indec = 0; // Reset chỉ số
//   } else if (indec == 5 && state == 2) {

    
//     Serial.println("   ");
//     Serial.print(key);
//     Serial.print("   ");
//     inputKeyboard[indec2] = key;      //inputKeyboard2  //indec2
//     indec2++;
//     Serial.print("State_2");
  
//     if (indec2 == 5) {

//       Serial.println(" ");
//       Serial.print("Lệnh đã nhập: ");
//       Serial.print(inputKeyboard);
//       option = 1;
      
//       if (compare(inputKeyboard, passwordDefault2) == true) {
//         Serial.print(" Mở_Cửa");
//         Serial.println("   ");
//         //...

//         // Reset chỉ số
//         indec = 0;
//         indec2 = 0;
//         state = 0;


//       }  else if (compare(inputKeyboard, passwordDefault3) == true) {

//           Serial.print(" Nhập mật khẩu mới ");
//           Serial.println("   ");
//           indec = 0;
//           indec2 = 0;
//           state = 0;


//       } else if (compare(inputKeyboard, passwordDefault4) == true) {
//         Serial.println(" Đọc MK mới");
//         readEpprom();
//         indec = 0;
//         indec2 = 0;
//         state = 0;

//       }
      
      
//       else {
//         indec2 = 0;
//         state = 2;

//         Serial.println(" SAI  ");
//       }
//     }

//   }

//   // else {
//   //   Serial.print(compare(key));

//   // }
// }



// }