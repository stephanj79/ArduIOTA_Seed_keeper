#include <EEPROM.h>
//#include <LiquidCrystal.h>
/*
 * ArduIOTA Seedkeeper
 * 
 * By Stephan Jäger
 * 
 * Adresse0 > if facotrydefault true = 1, otherwise 0
 * Adresse1 to Adresse4 > pin to access
 * Adresse5 to Adresse86 > Seed
 * 
 * Use on your own risk!!!
 * 
 */

// _______________________________________________________________________________________________________________________________________________________________PROP 
const byte factoryLength          = 1;
const byte pinLength              = 4;
const byte seedLength             = 81; 
const byte adresseFactoryDefault  = 0;

const byte adressePinStart        = adresseFactoryDefault + factoryLength;
const byte adressePinStop         = pinLength;
const byte adresseSeedStart       = adressePinStart + pinLength;
const byte adresseSeedStop        = adresseSeedStart + seedLength;

bool isAuth                       = false; // User ist mit richtig PIN eingeloggt
bool showMenu                     = true;  // Zeig an ob das Menu gerade angezeigt wird
bool showFactoryDefault           = false; // Zeig an ob das Menu Factzroy Default angezeigt wird
bool setupStart                   = false; // Zeigt an ob der Setup gestartet ist
bool loopStart                    = false; // Zeigt an ob der Loop gestartet ist
bool setSeedgestartet             = false; // Gibt an ob gerade der Seed gesetzt wird

//int lcdOben = 0;
//int lcdUnten = 1;
//LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// _______________________________________________________________________________________________________________________________________________________________ARDUIOTA  
void software_Reset() // Restarts program from beginning but does not reset the peripherals and registers
{
  bool IsAuth = false;
  delay(500);
  asm volatile ("  jmp 0");  
}

void FormatArduIota()
{
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
    if((i % 100) == 0)
    {
      Serial.print(".");
    }
  }
}

// _______________________________________________________________________________________________________________________________________________________________FACTORY DEFAULT
bool IfFactoryDefault() // is factory default state
{
  return (String(EEPROM.read(adresseFactoryDefault)) == "0");
}

String GetPin() // get pin from eeprom
{
  String returnPin = "";
  for (byte i = adressePinStart; i <= adressePinStop ; i++) 
  {
    char c = EEPROM.read(i);  
    returnPin += c;
  }

  return returnPin;
}

void WriteFactoryDefault(byte b)
{
  EEPROM.write(adresseFactoryDefault, b);
}

void SetFactoryDefault()
{
   //Serial.println("in SetFactoryDefault");
  if(showFactoryDefault)
  {
    Serial.println();
    Serial.println("Set to factory default (!!! >DELETE ALL< !!!)?\r\n1. Ok\r\n2. Cancel\r\n");
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
    Serial.println("Format arduIOTA seed keeper. Please wait...");
    
    FormatArduIota();
    
    WriteFactoryDefault(0);
    delay(1);
    
    Serial.println("\r\nFormat successful!\r\nReboot...\r\n");
    delay(1);
    
    Serial.flush();
    software_Reset();
  }
  else if(eingabeLoeschen == "2")
  {
    showMenu = true;
    ShowMenu();
    return;
  }
  else
  {
    SetFactoryDefault();
  }
}

// _______________________________________________________________________________________________________________________________________________________________PIN
void SetPin() // Set pin to eeprom
{
  byte bufferSetPin[pinLength]; 
  Serial.setTimeout(60000L);
  Serial.print("Enter " + String(pinLength));
  Serial.println(" digit PIN (end with #)");
  Serial.readBytesUntil('#', (char *) bufferSetPin, pinLength);
  String pinEingabe = String((char*) bufferSetPin);

  if(pinEingabe == "")
  {
    SetPin();
  }
  
  for (byte i = 0; i < pinLength ; i++) 
  {
    EEPROM.write(i + adressePinStart, bufferSetPin[i]);
  }  

  WriteFactoryDefault(1);
}

bool IfPinExist() // is pin exist = 1 otherwise 0
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

bool VerifyPin() //verifiy the pin
{
  byte bufferUserEingabePin[pinLength];

  for (byte i = 0; i < pinLength ; i++) 
  {
    bufferUserEingabePin[i] = 0;
  }
  
  Serial.setTimeout(5000L);
  if(!setupStart)
  {
    Serial.println("Enter PIN to access (end with #)");
  }
  Serial.readBytesUntil('#', (char *) bufferUserEingabePin, pinLength);
  String pinEingabe = (String((char*)bufferUserEingabePin)).substring(0, pinLength);

 // Serial.println("pinEingabe:"+pinEingabe);
 // Serial.println("GetPin():"+GetPin());

  if(GetPin() == pinEingabe)
  {
    isAuth = true;
  }
  else
  {
    isAuth = false;
  }
  
  return isAuth;
}

