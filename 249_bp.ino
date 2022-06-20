#include<WiFi.h>
#include<Adafruit_MQTT.h>
#include<Adafruit_MQTT_Client.h>
#include<DHT.h>

//  rgb led details
byte rpin = 5;
byte gpin = 18;
byte bpin = 19;
byte rchannel = 0;
byte gchannel = 1;
byte bchannel = 2;
byte resolution = 8;
int frequency = 5000;

//  dht details
byte dht_pin = 4;
#define dht_type DHT11
DHT dht(dht_pin , dht_type);

//  wifi credentials
const char ssid[] = "Maloo Home";
const char password[] = "rpds@231180";

//  io details
#define IO_USERNAME  "Dhairya281208"
#define IO_KEY       "aio_pFJN15nXxJPPg1EwPFIwpBUR08xS"
#define IO_BROKER    "io.adafruit.com"
#define IO_PORT       1883

//  client details
WiFiClient wificlient;
Adafruit_MQTT_Client mqtt(&wificlient , IO_BROKER , IO_PORT , IO_USERNAME , IO_KEY);


Adafruit_MQTT_Subscribe red = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/Ledr");
Adafruit_MQTT_Subscribe green = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/Ledg");
Adafruit_MQTT_Subscribe blue = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/Ledb");


Adafruit_MQTT_Publish dew_point = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/Dew");
Adafruit_MQTT_Publish tempc = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/tempc");
Adafruit_MQTT_Publish tempf = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/tempf");
Adafruit_MQTT_Publish tempk = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/tempk");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/humidity");




void setup()
{
  Serial.begin(115200);

  //  connecting with wifi
  Serial.println("Connecting with : ");
  Serial.println(ssid);
  WiFi.begin(ssid , password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected !");
  Serial.print("IP assigned by AP : ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //  RGB led setup
  ledcSetup(rchannel , frequency , resolution);
  ledcSetup(gchannel , frequency , resolution);
  ledcSetup(bchannel , frequency , resolution);

  //  attaching pins with channel
  ledcAttachPin(rpin , rchannel);
  ledcAttachPin(gpin , gchannel);
  ledcAttachPin(bpin , bchannel);

  //  dht setup
  dht.begin();

  //  feeds to be subscribed
  mqtt.subscribe(&red);
  mqtt.subscribe(&green);
  mqtt.subscribe(&blue);

  
}

void loop(){
  //  connecting with server
  mqttconnect();

  //  reading values from dht sensor
  float tempc = dht.readTemperature();
  float tempf = dht.readTemperature(true);
  float tempk = tempc + 273.15;
  float humidity = dht.readHumidity();
  float dew_point = (tempc - (100 - humidity) / 5);  //  dew point in celcius



  if (isnan(tempc)  ||  isnan(tempf)  ||  isnan(humidity)){
    Serial.println("Sensor not working!");
    delay(1000);
    return;
  }

  //  printing these values on serial monitor
  String val = String(tempc) + " *C" + "\t" + String(tempf) + " *F" + "\t" + String(tempk) + " *K" + "\t" + 
               String(humidity) + " %RH" + "\t" + String(dew_point) + " *C";
  Serial.println(val);
  
  Adafruit_MQTT_Subscribe *subscription;
  while (true){
    subscription = mqtt.readSubscription(5000); 
    if (subscription  ==  0){
      Serial.println("No Feed");
      break;
    }
    else 
    {
      if (subscription  ==  &red){
        String temp = (char *)red.lastread;
        
        makecolor(temp.toInt() , 0 , 0);
      }
      
      else if (subscription  ==  &green){
        String temp = (char *)green.lastread;

        //  converting string to integer
        
        makecolor(0 , temp.toInt() , 0);
      }

      else if (subscription  ==  &blue){
        String temp = (char *)blue.lastread;

        //  converting string to integer
        
        makecolor(0 , 0 , temp.toInt());
      }
    }
  }

  delay(7000);
}

void mqttconnect(){
  //  if already connected, return
  if (mqtt.connected())
  return;

  //  if not, connect
  else
  {
    while (true)
    {
      int connection = mqtt.connect();  
      if (connection  ==  0)
      {
        Serial.println("Connected to IO");
        break;  //  connected
      }
      else
      {
        Serial.println("Can't Connect");
        mqtt.disconnect();
        Serial.println(mqtt.connectErrorString(connection));
        delay(5000);  //  wait for 5 seconds
      }
    }
  }


  //  wait for some time
  delay(5000);
}

void makecolor(byte r , byte g , byte b)
{
  //  printing values
  Serial.print("RED : ");
  Serial.print(r);
  Serial.print('\t');
  Serial.print("GREEN : ");
  Serial.print(g);
  Serial.print('\t');
  Serial.print("BLUE : ");
  Serial.println(b);

  //  writing values
  ledcWrite(rchannel , r);
  ledcWrite(gchannel , g);
  ledcWrite(bchannel , b);
}
