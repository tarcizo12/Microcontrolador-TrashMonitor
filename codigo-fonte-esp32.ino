const int ledVerde = 4; 
const int ledVermelho = 7;
const int ledAmarelo = 3;
const int portaBotao = 2; 
const int trigPin = 9;
const int echoPin = 10;
long duration;
int distance;
int distanciaDefinida = -1; 

void setup() {
  configurarPinos();
}

void loop() {
    handleDistanciaDefinida();
    handleBotaoCalibragemPressionado();
}

void handleBotaoCalibragemPressionado(){
  if (isBotaoPressionado()) {
    definirDistancia();
  }
}

void handleDistanciaDefinida(){
  if (!isDistanciaDefinida()) {
    digitalWrite(ledVermelho, HIGH);
  } else {
    verificarDistancia();
    digitalWrite(ledVermelho, LOW);
  }
}

// Função para configurar os pinos
void configurarPinos() {
  pinMode(ledVerde, OUTPUT);  
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(portaBotao, INPUT); 
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
}

// Função para verificar se o botão foi pressionado
bool isBotaoPressionado() {
  return digitalRead(portaBotao) == HIGH;
}

bool isDistanciaDefinida() {
  return distanciaDefinida != -1;
}

// Função para medir e imprimir a distância
void medirEImprimirDistancia() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;
}

// Medir a distância do sensor ate o ponto maximo
void definirDistancia() {
  medirEImprimirDistancia(); 
  distanciaDefinida = distance;
  Serial.print("Distancia definida: ");
  Serial.print(distanciaDefinida);
  Serial.println(" cm");
  digitalWrite(ledVerde, HIGH);;
}

// Função para ativar o LED amarelo
void ativarLedAmarelo() {
  digitalWrite(ledAmarelo, HIGH);
}

// Normalizar porcentagem para que nunca passe de 100% ou seja menor ou igual a 0%
float normalizePorcentagem(float porcentagem) {
  if (porcentagem > 100) {
    return 100;
  } else if (porcentagem <= 0) {
    return 0;
  } else {
    return porcentagem;
  }
}

// Função para verificar a distância do objeto em relação à definida
void verificarDistancia() {
  medirEImprimirDistancia(); 
  float porcentagem = normalizePorcentagem((1 - (float(distance) / distanciaDefinida)) * 100);

  Serial.print("Distancia atual: ");
  Serial.print(distance);
  Serial.println(" cm");

  Serial.print("Porcentagem em relacao a distancia definida: ");
  Serial.print(porcentagem);
  Serial.println(" %");

  delay(1000);
}
