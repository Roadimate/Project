#include "WiFi.h"
#include "OV7670.h"
#include "BMP.h"
#include "ESPAsyncWebServer.h"
#include <AsyncTCP.h>
#include "SPIFFS.h"
#include "BluetoothSerial.h"
#include <HardwareSerial.h>
#include "TinyGPS++.h"
#include <math.h>
#include "HX711.h"
#include "EEPROM.h"
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>

#define CONFIG_INT_WDT_CHECK_CPU1=1;

/*Geral*/
int modo_voo = 1;                                                       // Var aux de ativação da mala
uint32_t auxt1 = 0;                                                     // Var aux tempo para delays
int delayy = 2000;                                                      // Var aux1 tempo para delays
int estado = 7;                                                         // Var aux funcao

/*Balança*/
const int DT_PIN = 18;                                                  // RX ESP
const int SCK_PIN = 19;                                                 // TX ESP
float peso = 0, peso1 = 0;                                                         // Var medida
String peso_str;                                                        // String para webpage
float limite_peso = 0;                                                  // Var aux excesso peso

/* GPS */
TinyGPSPlus gps;
HardwareSerial SerialGPS(1);                                            // 16 & 17
const double circT = 6378140.0;                                         // metros
float lat1 = 0, lat2 = 0, longt1 = 0, longt2 = 0, travel_distancia = 15;
double distancia = 0;
String lat_str = "0.0000" , lng_str = "0.0000", dist_str = "0.00", distp_str = "0.00";
int dia = 0, mes = 0, ano = 0, hora = 0, minuto = 0, segundo = 0;
String data_str = "00:00:00   00/00/00";


/* Wifi */
const char* ssid     = "Roadimate";                                     // Dados de Login
const char* password = "projeto1819";                                   // Dados de Login
String aux1 = "1";

/* Bluetooth */
int var1 = 1;                                                           // Var aux

/* Bluetooth */
int scanTime = 2;                                                       // Var aux
int RSSI_data;                                                          // Valor RSSI
int device_count = 0;                                                   // Valor aparelhos em alcançe do esp
const char *addr_name;
const char *dev_name;
String puls;                                                            // endereço mac do aparelho BLE
String auxstg;
int auxmv = 0;
uint32_t auxt2 = 0;                                                     // Var aux tempo para delays
int auxfp = 0;                                                          // Var aux encontrar aparelho BLE
uint32_t auxt3 = 0;                                                     // Var aux tempo para delays
int auxpulson = 0;

/*EEPROM*/
int addr = 0;                                                           // Endereço das variaveis
int addr1 = 0;
int minuto1 = 0;
#define EEPROM_SIZE 1050                                                // Tamanho memoria

/*OV7670*/
const int SIOD = 33;                                                    // SDA
const int SIOC = 32;                                                    // SCL

const int VSYNC = 34;                                                   // Sincronização Vertical
const int HREF = 35;                                                    // Sincronização Horizontal

const int XCLK = 25;                                                    // XTAL
const int PCLK = 26;                                                    // XTAL Perifericos

const int D0 = 15;                                                      // Pinos de data da camara
const int D1 = 13;
const int D2 = 21;
const int D3 = 12;
const int D4 = 5;
const int D5 = 14;
const int D6 = 4;
const int D7 = 27;

int auxov1 = 0;
OV7670 *camera;
uint32_t auxt4 = 0;                                                     // Var aux tempo para delays


String header;                                                          // Guardar HTTP request
unsigned char bmpHeader[BMP::headerSize];

AsyncWebServer server(80);                                              // Objeto AsyncWebServer na porta 80
BluetoothSerial SerialBT;                                               // Objeto Bluetooth
HX711 scale;                                                            // Objeto Balança

/*Bluetooth LE*/
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
    }
};

/*Cadeado*/
const int CAD = 22;
int auxcad = 0;

