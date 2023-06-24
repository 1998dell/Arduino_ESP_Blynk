#include <Wire.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT11
#define WET 250
#define DRY 450

//Pin config to Serial Communication between Arduino to ESP
const int RX = 13;
const int TX = 12;

char c;
float b; 
int buf[10], temp;
int sensorValue = 0; 
unsigned long int avgValue;

String dataIn,
       PhLevelMin, 
       PhLevelMax;

int8_t indexOfPhLevelMin,
       indexOfPhLevelMax;

// Pin config for ultrasonic sensor Mixer Container
const int echo_Mixer = 11;
const int trig_Mixer = 10;

// Pin config for ultrasonic sensor Alka Container
const int echo_Alka = 9;
const int trig_Alka = 8;

// Pin config for ultrasonic sensor Acid Container
const int echo_Acid = 7;
const int trig_Acid = 6;

// Pin config for pH sensor
const int sensor_phLevel = A0;

// Pin config for humidity sensor
const int sensor_humidity = A1;

// Pin config for moisture sensors
const int sensor_moisture_A = A2;  // Plant A
const int sensor_moisture_B = A3;  // Plant B
const int sensor_moisture_C = A4;  // Plant C

// Pin config for relays
const int relay_alka = 5;
const int relay_acid = 4;
const int relay_plantA =3;
const int relay_plantB = 2;
const int relay_plantC = 14;
const int relay_water = 15;

const int container_mixture_height = 55;
const int container_alka_height = 40;
const int container_acid_height = 40;

const int plantMoistureTreshold = 50;

LiquidCrystal_I2C lcd(0x27, 20, 4);
DHT dht(sensor_humidity, DHTTYPE);
SoftwareSerial espPin(RX, TX);

void setup() {
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);

  pinMode(relay_water, OUTPUT);
  pinMode(relay_alka, OUTPUT);
  pinMode(relay_acid, OUTPUT);
  pinMode(relay_plantA, OUTPUT);
  pinMode(relay_plantB, OUTPUT);
  pinMode(relay_plantC, OUTPUT);

  pinMode(sensor_phLevel, INPUT);
  pinMode(sensor_moisture_A, INPUT);
  pinMode(sensor_moisture_B, INPUT);
  pinMode(sensor_moisture_C, INPUT);
  pinMode(sensor_humidity, INPUT);

  pinMode(trig_Mixer, OUTPUT);
  pinMode(echo_Mixer, INPUT);

  pinMode(trig_Alka, OUTPUT);
  pinMode(trig_Alka, INPUT);

  pinMode(trig_Acid, OUTPUT);
  pinMode(echo_Acid, INPUT);

  digitalWrite(relay_water, HIGH);
  digitalWrite(relay_alka, HIGH);
  digitalWrite(relay_acid, HIGH);
  digitalWrite(relay_plantA, HIGH);
  digitalWrite(relay_plantB, HIGH); 
  digitalWrite(relay_plantC, HIGH);

  lcd.begin();
  dht.begin();
  espPin.begin(9600);
  Serial.begin(57600);

  lcd.backlight();
}

void loop() {

  int plantMoistureA = Moisture_Sensor_A();
  int plantMoistureB = Moisture_Sensor_B();
  int plantMoistureC = Moisture_Sensor_C();

  int humidity = Humidity_Sensor();
  int temperature = Temperature_Sensor();

  int mixContainerLevel = HCSR_Mixer();
  int alkContainerLevel = HCSR_Alka();
  int aciContainerLevel = HCSR_Acid();

  double pHLevel = pH_Level();

  ESP_Print(plantMoistureA, plantMoistureB, plantMoistureC, mixContainerLevel, alkContainerLevel, aciContainerLevel, pHLevel, temperature, humidity);
  LCD_Print(plantMoistureA, plantMoistureB, plantMoistureC, mixContainerLevel, alkContainerLevel, aciContainerLevel, pHLevel, temperature, humidity);


  while(espPin.available()>0){
    c = espPin.read();
    if( c == '\n' ){ break; }
    else       { dataIn += c; }
  }

  if( c == '\n'){
    Data_Parser();

    Serial.print(" Selected pH Level Min: " + PhLevelMin);
    Serial.print(" Selected pH Level Max: " + PhLevelMax);
    Serial.print("\n");
    
    c = 0;
    dataIn = "";
  }


  delay(100);
}


