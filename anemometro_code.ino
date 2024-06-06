// Porta utilizada pelo sensor de velocidade
int sensor = 2;
int rpm;
float velocidade_kmh;
volatile byte pulsos;
unsigned long timeold;
// Disco encoder
unsigned int pulsos_por_volta = 20;
// Area do disco encoder
float area_disco_encoder = 5.3;

//Váriaveis utilizada pelo FIFO
const int fifo_size = 300;
float fifo[fifo_size];
int fifo_index = 0;
bool fifo_full = false;
int leitura_contagem = 0;

// Configuração de som
int rele_port = 7;
int buzzer_port = 9;

// Configuração potenciometro

int potenc_port = A0;
int potenc_value;

String direcao;

void contador()
{
  // Adicionar mais um número no contador
  pulsos++;
}

void setup()
{
  Serial.begin(9600);
  // Definição das portas utilizadas
  pinMode(rele_port, INPUT);
  pinMode(buzzer_port, OUTPUT);
  pinMode(potenc_port, OUTPUT);
  // Interrupcao 0 - pino digital 2
  // Aciona o contador a cada pulso
  attachInterrupt(0, contador, FALLING);
  pulsos = 0;
  rpm = 0;
  timeold = 0;
  // Inicializa FIFO com zeros
  for (int i = 0; i < fifo_size; i++) {
    fifo[i] = 0.0;
  }

// Inicia-se com a porta do rele desligada e com a porta do buzzer ligada
  digitalWrite(rele_port, LOW);
  digitalWrite(buzzer_port, HIGH);
}

// Funçao criada para calcular a media das ultimas 10 leituras
float calcular_media_ultimas_10_leituras() {
  float soma = 0.0;
  int contagem = 0;
  for (int i = 0; i < 10; i++) {
    int indice = fifo_index - 1 - i;
    if (indice < 0) {
      indice += fifo_size;
    }
    soma += fifo[indice];
    contagem++;
  }
  return soma / contagem;
}

void loop()
{
    // Desabilita interrupcao durante o calculo
    detachInterrupt(0);
    rpm = (60 * 1000 / pulsos_por_volta ) / (millis() - timeold) * pulsos;
    timeold = millis();
    pulsos = 0;

  // Definiçao da direçao com base no valor total (924) do potenciometro
  // Maior que 462 é definido como Norte e menor do que 462 é definido como Sul
    potenc_value = analogRead(potenc_port);
    if(potenc_value > 462){
      direcao = "Norte";
    }else{
      direcao = "Sul";
    }
    

    // Converte RPM para km/h
    // Velocidade (km/h) = RPM * area do disco * 0.06
    velocidade_kmh = rpm * area_disco_encoder * 0.06;

    // Armazena a leitura no FIFO
    fifo[fifo_index] = velocidade_kmh;
    fifo_index++;
    if (fifo_index >= fifo_size) {
      fifo_index = 0;
      fifo_full = true;
    }

    // Incrementa a contagem de leituras
    leitura_contagem++;

    // Mostra o valor de RPM e km/h no serial monitor
    Serial.print("Velocidade = ");
    Serial.print(velocidade_kmh);
    Serial.println(" km/h");

    // Se tiverem sido realizadas 10 leituras, calcula e exibe a média
    if (leitura_contagem == 10) {
      float media = calcular_media_ultimas_10_leituras();
      Serial.print("Média das últimas 10 leituras = ");
      Serial.print(media);
      Serial.println(" km/h");
      Serial.print("Direção:");
      Serial.println(direcao);
       // Reiniciar a contagem
      leitura_contagem = 0;
      // Condiçao para acionamento do buzzer, se media for > 100 e a direçao for Norte
       if(media > 100 && direcao == "Norte"){
        // Mandar energia para o rele
        digitalWrite(rele_port, HIGH);
        // Porta do buzzer, frequencia do som, duraçao do som de 20 segundos
        tone(buzzer_port, 2500, 20000);
        delay(180000);
        tone(buzzer_port, 2500, 20000);
        delay(2000);
        noTone(buzzer_port);
       }

      // Desativar o rele e esperar 6 minutos para a proxima contagem
       digitalWrite(rele_port, LOW);
       delay(360000);
    }

    // Habilita interrupcao, faz a leitura a cada 1 segundo
    attachInterrupt(0, contador, FALLING);
    delay(1000);
}