/*Atualização valores Página inicial*/
String getPOS() {
  String all_json;
  if (modo_voo == 0) {
    all_json = "Posição: " + lat_str + ", " + lng_str + " <br>" +
               "Peso: " + peso_str + " kg<br>" +
               "Distância: " + distp_str + " km<br>" +
               "Distância Total: " + dist_str + " km<br>" +
               "Data da Última Atualização: " + data_str + "<br><br>Modo voo ativado, pode desligar a mala.";
    if (auxmv == 20) {
      all_json = "Posição: " + lat_str + ", " + lng_str + " <br>" +
                 "Peso: " + peso_str + " kg<br>" +
                 "Distância: " + distp_str + " km<br>" +
                 "Distância Total: " + dist_str + " km<br>" +
                 "Data da Última Atualização: " + data_str + "<br><br>A mala está fora do alcançe, modo voo ativado.";
    }
  }
  else if (auxcad == 1) {
    all_json = "Posição: " + lat_str + ", " + lng_str + " <br>" +
               "Peso: " + peso_str + " kg<br>" +
               "Distância: " + distp_str + " km<br>" +
               "Distância Total: " + dist_str + " km<br>" +
               "Data da Última Atualização: " + data_str + "<br><br>Cadeado Fechado.";
  }
  else if (auxcad == 0) {
    all_json = "Posição: " + lat_str + ", " + lng_str + " <br>" +
               "Peso: " + peso_str + " kg<br>" +
               "Distância: " + distp_str + " km<br>" +
               "Distância Total: " + dist_str + " km<br>" +
               "Data da Última Atualização: " + data_str + "<br><br>Cadeado Aberto.";
  }
  else if (auxmv == 20) {
    all_json = "Posição: " + lat_str + ", " + lng_str + " <br>" +
               "Peso: " + peso_str + " kg<br>" +
               "Distância: " + distp_str + " km<br>" +
               "Distância Total: " + dist_str + " km<br>" +
               "Data da Última Atualização: " + data_str + "<br><br>A mala está fora do alcançe";
  }
  else {
    all_json = "Posição: " + lat_str + ", " + lng_str + " <br>" +
               "Peso: " + peso_str + " kg<br>" +
               "Distância: " + distp_str + " km<br>" +
               "Distância Total: " + dist_str + " km<br>" +
               "Data da Última Atualização: " + data_str;
  }
  return String(all_json);
}

String processs(const String& var) {
  if (var == "POSICAO") {
    return getPOS();
  }
}

