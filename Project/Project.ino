#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <math.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define tempPin A3
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

String ids[] = {"B0 A1 B6 A6", "7D DA 77 89", "22 20 CE 75", "80 F9 AF A6", "B2 F0 0C F2", "B2 77 B3 4B", "B0 94 CA A6", "F3 BA B7 2D", "12 C7 65 E3"};
String names[] = {"W Top", "R Top", "B Top", "W Hoodie", "R Hoodie", "B Hoodie", "W Jeans", "R Jeans", "B Jeans"};
int types[] = {1, 1, 1, 0, 0, 0, 2, 2, 2};
int inside[] = {1, 1, 1, 1, 1, 1, 1, 1, 1};
int heatFactors[] = {25, 26, 27, 19, 20, 21, 24, 25, 26};
int lastB1State = 0;
int lastB2State = 0;
String scannedId;

void printWardrobe()
{
    Serial.print("Wardrobe: ");
    for(int i = 0; i <= 8; i++)
    {
        if(inside[i] == 1)
            Serial.print(names[i] + ", ");
    }
    Serial.println();
}

double getTemp()
{
    int val = analogRead(tempPin);
    double temp = (double)val / 1024;
    temp *= 5;
    temp -= 0.5;
    temp *= 100;
    return temp;
}

void addItem(String id)
{
    lcd.clear();
    Serial.println("Pressed B1");
    for(int i = 0; i <= 8; i++)
    {
        if(ids[i] == id)
        {
            inside[i] = 1;
            i = 9;
        }
    }
    scannedId = "";
    printWardrobe();
}

void removeItem(String id)
{
    lcd.clear();
    Serial.println("Pressed B2");
    for(int i = 0; i <= 8; i++)
    {
        if(ids[i] == id)
        {
            inside[i] = 0;
            i = 9;
        }
    }
    scannedId = "";
    printWardrobe();
}

void checkB1()
{
    if(digitalRead(2) == HIGH)
        if(scannedId == "")
            getOutfit(getTemp());
        else
            addItem(scannedId);
}

void checkB2()
{
    if(digitalRead(3) == HIGH)
    {
        if(scannedId != "")
            removeItem(scannedId);
    }
}


void getOutfit(double currTemp)
{
    double closestTop = -99.9;
    double closestPants = -99.9;
    String closestTopName = "";
    String closestPantsName = "";

    for(int i = 0; i <= 2; i++)
    {
        if(inside[i] == 1)
            if(abs(currTemp - closestTop) > abs(currTemp - heatFactors[i]))
            {
                closestTop = heatFactors[i];
                closestTopName = names[i];
            }
    }
    for(int i = 6; i <= 8; i++)
    {
        if(inside[i] == 1)
            if(abs(currTemp - closestPants) > abs(currTemp - heatFactors[i]))
            {
                closestPants = heatFactors[i];
                closestPantsName = names[i];
            }
    }
    for(int i = 0; i <= 2; i++)
    {
        for(int j = 3; j <= 5; j++)
        {
            if(inside[i] == 1 && inside[j] == 1)
                if(abs(currTemp - closestTop) > abs(currTemp - (double)(heatFactors[i] + heatFactors[j]) / 3))
                {
                    closestTop = (double)(heatFactors[i] + heatFactors[j]) / 3;
                    closestTopName = names[i] + " & " + names[j];
                }

        }
    }
    Serial.println(currTemp);
    Serial.println(closestTopName + " - " + closestTop);
    Serial.println(closestPantsName + " - " + closestPants);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(closestTopName);
    lcd.setCursor(0, 1);
    lcd.print(closestPantsName);
}

void setup()
{
    pinMode(2, INPUT);
    pinMode(3, INPUT);
    lcd.begin(16, 2);
    lcd.backlight();
    Serial.begin(9600);   // Initiate a serial communication
    SPI.begin();      // Initiate  SPI bus
    mfrc522.PCD_Init();   // Initiate MFRC522
    Serial.println("Approximate your card to the reader...");
    Serial.println();

}
void loop()
{
    checkB1();
    checkB2();
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
    {
        return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
    {
        return;
    }
    //Show UID on serial monitor
    Serial.print("UID tag :");
    String content = "";
    byte letter;
    for (byte i = 0; i < mfrc522.uid.size; i++)
    {
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(mfrc522.uid.uidByte[i], HEX);
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.println();
    content.toUpperCase();
    scannedId = String(content.substring(1));
    lcd.clear();


    for(int i = 0; i <= 8; i++)
    {
        if(String(content.substring(1)) == ids[i])
        {
            lcd.setCursor(0, 0);
            lcd.print(names[i]);
            lcd.setCursor(0, 1);
            lcd.print(heatFactors[i]);
        }
    }
    delay(1000);
}
