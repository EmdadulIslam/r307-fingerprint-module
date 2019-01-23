    #include <Arduino.h>
    #include "Adafruit_Fingerprint.h"
    #include <SoftwareSerial.h>
    #include <EEPROM.h>

   // get a template from the sensor and saves on arduino eeprom
     boolean SaveTemplateOnEEPROM(Adafruit_Fingerprint&, SoftwareSerial&, uint16_t);
    //print on serial monitor the template packages saved on arduino eeprom
    void PrintTemplateOnEEPROM ();
    //get an user fingerprint, load the template saved on eeprom to char buffer 2 and compare this two templates
    // void getFingerprintAndCompareWithEEPROM();

    // pin #2 is IN from sensor (GREEN wire)
    // pin #3 is OUT from arduino  (WHITE wire)
    SoftwareSerial mySerial(2, 3);
    


    Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

    void setup()
    {
       Serial.begin(9600);
       Serial.println();
       Serial.println("Match user fingerprint with saved fingerprint on arduino eeprom");

       // set the data rate for the sensor serial port
       finger.begin(57600);

       if (finger.verifyPassword()) {
          Serial.println("Found fingerprint sensor!");
       } else {
          Serial.println("Did not find fingerprint sensor :(");
          while (1);
       }

     //  SaveTemplateOnEEPROM(finger, mySerial, 1);
       //PrintTemplateOnEEPROM();
    }

    void loop()
    {
       getFingerprintAndCompareWithEEPROM();
   //SaveTemplateOnEEPROM(finger, mySerial, 1);
    }

    void getFingerprintAndCompareWithEEPROM() {
       uint8_t p = finger.getImage();
       if (p != FINGERPRINT_OK)
       {
          Serial.println("Error 1!");
          return;
       }
       else
          Serial.println("Image taken!");

       p = finger.image2Tz();
       if (p != FINGERPRINT_OK)
       {
          Serial.println("Error 2!");
          return;
       }
       else
          Serial.println("Image converted");

       Serial.println("Sending command...");
       p = finger.setModel();
       if (p == FINGERPRINT_OK)
       {
          Serial.println("Sensor ready to receive data packets");
          uint8_t packet[556];
          for(uint16_t i=0; i<556; i++)
          {
             packet[i] = EEPROM.read(i);
          }
          finger.sendFormattedTemplatePackages(packet);
          Serial.println("Template loaded in buffer 2");
       }
       else
       {
          Serial.println("Error 3!");
          return;
       }

       p = finger.getMatch();
       if (p == FINGERPRINT_OK)
       {
          // found a match!
          Serial.print("User fingerprint matches with fingerprint loaded on eeprom");
          Serial.print(" with confidence of "); Serial.println(finger.confidence);
          return;
       }
       else
       {
          Serial.println("Error 4!");
          return;
       }
    }

    void PrintTemplateOnEEPROM ()
    {
       //dump entire templateBuffer on serial.  This prints out 20 lines
       for (int count= 0; count < 28; count++)
       {
          for (int i = 0; i < 20; i++)
          {
             if (count*20+i > 555)
                return;
             Serial.print("0x");
             Serial.print(EEPROM.read(count*20+i), HEX);
             Serial.print(", ");
             if (count*20+i == 138 ||count*20+i == 277 || count*20+i == 416)
             {
                Serial.println();
                Serial.println();
             }
          }
          Serial.println();
       }
    }

    boolean SaveTemplateOnEEPROM(Adafruit_Fingerprint &fingerSesor, SoftwareSerial &fingerSocket, uint16_t id)
    {
       uint8_t p = fingerSesor.loadModel(id);
       switch (p)
       {
       case FINGERPRINT_OK:
          Serial.print("template "); Serial.print(id); Serial.println(" loaded");
          break;
       case FINGERPRINT_PACKETRECIEVEERR:
          Serial.println("Communication error");
          return false;
       default:
          Serial.print("Unknown error "); Serial.println(p);
          return false;
       }

       // OK success!

       p = fingerSesor.getModel();
       switch (p) {
       case FINGERPRINT_OK:
          Serial.print("template "); Serial.print(id); Serial.println(" transferring");
          break;
       default:
          Serial.print("Unknown error "); Serial.println(p);
          return false;
       }

       uint8_t templateBuffer[556];
       memset(templateBuffer, 0xff, 556);  //zero out template buffer
       uint16_t index=0;
       uint32_t starttime = millis();
       while ((index < 556) && ((millis() - starttime) < 1000))
       {
          if (fingerSocket.available())
          {
             templateBuffer[index] = fingerSocket.read();
             index++;
          }
       }

       Serial.print(index); Serial.println(" bytes read");

       //dump entire templateBuffer on EEPROM.
       index = 0;
       while (index < 556)
       {
          EEPROM.update(index, templateBuffer[index]);
          index++;
       }

       Serial.print(index); Serial.println(" bytes saved on EEPROM");

       //dump entire templateBuffer on serial.  This prints out 20 lines
       for (int count= 0; count < 28; count++)
       {
          for (int i = 0; i < 20; i++)
          {
             if (count*20+i > 556)
                return p;
             Serial.print("0x");
             Serial.print(templateBuffer[count*20+i], HEX);
             Serial.print(", ");
             if (count*20+i == 138 ||count*20+i == 277 || count*20+i == 416)
             {
                Serial.println();
                Serial.println();
             }
          }
          Serial.println();
       }
       return true;
    }



