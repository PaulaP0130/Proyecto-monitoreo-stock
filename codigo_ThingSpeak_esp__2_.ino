#include <WiFi.h>
#include <ThingSpeak.h> 
#include <HX711.h>


const char* ssid = "Ines Elvira";
const char* password = "51665204";

unsigned long channelID = 2684450;  
const char* apiKey = "X2EOB99KJG0V8466";        
WiFiClient client;


const int TRIG_PIN = 18;   
const int ECHO_PIN = 19;  
const int LOADCELL_DOUT_PIN = 4;  
const int LOADCELL_SCK_PIN = 5;

float scaleFactor = 2100.0;  


HX711 scale;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 

  if (duration == 0) {
    return -1;
  }

  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(scaleFactor);
  scale.tare();
  Serial.println("Celda de carga inicializada.");

  setup_wifi();

  ThingSpeak.begin(client);  
}

void loop() {
  long distance = medirDistancia();

  if (distance != -1) {
    Serial.print("Distancia medida: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    Serial.println("Error al medir la distancia.");
  }

  if (scale.is_ready()) {
    float weight = scale.get_units(10);
    Serial.print("Peso medido: ");
    Serial.print(weight);
    Serial.println(" gramos");

    // Enviar los datos a ThingSpeak
    ThingSpeak.setField(1, weight);
    ThingSpeak.setField(2, distance);

    int responseCode = ThingSpeak.writeFields(channelID, apiKey);

    if (responseCode == 200) {
      Serial.println("Datos enviados a ThingSpeak correctamente.");
    } else {
      Serial.println("Error al enviar datos a ThingSpeak. Código de respuesta: " + String(responseCode));
    }
  } else {
    Serial.println("Celda de carga no está lista.");
  }

  delay(20000);  
}
