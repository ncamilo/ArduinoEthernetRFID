/**
* Read a card using a mfrc522 reader on your SPI interface
* Pin layout should be as follows (on Arduino Uno):
* MOSI: Pin 11 / ICSP-4
* MISO: Pin 12 / ICSP-1
* SCK: Pin 13 / ISCP-3
* SS: Pin 10
* RST: Pin 9
*
* Script is based on the script of Miguel Balboa. 
* @version 0.1
* 
* @author Nelson Almeida
* @since 06-01-2017
*/

// Include libraries
#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <MyRealTimeClock.h>
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 1, en = 0, d4 = 7, d5 = 6, d6 = 5, d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Pinos do RTC
MyRealTimeClock myRTC( A0, A1, A2 ); // Assign Digital Pins
//                     CLK DAT RST
#define SS_PIN  8
#define RST_PIN 9

// Initialize Mac for each EthernetShield (NEED CHANGE!)
byte mac[] = { 0x90,0xA2,0xDA,0x0F,0x49,0x90 };

// Descomentar esta linha se deseja um IP fixo
// Uncomment this line to fixed IP
IPAddress ip(10,67,102,241);

// Descomentar esta linha para obter um DNS fixo
// Uncomment this line to fixed DNS
IPAddress myDns ( 10,  67, 102,   1);
IPAddress subnet(255, 255, 254,   0); // Netmask
byte server[] = { 10,  67, 102, 250}; 
String locationPost = "http://10.67.102.253/rfid.php?idcard=";

// Inicializa a instância client
// Initialize client
EthernetClient client;

// Instancia o leitor RFID
// Initialize RFID Reader
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Configura variáveis
// Setup variables:
int serNum[4], cont = 0;
String finalnumber, RA, nome;
char c, response;
void Tela_Inicio(void);
int led_vm = A5;
int led_vd = A4;
unsigned long tempo = 0;
    
void setup(){
  // Define pinos dos LEDs
  // Define LEDs pins
  pinMode(led_vm, OUTPUT);
  pinMode(led_vd, OUTPUT);
  digitalWrite(led_vm, HIGH);

  // Inicia LCD
  // Initialize LCD
  lcd.begin(20, 4);

  // Desabilita SPI SD
  // Disable SD SPI
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  // Desabilita SPI W5100
  // Disable W5100 SPI
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);

  // Iniciando...
  // Starting...
  lcd.print ("Iniciando...");

  // Ajust hora inicial no RTC
  // Set time on RTC
  myRTC.setDS1302Time(00, 58, 11, 2, 16, 10, 2017);

  // If wish to use fixed IP and a fixed DNS uncomment these lines  
  // and comment line 98
  //Ethernet.begin(mac, ip, myDns);
  // Inicialize the Ethernet Shield by DHCP  
  //EthernetServer server(80);
  Ethernet.begin(mac, ip, myDns, subnet);
  // Begins the server
  //server.begin();

  // Mostra IP configurado
  // Shows configured IP
  lcd.setCursor (0, 1);
  lcd.print ("IP: ");
  lcd.print(Ethernet.localIP());
  delay(1000);

  // Initialize MFRC522
  // Initialize MFRC522
  SPI.begin(); 
  mfrc522.PCD_Init();  

  // Atualiza hora
  // Hour update
  myRTC.updateTime();

  // Tela de início
  // Startup screen
  Tela_Inicio();
}

