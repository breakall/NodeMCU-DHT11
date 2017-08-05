#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include<dht.h>
dht DHT;

//DHT11's data pin must be wired to D5 on the NodeMCU
#define DHT11_PIN D5

// configuration
const char* ssid = ""; //wifi ssid name
const char* password = ""; //wifi password
const char* mqtt_server = ""; // will use port: 1883; can be IP address or hostname
const char* mqtt_topic = "";  //this is theM MQTT topic that the data will be posted to
const char* sensor_name = ""; //descriptive name of the sensor; e.g. "NodeMCU+DHT11"
const char* sensor_location = ""; //descriptive name of the sensor's location; e.g. "living room"
const char* mqtt_user = ""; //enter MQTT user here if needed
const char* mqtt_pass = ""; //enter MQTT pass here if needed

//temperature variables
float tempC = 0.0;  // initialise a variable for Temperature in Celsius
float humid = 0.0; // initialise a variable for Humidity


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
	delay(100);
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) 
	{
	  delay(500);
	  Serial.print(".");
	}
	randomSeed(micros());
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) 
{
	Serial.print("Command is : [");
	Serial.print(topic);
	int p =(char)payload[0]-'0';
	int chk = DHT.read11(DHT11_PIN);
	// if MQTT comes a 0 message, show humidity
	if(p==0) 
	{
		Serial.println("to show humidity!]");
		Serial.print(" Humidity is: " );
		Serial.print(DHT.humidity, 1);
		Serial.println('%');
	} 
	// if MQTT comes a 1 message, show temperature
	if(p==1)
	{
	  // digitalWrite(BUILTIN_LED, HIGH);
		Serial.println(" is to show temperature!] ");
		int chk = DHT.read11(DHT11_PIN);
		Serial.print(" Temp is: " );
		Serial.print(DHT.temperature, 1);
		Serial.println(' C');
	}
	Serial.println();
} //end callback

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) 
	{
		Serial.print("Attempting MQTT connection...");
		// Create a random client ID
		String clientId = "ESP8266Client-";
		clientId += String(random(0xffff), HEX);

		// connect to MQTT broker
		//if no user/pass are needed for the MQTT broker, omit them from the client.connect paramters
		//if (client.connect(clientId.c_str()))
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass))
		{
			Serial.println("connected");
			Serial.print("ClientID: " + clientId);
			//once connected to MQTT broker, subscribe command if any
			client.subscribe(""); //enter command topic in this string
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 6 seconds before retrying
			delay(6000);
		}
	}
} //end reconnect()

void setup() {
	Serial.begin(115200);
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	int chk = DHT.read11(DHT11_PIN);
	Serial.print("humidity: " );
	Serial.print(DHT.humidity, 1);
	Serial.println('%');
	Serial.print("temparature: ");
	Serial.print(DHT.temperature, 1);
	Serial.println('C');
}


// format temp and humidity data in json format

String buildJson() {
	String data = "{";
	data+= "\"data\": {";
	data+="\"sensor\": \"";
	data+=sensor_name;
	data+="\",";
	data+="\"location\": \"";
	data+=sensor_location;
	data+="\",";
	data+="\"temperature (C)\": ";
	data+=(int)tempC;
	data+= ",";
	data+="\"humidity\": ";
	data+=(int)humid;
	data+="}";
	data+="}";
	return data;
}



void loop() {
	if (!client.connected()) {
		reconnect();
	}
	client.loop();
	long now = millis();
	// read DHT11 sensor every 10 seconds
	if (now - lastMsg > 10000) 
	{
		lastMsg = now;
		int chk = DHT.read11(DHT11_PIN);
	 
		tempC = DHT.temperature;
		humid = DHT.humidity;

		String json = buildJson();
		char jsonStr[200];
		json.toCharArray(jsonStr,200);
		client.publish(mqtt_topic,jsonStr);
		Serial.println(jsonStr);
	}
}