// _______________________________________________________________________________________________________________________________________________________________SEED
void GetSeed() // get the seed
{
  String returnSeed = "";
  
  for (byte i = adresseSeedStart; i <= adresseSeedStop ; i++) 
  {
    char c = EEPROM.read(i);  
    returnSeed += c;
  }

  Serial.println();
  Serial.println("Seed: ");
  Serial.print(returnSeed);
  Serial.println();
  showMenu = true;
  ShowMenu();
  return;
}

void SetSeed() // Set Seed
{
  String seedEingabe = "";
  byte bufferSetPin[seedLength];
  
  for (byte i = 0; i < seedLength ; i++) 
  {
    bufferSetPin[i] = 0;
  }
  
  Serial.setTimeout(60000L);
  if(!setSeedgestartet)
  {
    Serial.println();
    Serial.println("Enter seed (end with #)");
    setSeedgestartet = true;
  }
  Serial.readBytesUntil('#', (char *) bufferSetPin, seedLength);
  
  seedEingabe = (String((char*)bufferSetPin)).substring(0,seedLength);

  if(seedEingabe == "")
  {
    SetSeed();
  }

  for (byte i = 0; i < seedLength ; i++) 
  {
    EEPROM.write(i + adresseSeedStart, bufferSetPin[i]);
    delay(1);
    Serial.print("write seed " + String(i * 100 / seedLength));
    Serial.println("%");
  }  

  Serial.print("write seed 100%");

  String returnSeed = "";
  for (byte i = adresseSeedStart; i <= adresseSeedStop ; i++) 
  {
    char c = EEPROM.read(i);  
    returnSeed+=  c;
  }
  
  Serial.println();
  Serial.println("status: success.");

  Serial.println("Seed: ");
  Serial.print(returnSeed);
  
  setSeedgestartet = false;
  showMenu = true;
  ShowMenu();
  return;
}

// _______________________________________________________________________________________________________________________________________________________________MAIN MENU
void ShowMenu() // Show main menu
{
  //lcd.clear();
  //SchreibeLcd("Set Seed", 1, lcdOben);
  //SchreibeLcd("Get Seed", 1, lcdUnten);
  
  Serial.println("\r\nMenu\r\n1. Set seed\r\n2. Read seed\r\n3. Factory default\r\n4. Close\r\n_____________");
  Serial.print(">  ");
  loopStart = true;  
}

String MenuEingabe() // main menu choice
{
  Serial.setTimeout(1000L) ; 
  String menuEingabe = Serial.readString();
  menuEingabe.replace("#", "");
  //Serial.print("EINGABE:"+menuEingabe);
  return menuEingabe;
}

// _______________________________________________________________________________________________________________________________________________________________LCD
void SchreibeLcd(String s, int offset, int v)
{
  //lcd.setCursor(offset, v);
  //lcd.print(s);
}

// _______________________________________________________________________________________________________________________________________________________________SETUP
void setup() {
  Serial.flush();
  Serial.begin(9600);
  
  Serial.println("arduIOTA Seed Keeper");


  //char titel[] = "arduIOTA Seed Keeper";
  //lcd.begin(16, 2);

  //SchreibeLcd("arduIOTA", 4, lcdOben);
  //SchreibeLcd("Seed  Keeper", 2, lcdUnten);

  while (!Serial) {
    ; 
  }

  if(IfFactoryDefault())
  {
    Serial.println("Welcome! Please set up your Pin.");  
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

// _______________________________________________________________________________________________________________________________________________________________LOOP
void loop() {
  String menuEingabe = ""; // Was User als Menü haben will
   
  while(showMenu)
  {
    if(!loopStart)
    {
      ShowMenu();
    }

    menuEingabe = MenuEingabe();

    //Serial.println("menuEingabe:"+menuEingabe);
    
    if(menuEingabe == "")
    {
      //Serial.println("leer");
      return;
    }
    
    if(menuEingabe == "1")
    {
      //Serial.println("1");
      SetSeed();
    }
    
    if(menuEingabe == "2")
    {
      //Serial.println("2");
      GetSeed();
    } 
     
    if(menuEingabe == "3")
    {
      //Serial.println("3");
      showFactoryDefault = true;
      SetFactoryDefault();
    }  
    
    if(menuEingabe == "4")
    {
      //Serial.println("4");
      Serial.flush();
      Serial.println();
      software_Reset();
    }    

    //Serial.println("else");
    return;

  }
}
