#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>

// Dados da rede Wi-Fi
const char* ssid = "iPhone de Lucas";
const char* password = "lucas1106";

// Configuração do broker MQTT
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* mqtt_topic_receive = "consulta/volume";
const char* mqtt_topic_send = "resposta/volume";

// Identificador único da lixeira
const char* lixeira_id = "lixeira_01";

// Pinos do sensor e LEDs
const int trigPin = 26;
const int echoPin = 27;
const int ledVermelho = 18;
const int ledAmarelo = 19;
const int ledVerde = 21;

float distancia;
String ultimoStatus = "";

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMeasureTime = 0;
const unsigned long interval = 500; // intervalo entre leituras

// Fuso horário de Brasília (UTC-3)
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

// Gera status com base na distância
String obterStatus(float d) {
  if (d < 20.0 || d > 450.0) {
    return "Lixeira cheia";
  } else if (d < 30.0) {
    return "Lixeira com meio volume";
  } else {
    return "Lixeira vazia";
  }
}

// Obtém a data/hora atual formatada
String obterDataHoraBrasilia() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Erro ao obter hora";
  }

  char buffer[30];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
  return String(buffer);
}

// Callback para mensagens recebidas
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  if (String(topic) == mqtt_topic_receive && msg == "consultar") {
    String status = obterStatus(distancia);
    String dataHora = obterDataHoraBrasilia();
    String payload = "ID: " + String(lixeira_id) +
                     ", Distância: " + String(distancia) +
                     " cm, Status: " + status +
                     ", Data/Hora: " + dataHora;
    client.publish(mqtt_topic_send, payload.c_str());
  }
}

void conectarWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Conectando-se ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado ao Wi-Fi!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());
}

void configurarNTP() {
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Aguardando sincronização NTP...");
    delay(1000);
  }
  Serial.println("Hora sincronizada com sucesso.");
}

void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT... ");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado!");
      client.subscribe(mqtt_topic_receive);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

void medirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  unsigned long tempoSom = pulseIn(echoPin, HIGH);
  distancia = tempoSom / 58.0;
}

void atualizarLEDs(String status) {
  if (status == "Lixeira cheia") {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, HIGH);
  } else if (status == "Lixeira com meio volume") {
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarelo, HIGH);
    digitalWrite(ledVermelho, LOW);
  } else {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarelo, LOW);
    digitalWrite(ledVermelho, LOW);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  conectarWiFi();
  configurarNTP();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  conectarMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  if (!client.connected()) {
    conectarMQTT();
  }

  client.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastMeasureTime >= interval) {
    lastMeasureTime = currentMillis;

    medirDistancia();
    String statusAtual = obterStatus(distancia);

    atualizarLEDs(statusAtual);

    Serial.print("Distância: ");
    Serial.print(distancia);
    Serial.print(" cm | Status: ");
    Serial.println(statusAtual);

    if (statusAtual != ultimoStatus) {
      ultimoStatus = statusAtual;
      String dataHora = obterDataHoraBrasilia();
      String payload = "ID: " + String(lixeira_id) +
                       ", Distância: " + String(distancia) +
                       " cm, Status: " + statusAtual +
                       ", Data/Hora: " + dataHora;
      client.publish(mqtt_topic_send, payload.c_str());
    }
  }
}
