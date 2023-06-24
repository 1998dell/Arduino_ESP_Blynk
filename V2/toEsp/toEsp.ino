#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPL68ztH2EFL"
#define BLYNK_TEMPLATE_NAME "ProjA"
#define BLYNK_AUTH_TOKEN "9kjpUz9rg6vSiyffMDhk4mGaE6faGv0Q"
#define BLYNK_PRINT Serial

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Lulab";
char pass[] = "1234ggwp";

double SelectedpHValueMin;
double SelectedpHValueMax;

const int RX = D7;
const int TX = D8;

char c;

String dataIn, PlantMoistureA, PlantMoistureB, PlantMoistureC,
       Humidity, Temperature,
       MixContainerLevel, AlkContainerLevel, AciContainerLevel,
       PhLevel;

int8_t indexOfPlantMoistureA, indexOfPlantMoistureB, indexOfPlantMoistureC,
       indexOfHumidity, indexOfTemperature,
       indexOfMixContainerLevel, indexOfAlkContainerLevel, indexOfAciContainerLevel,
       indexOfPhLevel;

SoftwareSerial arduinoPin(RX,TX);

void setup() {
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  delay(2000);
  Serial.begin(57600);
  arduinoPin.begin(9600);
}

BLYNK_CONNECTED() {
   Blynk.syncAll();
}

void loop() {

  while(arduinoPin.available()>0){
    c = arduinoPin.read();

    if(c == '\n') { break; }
    else          { dataIn += c; }
  }

  if(c == '\n'){
    Data_Parser();

    Serial.println("pA: " + PlantMoistureA);
    Serial.println("pB: " + PlantMoistureB);
    Serial.println("pC: " + PlantMoistureC);
    Serial.println("Humd: " + Humidity);
    Serial.println("Temp: " + Temperature);
    Serial.println("MixCon: " + MixContainerLevel);
    Serial.println("AlkCon: " + AlkContainerLevel);
    Serial.println("AciCon: " + AciContainerLevel);
    Serial.println("PhLvl: " + PhLevel);
    Serial.println("///////////////////////////");

    Blynk.virtualWrite(V0, PlantMoistureA);
    Blynk.virtualWrite(V1, PlantMoistureB);
    Blynk.virtualWrite(V2, PlantMoistureC);
    Blynk.virtualWrite(V3, Humidity);
    Blynk.virtualWrite(V4, Temperature);
    Blynk.virtualWrite(V5, MixContainerLevel);
    Blynk.virtualWrite(V6, AlkContainerLevel);
    Blynk.virtualWrite(V7, AciContainerLevel);
    Blynk.virtualWrite(V8, PhLevel);    

    c = 0;
    dataIn = "";
  }
  
  Blynk.run();
  delay(2000);
}

BLYNK_WRITE(V9){
  //Min pH Level Selector
  SelectedpHValueMin = param.asDouble();

  arduinoPin.print(SelectedpHValueMin, 1);     arduinoPin.print("A");
  arduinoPin.print(SelectedpHValueMax, 1);     arduinoPin.print("B");
  arduinoPin.print("\n");
}

BLYNK_WRITE(V10){
  //Max pH Level Selector
  SelectedpHValueMax = param.asDouble();

  arduinoPin.print(SelectedpHValueMin, 1);     arduinoPin.print("A");
  arduinoPin.print(SelectedpHValueMax, 1);     arduinoPin.print("B");
  arduinoPin.print("\n");
}

void Data_Parser(){
      indexOfPlantMoistureA     = dataIn.indexOf("A");
      indexOfPlantMoistureB     = dataIn.indexOf("B");
      indexOfPlantMoistureC     = dataIn.indexOf("C");
      indexOfHumidity           = dataIn.indexOf("D");
      indexOfTemperature        = dataIn.indexOf("E");
      indexOfMixContainerLevel  = dataIn.indexOf("F");
      indexOfAlkContainerLevel  = dataIn.indexOf("G");
      indexOfAciContainerLevel  = dataIn.indexOf("H");
      indexOfPhLevel            = dataIn.indexOf("I");

      PlantMoistureA      = dataIn.substring(0, indexOfPlantMoistureA);
      PlantMoistureB      = dataIn.substring(indexOfPlantMoistureA + 1, indexOfPlantMoistureB);
      PlantMoistureC      = dataIn.substring(indexOfPlantMoistureB + 1, indexOfPlantMoistureC);
      Humidity            = dataIn.substring(indexOfPlantMoistureC + 1, indexOfHumidity);
      Temperature         = dataIn.substring(indexOfHumidity + 1, indexOfTemperature);
      MixContainerLevel   = dataIn.substring(indexOfTemperature + 1, indexOfMixContainerLevel);
      AlkContainerLevel   = dataIn.substring(indexOfMixContainerLevel + 1, indexOfAlkContainerLevel);
      AciContainerLevel   = dataIn.substring(indexOfAlkContainerLevel + 1, indexOfAciContainerLevel);
      PhLevel             = dataIn.substring(indexOfAciContainerLevel + 1, indexOfPhLevel);
}