void loop(){
  // Atualiza a hora
  // Hour update
  myRTC.updateTime();

  // Mostra a hora
  // Prints hour
  lcd.setCursor(0, 3);
  if (myRTC.dayofmonth < 10)
  {
    lcd.print ("0");
    lcd.print(myRTC.dayofmonth);
    } else {
      lcd.print(myRTC.dayofmonth);
    }
    lcd.print("/");
    if (myRTC.month < 10)
    {
      lcd.print ("0");
      lcd.print(myRTC.month);
    } else {
      lcd.print(myRTC.month);
    }
    lcd.print("/");
    lcd.print(myRTC.year);
    lcd.print("  ");
    if (myRTC.hours < 10)
    {
      lcd.print ("0");
      lcd.print(myRTC.hours);
    } else {
      lcd.print(myRTC.hours);
    }
    lcd.print(":");
    if (myRTC.minutes < 10)
    {
      lcd.print ("0");
      lcd.print(myRTC.minutes);
    } else {
      lcd.print(myRTC.minutes);
    }
    lcd.print(":");
    if (myRTC.seconds < 10)
    {
      lcd.print ("0");
      lcd.print(myRTC.seconds);
    } else {
      lcd.print(myRTC.seconds);
    }

    // Faz a leitura do cartão RFID
    // Reads RFID card
    if (mfrc522.PICC_IsNewCardPresent()){
        if (mfrc522.PICC_ReadCardSerial()){
            // Faz a leitura do cartão RFID
            // Reads RFID card
            for (int i = 0; i < mfrc522.uid.size; i++){
              serNum[i] = mfrc522.uid.uidByte[i];
              finalnumber += String(serNum[i], HEX);
            }
            finalnumber.toUpperCase();
            lcd.setCursor(0, 1);
            lcd.print("AGUARDE...    ");

            // Conecta ao servidor
            // Conect to server
            if (client.connect(server,80)){
              // Envia requisição ao servidor via GET
              // Sends a server requirement via GET
              client.print("GET /rfid.php?idcard=");
              client.print(finalnumber);
              client.println(" HTTP/1.1");
              client.println("Host: 10.67.102.253");
              client.println("User-Agent: Arduino-Ethernet");
              client.println("Accept: text/html");
              client.println("Connection: close");
              client.println();
              while (client.connected() && !client.available())
                delay(1);
              while (client.connected() || client.available()){
                c = client.read();
                if (c == '<')
                  while (1){
                    c = client.read();
                    if (c == '>')
                        break;
                    RA += c;
                  }
                  if (c == '@')
                    while (cont != 20 && c != '>'){
                      c = client.read();
                      nome += c;
                      cont++;
                    }
                  if (c == '#')
                    nome = "NONE!";
                  cont = 0;
                }
      
                // Se não encontrado
                // If unknown
                if (!nome.equals("NONE!")){
                  lcd.setCursor (0, 1);
                  lcd.print ("RA: ");
                  lcd.print(RA);
                  lcd.setCursor(0, 2);
                  lcd.print (nome);
                  digitalWrite (led_vm, LOW);
                  digitalWrite (led_vd, HIGH);
                  // Beep
                  tone (A3, 1000, 200);
                  delay(3000);
                }
                else {
                  digitalWrite (led_vm, HIGH);
                  digitalWrite (led_vd, LOW);
                  lcd.setCursor (0, 1);
                  lcd.print ("RA: nao encontrado");
                  Beep_Beep();
                }
                Tela_Inicio();
                ////Serial.print("Nome: ");
                ////Serial.println (nome);
                digitalWrite (led_vm, HIGH);
                digitalWrite (led_vd, LOW);
              } 
              else {
                // Se conexão falhar...
                // If conection fails...
                lcd.setCursor (0, 1);
                lcd.print("   Erro conexao!   ");
                lcd.setCursor (3, 2);
                lcd.print("Tente de novo!");
                client.stop();
                Beep_Beep();
                delay(2000);
                Tela_Inicio();
              }
              RA = "";
              nome = "";
              finalnumber = "";
        }
    }
    client.stop();
}

void Tela_Inicio(void){

  // Monta rela de início
  // Mount Startup screen
  lcd.clear();
  lcd.print("Access Syst ");
  if (myRTC.hours >= 1 && myRTC.hours < 12)
    lcd.print("Bom dia!  ");
  else if (myRTC.hours >= 12 && myRTC.hours < 18)
    lcd.print("Boa tarde!");
  else
    lcd.print("Boa noite!");
  lcd.setCursor (4, 1);
  lcd.print("Passe o RA");
}

void Beep_Beep(void){

  // Indica erro
  // Indicate error
  tone (A3, 800, 100);
  delay(200);
  tone (A3, 100, 100);
  delay(3000);
}