int HCSR_Mixer() {

  digitalWrite(trig_Mixer, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_Mixer, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Mixer, LOW);

  long duration = pulseIn(echo_Mixer, HIGH);
  int distance = duration * 0.034 / 2;

  return distance; //in cm
}

int HCSR_Alka() {

  digitalWrite(trig_Alka, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_Alka, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Alka, LOW);

  long duration = pulseIn(echo_Alka, HIGH);
  int distance = duration * 0.034 / 2;

  return distance; //in cm
}

int HCSR_Acid() {

  digitalWrite(trig_Acid, LOW);
  delayMicroseconds(2);
  digitalWrite(trig_Acid, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_Acid, LOW);

  long duration = pulseIn(echo_Acid, HIGH);
  int distance = duration * 0.034 / 2; 

  return distance; //in cm
}

int Moisture_Sensor_A() {
  int sensorValue = analogRead(sensor_moisture_A);
  int outputValue = map(sensorValue, WET, DRY, 100, 0);
  return outputValue;
}

int Moisture_Sensor_B() {
  int sensorValue = analogRead(sensor_moisture_B);
  int outputValue = map(sensorValue, WET, DRY, 100, 0);
  return outputValue;
}

int Moisture_Sensor_C() {
  int sensorValue = analogRead(sensor_moisture_C);
  int outputValue = map(sensorValue, WET, DRY, 100, 0);
  return outputValue;
}

int Temperature_Sensor() {
  int outputValue = dht.readTemperature();
  return outputValue;
}

int Humidity_Sensor() {
  int outputValue = dht.readHumidity();
  return outputValue;
}

float pH_Level(){
  for(int i=0;i<10;i++) 
  { 
    buf[i]=analogRead(sensor_phLevel);
    delay(10);
  }

  for(int i=0;i<9;i++)
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
        {
          temp=buf[i];
          buf[i]=buf[j];
          buf[j]=temp;
        }
    }
  }

  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
 
  float pHVol=(float)avgValue*5.0/1024/4.3;
  float phValue = -5.70 * pHVol + 22.8 ;
  phValue=14.2-phValue;
  
  return phValue;
}

void Data_Parser(){
      indexOfPhLevelMin     = dataIn.indexOf("A");
      indexOfPhLevelMax     = dataIn.indexOf("B");

      PhLevelMin      = dataIn.substring(0, indexOfPhLevelMin);
      PhLevelMax      = dataIn.substring(indexOfPhLevelMin + 1, indexOfPhLevelMax);
}

void LCD_Print(int PMA, int PMB, int PMC, int MXCL, int ALCL, int ACCL, double PHL, int TMP, int HMD){
  
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("REALTIME MONITORING");

  lcd.setCursor(0,1);
  lcd.print("PA:" + String(PMA) + " ");
  lcd.setCursor(0,2);
  lcd.print("PB:" + String(PMB) + " ");
  lcd.setCursor(0,3);
  lcd.print("PC:" + String(PMC) + " ");

  lcd.setCursor(7,1);
  lcd.print("TA:" + String(MXCL) + " ");
  lcd.setCursor(7,2);
  lcd.print("TB:" + String(ALCL) + " ");
  lcd.setCursor(7,3);
  lcd.print("TC:" + String(ACCL) + " ");

  lcd.setCursor(12,1);
  lcd.print("PHL:" + String(PHL));
  lcd.setCursor(12,2);
  lcd.print("TMP:" + String(TMP));
  lcd.setCursor(12,3);
  lcd.print("HMD:" + String(HMD));

}

void  ESP_Print(int PMA, int PMB, int PMC, int MXCL, int ALCL, int ACCL, double PHL, int TMP, int HMD){
  espPin.print(PMA);        espPin.print("A"); 
  espPin.print(PMB);        espPin.print("B");
  espPin.print(PMC);        espPin.print("C");

  espPin.print(HMD);        espPin.print("D");
  espPin.print(TMP);        espPin.print("E");

  espPin.print(MXCL);       espPin.print("F");
  espPin.print(ALCL);       espPin.print("G");
  espPin.print(ACCL);       espPin.print("H");

  espPin.print(PHL, 2);     espPin.print("I"); 
  espPin.print("\n");
}
