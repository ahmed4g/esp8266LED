#include <PubSubClient.h> // https://github.com/Imroy/pubsubclient
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Update these with values suitable for your network.
IPAddress server(192, 168, x, xxx);

const char *ssid =  "yourwifi";
const char *pass =  "yourpassword";

const char *mqtt_device = "home/test/light_RGB_01";
const char *mqtt_bri    = "home/test/light_RGB_01/Brightness";
const char *mqtt_col    = "home/test/light_RGB_01/Color";
const char *mqtt_sw1    = "home/test/light_RGB_01/Switch1";
const char *mqtt_sta    = "home/test/light_RGB_01/Status";

// Globle V
int Brightness = 255;
int red = 255;
int green = 255;
int blue = 255;

int *P_Brightness = &Brightness;
int *P_red = &red;
int *P_green = &green;
int *P_blue = &blue;


// RGB FET
#define redPIN    15
#define greenPIN  14
#define bluePIN   12


#define PWM_VALUE 255
int gamma_table[PWM_VALUE + 1] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};
// #####################



WiFiClient wclient;
PubSubClient client(wclient, server);


void callback(const MQTT::Publish& pub) {
  Serial.print(pub.topic());
  Serial.print(" => ");
  Serial.println(pub.payload_string());

  String payload = pub.payload_string();

  if (String(pub.topic()) == mqtt_col) {
    int c1 = payload.indexOf(',');
    int c2 = payload.indexOf(',', c1 + 1);

    int red = map(payload.toInt(), 0, 255, 0, *P_Brightness);
    red = constrain(red, 0, *P_Brightness);
    int green = map(payload.substring(c1 + 1, c2).toInt(), 0, 255, 0, *P_Brightness);
    green = constrain(green, 0, *P_Brightness);
    int blue = map(payload.substring(c2 + 1).toInt(), 0, 255, 0, *P_Brightness);
    blue = constrain(blue, 0, *P_Brightness);

    *P_red = red;
    *P_green = green;
    *P_blue = blue;

    red = gamma_table[red] * 4;
    green = gamma_table[green] * 4;
    blue = gamma_table[blue] * 4;


    Serial.print("Received values:");
    Serial.println("Red: " + String(payload.toInt()) + ", Green: " + String(payload.substring(c1 + 1, c2).toInt()) + ", Blue: " + String(payload.substring(c2 + 1).toInt()));
    Serial.print("Target values:");
    Serial.println("Red: " + String(red) + ", Green: " + String(green) + ", Blue: " + String(blue));
    Serial.println("");

    
    analogWrite(redPIN, red);
    analogWrite(greenPIN, green);
    analogWrite(bluePIN, blue);
  }
  else if (String(pub.topic()) == mqtt_sw1) {
    Serial.println(payload);
    if (String(payload) == "ON") {

      analogWrite(redPIN, *P_red * 4);
      analogWrite(greenPIN, *P_green * 4);
      analogWrite(bluePIN, *P_blue * 4);

      client.publish(mqtt_sta, "ON");

    }
    else if (String(payload) == "OFF") {

      analogWrite(redPIN, 0);
      analogWrite(greenPIN, 0);
      analogWrite(bluePIN, 0);

      client.publish(mqtt_sta, "OFF");

    }



  }
  else if (String(pub.topic()) == mqtt_bri) {

    int Brightness = map(payload.toInt(), 0, 255, 0, PWM_VALUE);

    Brightness = constrain(Brightness, 0, PWM_VALUE);
    *P_Brightness = Brightness;

    int red = map(*P_red, 0, 255, 0, *P_Brightness);
    int green = map(*P_green, 0, 255, 0, *P_Brightness);
    int blue = map(*P_blue, 0, 255, 0, *P_Brightness);

    analogWrite(redPIN, red);
    analogWrite(greenPIN, green);
    analogWrite(bluePIN, blue);

    *P_red = red;
    *P_green = green;
    *P_blue = blue;

    Serial.println("Brightness change");
    Serial.println(Brightness);
  }
}



void setup()
{

  
  pinMode(redPIN, OUTPUT);
  pinMode(greenPIN, OUTPUT);
  pinMode(bluePIN, OUTPUT);

  // turn LED on at start

  analogWrite(redPIN, red);
  analogWrite(greenPIN, green);
  analogWrite(bluePIN, blue);

  // Setup console
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();

  client.set_callback(callback);

  WiFi.begin(ssid, pass);



  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");

  }

  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (client.connect(mqtt_device)) {
    client.subscribe(mqtt_col);
    client.subscribe(mqtt_sw1);
    client.subscribe(mqtt_bri);
    Serial.println("MQTT connected:");

    Serial.println(mqtt_device);
    Serial.println(mqtt_bri);
    Serial.println(mqtt_col);
    Serial.println(mqtt_sw1);
    //  Serial.println(mqtt_sw2);
  }

  Serial.println("");
}


void loop()
{
  client.loop();
}
