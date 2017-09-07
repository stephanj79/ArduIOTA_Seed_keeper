#include <EEPROM.h>
/*
 * ArduIOTA Seedkeeper
 * 
 * By Stephan Jäger
 * 
 * Adresse0 > if facotrydefault true = 1, otherwise 0
 * Adresse1 to Adresse4 > pin to access
 * Adresse5 to Adresse86 > Seed
 * 
 * Use for your own risk!!!
 * 
 */

const int adressePinStart  = 1;
const int adressePinStop   = 4;
const int adresseSeedStart = 5;
const int adresseSeedStop  = 86;

 
bool isAuth                = false; // User ist mit richtig PIN eingeloggt
bool showMenu              = true;  // Zeig an ob das Menu gerade angezeigt wird
bool showFactoryDefault    = false; // Zeig an ob das Menu Factzroy Default angezeigt wird
bool setupStart            = false; // Zeigt an ob der Setup gestartet ist
bool loopStart             = false; // Zeigt an ob der Loop gestartet ist
bool setSeedgestartet      = false; // Gibt an ob gerade der Seed gesetzt wird
 
 
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  bool IsAuth = false;
  asm volatile ("  jmp 0");  
}

// Gibt an ob der Arduino neu ist
bool IfFactoryDefault()
{
  if(String(EEPROM.read(0)) == "0")
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Liest den Pin
String GetPin()
{
  String returnPin = "";
  for (byte i = adressePinStart; i < adressePinStop+1 ; i++) 
  {
    char c = EEPROM.read(i);  
    returnPin += c;
  }

  return returnPin;
}

// Setz den Pin 
void SetPin()
{
  byte bufferSetPin[4]; 
  Serial.setTimeout(60000L);
  Serial.println("Enter 4 Digit PIN (end with #)");
  Serial.readBytesUntil('#', (char *) bufferSetPin, 4);
  String pinEingabe = String((char*)bufferSetPin);

  if(pinEingabe=="")
  {
    SetPin();
  }
  
  for (byte i = 0; i < 4 ; i++) 
  {
    EEPROM.write(i + 1,bufferSetPin[i]);
    EEPROM.write(0, 1);
  }  
}

// Ist ein Pin auf dem Eeprom gespeichert
bool IfPinExist()
{
  if(IfFactoryDefault())
  {
    return true;
  }
  else
  {
    return false;
  }
}

// Verifiziert die Pineingabe
bool VerifyPin()
{
  byte bufferUserEingabePin[4] = {0,0,0,0};
  
  Serial.setTimeout(5000L);
  if(!setupStart)
  {
    Serial.println("Enter PIN to access (end with #)");
  }
  Serial.readBytesUntil('#', (char *) bufferUserEingabePin, 4);
  String pinEingabe = String((char*)bufferUserEingabePin);

  if(GetPin() == pinEingabe.substring(0,4))
  {
    isAuth = true;
  }
  else
  {
    isAuth = false;
  }
  
  return isAuth;
}

// Zeigt das Menü an
void ShowMenu()
{
  Serial.println("\r\nMenu\r\n1. Setze Seed\r\n2. Lese Seed\r\n3. Auslieferungszustand\r\n4. Beenden\r\n_____________");
  Serial.print(">  ");
  loopStart = true;  
}

// Menüeingabe
String MenuEingabe()
{
  Serial.setTimeout(1000L) ; 
  String menuEingabe = Serial.readString();
  menuEingabe.replace("#", "");
  Serial.print(menuEingabe);
  return menuEingabe;
}

// Setze den Seed
void SetSeed()
{
  String seedEingabe = "";
  byte bufferSetPin[81] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  
  Serial.setTimeout(60000L);
  if(!setSeedgestartet)
  {
    Serial.println();
    Serial.println("Enter Seed (end with #)");
    setSeedgestartet = true;
  }
  Serial.readBytesUntil('#', (char *) bufferSetPin, 81);
  
  seedEingabe = (String((char*)bufferSetPin)).substring(0,81);
  
  if(seedEingabe == "")
  {
    SetSeed();
  }

  for (byte i = 0; i < 81 ; i++) 
  {
    EEPROM.write(i + 5, bufferSetPin[i]);
    delay(1);
    Serial.print("Beschreibe Seed " + String(i*100/81));
    Serial.println("%");
  }  

  Serial.print("Beschreibe Seed 100%");

  String returnSeed = "";
  for (byte i = adresseSeedStart; i <= adresseSeedStop ; i++) 
  {
    char c = EEPROM.read(i);  
    returnSeed+=  c;
  }
  
  Serial.println();
  Serial.println("Seed geschrieben.");

  Serial.println("Gespeichert Seed lautet: ");
  Serial.print(returnSeed);
  
  setSeedgestartet = false;
  showMenu = true;
  ShowMenu();
  return;
}

// Lese den gespeicherten Seed
void GetSeed()
{
  String returnSeed = "";
  for (byte i = adresseSeedStart; i <= adresseSeedStop ; i++) 
  {
    char c = EEPROM.read(i);  
    returnSeed += c;
  }

  Serial.println();
  Serial.println("Gespeichert Seed lautet: ");
  Serial.print(returnSeed);
  Serial.println();
  showMenu = true;
  ShowMenu();
  return;
}

void SetFactoryDefault()
{
  if(showFactoryDefault)
  {
    Serial.println();
    Serial.println("Arduino auf Auslieferungszustand?\r\n1. Ja\r\n2. Abbrechen\r\n");
    showFactoryDefault = false;
  }
  
  Serial.setTimeout(1000L); 
  String eingabeLoeschen = Serial.readString();
  eingabeLoeschen.replace("#", "");

  if(eingabeLoeschen == "0")
  {
    SetFactoryDefault();
  }
  else if(eingabeLoeschen == "1")
  {
    Serial.println("Wird gelöscht.");
    
    for (int i = 0 ; i < EEPROM.length() ; i++) {
      EEPROM.write(i, 0);
    }
    
    EEPROM.write(0, 0);
    delay(1);
    
    Serial.println("ok!\r\nReboot...\r\n");
    delay(1);
    
    Serial.flush();
    software_Reset();
  }
  else if(eingabeLoeschen == "2")
  {
    showMenu=true;
    ShowMenu();
    return;
  }
  else
  {
    SetFactoryDefault();
  }
}

void setup() {
  Serial.flush();
  Serial.begin(9600);
  Serial.println(F("arduIOTA Wallet"));
  
  while (!Serial) {
    ; 
  }

  if(IfFactoryDefault())
  {
    Serial.println(F("Welcome! Please set up your Pin."));  
    SetPin(); 
    software_Reset(); 
  }
  
  if(!VerifyPin()&&!isAuth)
  {
    Serial.flush();
    setupStart = true;
    software_Reset(); 
  }
}

void loop() {
  String menuEingabe = ""; // Was User als Menü haben will
   
  while(showMenu)
  {
    if(!loopStart)
    {
      ShowMenu();
    }

    menuEingabe = MenuEingabe();
    
    if(menuEingabe == "")
    {
      return;
    }
    else if(menuEingabe == "1")
    {
      SetSeed();
    }
    else if(menuEingabe == "2")
    {
      GetSeed();
    }  
    else if(menuEingabe == "3")
    {
      showFactoryDefault = true;
      SetFactoryDefault();
    }  
    else if(menuEingabe == "4")
    {
      Serial.flush();
      Serial.println();
      software_Reset();
    }    
    else
    {
      return;
    }
  }
}