void setup() {
  Serial.begin(115200);                                                  // velocidade da porta Serial
  SerialGPS.begin(9600, SERIAL_8N1, 16, 17);                             // Velocidade de transmissão pra gps, ligações (G & Y)

  pinMode(CAD, OUTPUT);                                                  // Porta do Cadeado

  // Iniciar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // WIFI AP
  WiFi.softAP(ssid, password);
  //WiFi.softAPConfig(local_ip, gateway, subnet);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(F("AP Endereço IP: "));
  Serial.println(IP);
  delay(100);


  /*Página web*/
  server.on("/start", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para a pagina inicial
    request->send(SPIFFS, "/intro.html", String(), false, processs);
  });

  server.on("/fotos", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para a pagina de fotos
    request->send(SPIFFS, "/fotos.html", "text/html");
  });

  server.on("/definicoes", HTTP_GET, [](AsyncWebServerRequest * request) {           // Direcionamento para a pagina de definicoes
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/descricao", HTTP_GET, [](AsyncWebServerRequest * request) {            // Direcionamento para a pagina de descricao
    request->send(SPIFFS, "/descricao.html", "text/html");
  });

  server.on("/styleintro.css", HTTP_GET, [](AsyncWebServerRequest * request) {       // Direcionamento para o ficheiro styleintro.css para formatacao pag
    request->send(SPIFFS, "/styleintro.css", "text/css");
  });

  server.on("/stylefotos.css", HTTP_GET, [](AsyncWebServerRequest * request) {       // Direcionamento para o ficheiro stylefotos.css para formatacao pag
    request->send(SPIFFS, "/stylefotos.css", "text/css");
  });

  server.on("/styledefinicoes.css", HTTP_GET, [](AsyncWebServerRequest * request) {  // Direcionamento para o ficheiro styledefinicoes.css para formatacao pag
    request->send(SPIFFS, "/styledefinicoes.css", "text/css");
  });

  server.on("/mvon", HTTP_GET, [](AsyncWebServerRequest * request) {                 // Direcionamento para botao de alteracao do modo de voo (HIGH)
    modo_voo = 0;
    estado = 6;
    auxmv = 0;
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/mvoff", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para botao de alteracao do modo de voo (LOW)
    modo_voo = 1;
    estado = 7;
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/lockon", HTTP_GET, [](AsyncWebServerRequest * request) {               // Direcionamento para botao do cadeado (Fechado)
    if (modo_voo == 1) {
      digitalWrite(CAD, HIGH);
      auxcad = 1;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  // Route to set GPIO to LOW
  server.on("/lockoff", HTTP_GET, [](AsyncWebServerRequest * request) {              // Direcionamento para botao do cadeado (Aberto)
    if (modo_voo == 1) {
      digitalWrite(CAD, LOW);
      auxcad = 0;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/balon", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para peso da mala
    if (modo_voo == 1) {
      estado = 2;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });
  server.on("/gpson", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para leitura gps
    if (modo_voo == 1) {
      estado = 1;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/dev", HTTP_GET, [](AsyncWebServerRequest * request) {                  // Direcionamento para limpeza de dados na memoria
    if (modo_voo == 1) {
      estado = 8;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/moving", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para teste gps
    if (modo_voo == 1) {
      lat1 = 39.749772;
      longt1 = -8.807482;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/pulson", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para ativação do tracking
    if (modo_voo == 1) {
      auxpulson = 1;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/pulsoff", HTTP_GET, [](AsyncWebServerRequest * request) {                // Direcionamento para desativação do tracking
    if (modo_voo == 1) {
      auxpulson = 0;
    }
    request->send(SPIFFS, "/definicoes.html", "text/html");
  });

  server.on("/test", HTTP_GET, [](AsyncWebServerRequest * request) {                   // Direcionamento para imagens das paginas
    request->send(SPIFFS, "/test.jpeg", "image/jpeg");
  });
  server.on("/test1", HTTP_GET, [](AsyncWebServerRequest * request) {                  // Direcionamento para imagens das paginas
    request->send(SPIFFS, "/test1.jpeg", "image/jpeg");
  });
  server.on("/foto", HTTP_GET, [](AsyncWebServerRequest * request) {                   // Direcionamento para imagens das paginas
    if (auxov1 == 1)
      request->send(SPIFFS, "/foto.bmp", "image/bmp");
    else
      request->send(SPIFFS, "/test.jpeg", "image/jpeg");
  });

  // Start server
  server.begin();

  //Bluetooth le
  puls = String("ed:7b:17:90:a3:70");

  //Setup hx711
  scale.begin(DT_PIN, SCK_PIN);
  Serial.println(F("HX711 Setup"));
  scale.set_scale();
  scale.tare();                                                         // Reset a balanca
  delay(5000);
  scale.set_scale(15550);

  //EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Serial.println(F("EEPROM Setup"));
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println(F("Failed to initialise EEPROM"));
    delay(10000);
  }

  //camera
  camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

}

void loop() {
  if (modo_voo == 1) {
    if (auxt2 < millis()) {
      auxov1 = 0;
      if (auxt3 < millis()) {
        //scanBLE();
        auxt3 = millis() + 15 * delayy;
      }
      if (auxt4 < millis()) {
        //Get_photo();
        auxt4 = millis() + 10 * delayy;
      }
      auxt2 = millis() + 2 * delayy;
    }
    if (estado == 6 || estado == 7 || estado == 8) {
      flashread();
    }
    if (estado == 1) {

      locgps();
      auxt2 = millis() + 2 * delayy;
      if (auxt2 < millis()) {
        locgps();
        auxt2 = millis() + 2 * delayy;
      }
      estado = 0;
    }
    if (estado == 2) {
      balanca();
      auxt2 = millis() + 2 * delayy;
      if (auxt2 < millis()) {
        balanca();
        auxt2 = millis() + 2 * delayy;
      }
      estado = 0;
    }
  }
  else if (modo_voo == 0 & estado >= 0) {
    Serial.println(F("A sua mala está pronta para ser transportada para o porão."));
    estado = -1;
  }
}


/* Leitura de Peso */
/************************************************************/
void balanca() {
  if (estado == 2) {
    if (auxt1 < millis()) {
      if (scale.is_ready()) {                                           // balanca pronta
        Serial.print(F("Peso: "));
        peso1 = -scale.get_units(), 2;                                  // 2 pontos decimais
        peso1 = peso1 + peso;
      }
      Serial.print(peso1);
      Serial.println(F(" kg"));
      if (peso1 >= 0) {
        peso_str = String(peso1, 2);
      }
    }
    else {
      Serial.println(F("Erro 404: HX711 not found."));
    }
    if (peso > 4) {                                                   // 20kg
      Serial.println(F("Peso Excedido, sujeito a mais custos add string all peso supoerior a 20"));
    }
    auxt1 = millis() + delayy;
  }

  estado = 0;
}
/************************************************************/

/* EEPROM */
/************************************************************/
void flashread() {
  if (addr == 2000) {
    addr = 0;
  }
  if (estado == 6) {
    addr = addr + 50;                                                   // Extender longevidade da memoria eeprom
    if (gps.satellites.value() != 0) {
      EEPROM.writeFloat(addr, lat1);
      EEPROM.writeFloat(addr + 4, longt1);
      EEPROM.writeFloat(addr + 8, distancia);
      EEPROM.writeFloat(addr + 12, travel_distancia);
      EEPROM.writeFloat(addr + 20, hora);
      EEPROM.writeFloat(addr + 24, minuto);
      EEPROM.writeFloat(addr + 28, segundo);
      EEPROM.writeFloat(addr + 32, dia);
      EEPROM.writeFloat(addr + 36, mes);
      EEPROM.writeFloat(addr + 40, ano);
    }
    peso = peso1;
    EEPROM.writeFloat(addr + 16, peso);
    Serial.print("Valores guardados na memória Flash: ");
    Serial.print(addr);
    EEPROM.writeFloat(0, addr);
    Serial.println(".");
    EEPROM.commit();                                                  // Salva os dados na EEPROM
    estado = 0;
    Serial.println(F("Leituras OFF"));
  }
  if (estado == 7) {
    Serial.print("Valores lidos da memória Flash: ");
    addr = EEPROM.readFloat(0);
    Serial.print(addr);
    Serial.println(".");
    Serial.print("Posição: ");
    lat1 = EEPROM.readFloat(addr);
    lat_str = String(lat1, 4);
    Serial.print(lat_str);
    Serial.print(" , ");
    longt1 = EEPROM.readFloat(addr + 4);
    lng_str = String(longt1, 4);
    Serial.println(lng_str);
    Serial.print("Distância parcial: ");
    travel_distancia = EEPROM.readFloat(addr + 8);
    distp_str = String(distancia, 2);
    Serial.print(distp_str);
    Serial.println(" km");
    Serial.print("Distância Total: ");
    travel_distancia = EEPROM.readFloat(addr + 12);
    dist_str = String(travel_distancia, 2);
    Serial.print(dist_str);
    Serial.println(" km");
    Serial.print("Peso:");
    peso = EEPROM.readFloat(addr + 16);
    peso_str = String(peso, 2);
    Serial.print(peso_str);
    Serial.println(" kg");
    Serial.print("Data da última atualização: ");
    hora = EEPROM.readFloat(addr + 20);
    minuto = EEPROM.readFloat(addr + 24);
    segundo = EEPROM.readFloat(addr + 28);
    dia = EEPROM.readFloat(addr + 32);
    mes = EEPROM.readFloat(addr + 36);
    ano = EEPROM.readFloat(addr + 40);
    data_str = String(hora) + ":" + String(minuto) + ":" + String(segundo) + "   " + String(dia) + "/" + String(mes) + "/" + String(ano);
    Serial.println(data_str);
    estado = 0;
    Serial.println(F("Leituras OFF"));
  }
  if (estado == 8) {
    Serial.print("Valores limpos da memória Flash: ");
    addr = EEPROM.readFloat(0);
    Serial.print(addr);
    Serial.println(".");
    EEPROM.writeFloat(0, 0);
    EEPROM.writeFloat(addr, 0);
    EEPROM.writeFloat(addr + 4, 0);
    EEPROM.writeFloat(addr + 8, 0);
    EEPROM.writeFloat(addr + 12, 0);
    EEPROM.writeFloat(addr + 16, 0);
    EEPROM.writeFloat(addr + 20, 0);
    EEPROM.writeFloat(addr + 24, 0);
    EEPROM.writeFloat(addr + 28, 0);
    EEPROM.writeFloat(addr + 32, 0);
    EEPROM.writeFloat(addr + 36, 0);
    EEPROM.writeFloat(addr + 40, 0);
    EEPROM.commit();                                                  // Salva os dados na EEPROM
    estado = 0;
    addr = 0;
    Serial.println(F("Leituras OFF"));
  }
}
/************************************************************/

/* Localização/altitude GPS and Distância percorrida */
/************************************************************/
void locgps() {
  if (estado == 1) {
    while (SerialGPS.available() > 0) {
      gps.encode(SerialGPS.read());
    }
    if (auxt1 < millis()) {
      if (gps.satellites.value() != 0) {
        if (lat2 != 0 && longt2 != 0) {
          if (lat1 != lat2 || longt1 != longt2 ) {                                     // Formula haversine, precisa de coordenadas em graus (Radianos = Graus * PI / 180)
            double x = pow( sin( ((lat2 - lat1) * M_PI / 180.0) / 2.0), 2.0 );
            double y = cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0);
            double z = pow( sin( ((longt2 - longt1) * M_PI / 180.0) / 2.0), 2.0 );
            double vetor = x + y * z;
            double distancia = circT * 2.0 * atan2(sqrt(vetor), sqrt(1.0 - vetor));
            if (distancia > 3) {
              travel_distancia = travel_distancia + (distancia / 1000);
              Serial.print("Distancia Total = ");
              Serial.print(travel_distancia);
              Serial.println(" km");
              Serial.print("Distancia entre pontos = ");
              Serial.print(distancia / 1000);
              Serial.println(" km");
              distp_str = String(distancia / 1000);
              dist_str = String(travel_distancia);
            }
          }
        }
        lat2 = lat1;
        longt2 = longt1;
        Serial.print(F("Satelites = "));
        Serial.println(gps.satellites.value());
        Serial.print(F("Posicao = "));
        lat1 = gps.location.lat(), 4;
        lat_str = String(lat1, 4);
        Serial.print(lat_str);
        Serial.print(F(", "));
        longt1 = gps.location.lng(), 4;
        lng_str = String(longt1, 4);
        Serial.println(lng_str);
        lat2 = lat1;
        longt2 = longt1;
        unsigned long age;
      }
      hora = gps.time.hour() + 1;
      minuto = gps.time.minute();
      segundo = gps.time.second();
      dia = gps.date.day();
      mes = gps.date.month();
      ano = gps.date.year();
      if (hora <= 23) {
        hora = hora - 1;
      }
      if (hora == 24) {
        hora = 0;
      }
      data_str = String(hora) + ":" + String(minuto) + ":" + String(segundo) + "   " + String(dia) + "/" + String(mes) + "/" + String(ano);
      if (minuto < 10) {
        data_str = String(hora) + ":" + "0" + String(minuto) + ":" + String(segundo) + "   " + String(dia) + "/" + String(mes) + "/" + String(ano);
      }
      if (segundo < 10) {
        data_str = String(hora) + ":" + String(minuto) + ":" + "0" + String(segundo) + "   " + String(dia) + "/" + String(mes) + "/" + String(ano);
      }
      if (minuto < 10 && segundo < 10) {
        data_str = String(hora) + ":" + "0" + String(minuto) + ":" + "0" + String(segundo) + "   " + String(dia) + "/" + String(mes) + "/" + String(ano);
      }
      Serial.println(data_str);

      auxt1 = millis() + delayy;
    }
  }
}
/************************************************************/


/* BLE */
/************************************************************/
void scanBLE() {
  if (auxpulson == 1) {
    BLEDevice::init("ROADI");
    BLEScan* pBLEScan = BLEDevice::getScan();                                         // Inicia scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);                                                    // Scan ativo consome mais, mas demora menos tempo
    BLEScanResults foundDevices = pBLEScan->start(scanTime);
    device_count = foundDevices.getCount();
    Serial.printf("Devices found: %d\n", device_count);
    for (uint32_t i = 0; i < device_count; i++) {
      BLEAdvertisedDevice device = foundDevices.getDevice(i);
      RSSI_data = device.getRSSI();
      addr_name = device.getAddress().toString().c_str();
      auxstg = String(addr_name);
      if (auxstg.equals(puls)) {
        auxfp = 1;
        auxmv = 0;
        Serial.println("Ao alcançe");
        auxstg = String("0");
      }
      delay(2000);
    }
    if (auxfp == 0) {
      auxmv++;
      Serial.println(auxmv);
      auxstg = String("0");
      if (auxmv == 20) {
        Serial.printf("A mala está fora de alcançe, irá se desligar em 10 segundos");
        modo_voo = 0;
      }
    }
    pBLEScan->clearResults();
    delay(300);
  }
}
/************************************************************/

/* Camara*/
/************************************************************/
void Get_photo() {

  camera-> oneFrame ();
  File file = SPIFFS.open ("/foto.bmp", FILE_WRITE);                              // Abertura do ficheiro

  if (!file) {                                                                    // Boa pratica se o anterior realmente aconteceu
    Serial.println("Erro com o ficheiro.");
    return;                                                                       // If file not opened, do not proceed
  }

  for (int i = 0; i < BMP :: headerSize; i ++)
  {
    file.write(bmpHeader [i]);                                                    // Escreve informação do header no ficheiro
  }

  for (int i = 0; i < camera-> xres * camera-> yres * 2; i ++)
  {
    file.write(camera-> frame [i]);                                               // Escreve informação dos pixeis captados no ficheiro
  }

  file.close();                                                                   // Guarda o ficheiro e fecha-o

  Serial.println ("PHOTO_OK!");
  auxov1 = 1;
}
/************************************************************/
