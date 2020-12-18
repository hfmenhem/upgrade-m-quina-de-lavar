/*
 * programa: Upgrade máquina de lavar
 * Descrição: transformando uma máquina de lavar Eletrolux LS12 usando 3.5" TFT LCD shield em um arduino mega 2560
 * 
 * Neste programa é usada as bibliotecas <MCUFRIEND_kbv.h>, <Adafruit_GFX.h>, <TouchScreen.h>, <TimerThree.h> e <TimerOne.h> .
 * A primeira biblioteca é usada paraa comunicação com a shield 3.5" TFT LCD. A segunda é uma biblioteca gráfica, neste site é explicada suas funções: https://learn.adafruit.com/adafruit-gfx-graphics-library
 * A terceira é usada para controle do touch screem e as últimas duas são usadas para facilitar o uso do Timer 3 do arduino mega e do Timer 1.
 * 
 * Este é um site que mostra um exemplo da biblioteca <TouchScreen.h> nesta shield: https://www.arduinoecia.com.br/display-tft-touch-3-5-para-arduino/
 * 
 * Algumas contas não foram simplificadas com o intuito de facilitar sua reutilização
 * Sinta-se livre para utilizar e melhorar o código!
 * 
 * Disponível no GitHub: https://github.com/hfmenhem/upgrade-maquina-de-lavar
 * 
 * Programa feito por Hugo Fares Menhem
*/
#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h> //Biblioteca grafica
#include <TouchScreen.h>  //Biblioteca Touch
#include <TimerThree.h>   //Biblioteca para gerenciar o timer 3 do arduino mega
#include <TimerOne.h>     //Biblioteca para gerenciar o timer 1

#define YP A3 // Y+ is on Analog1
#define XM A2 // X- is on Analog2
#define YM 9  // Y- is on Digital7
#define XP 8  // X+ is on Digital6

#define TS_MINX 108
#define TS_MINY 93
#define TS_MAXX 903
#define TS_MAXY 955

#define LCD_RESET A4 //Pode ser conectado ao pino reset do Arduino
#define LCD_CS A3    // Chip Select
#define LCD_CD A2    // Command/Data
#define LCD_WR A1    // LCD Write
#define LCD_RD A0    // LCD Read

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

MCUFRIEND_kbv tft;

#define MINPRESSURE 10
#define MAXPRESSURE 1000

//Definicao de cores
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define info 53     //pino que transmitirá a informação para a máquina de lavar
#define enableR 51  //pino que será o enable do register
#define enableF 49  //pino que será o enable da saída do register
#define Tsecagem 47 //pino que está ligado no botão de turbo secagem
#define pres A15    //pino que será a entrada de informação do pressostato
#define tampa 45    //pino que será a entrada de informação da tampa

int pagina = 1;
int ciclo = 0;
int agua = 0;
bool enxague = false;
bool passa = false;
bool secagem = false;
int fase = 1;  //variavel que controla em qual ciclo a lavagem está
int fase2 = 0; //variavel que controla em qual ciclo a lavagem pode ser que esteja na próxima
int velocidade = 1;
int maquina = 0;
int pressao = 400;
int debug = 0;
int paginaDebug = 1;
int maquinaDebug = 0;
//tabela referente aos ciclos da maquina de lavar:
/*
primeiro dígito: função
ultimos dois: tempo em minutos (apensas para molho, agitação e centrigação)
funções:
EC --
A --
B --
C -- 
D --
E -- 
F --
FR --
G 5
H --
HR --
I --
J 4
K --
L 6
AGITAÇÃO 1
MOLHO 2
centrufugação 3
*/
int tabela[21][19]{
    //21-> como começa do 0, para facilitar é usado 21 para 20 elementos pulando o 0. 19->existem 19 termos em cada ciclos que não foram generalizados e precisam ficar na lista
    {},
    {102, 200, 102, 205, 102, 200, 102, 205, 104, 206, 101, 206, 103, 200, 103, 200, 310, 306, 318},
    {102, 200, 102, 205, 104, 205, 101, 205, 104, 205, 101, 205, 103, 200, 103, 200, 312, 308, 320},
    {102, 200, 102, 205, 102, 204, 102, 204, 102, 204, 102, 203, 101, 200, 101, 200, 310, 306, 318},
    {102, 200, 102, 205, 104, 205, 101, 205, 104, 205, 101, 205, 103, 200, 103, 200, 312, 308, 320},
    {102, 200, 102, 205, 104, 205, 101, 205, 104, 205, 101, 205, 103, 200, 103, 200, 312, 308, 320},
    {103, 211, 103, 204, 103, 205, 104, 216, 104, 209, 105, 206, 103, 200, 101, 200, 312, 306, 318},
    {104, 210, 103, 210, 103, 210, 103, 210, 103, 210, 103, 210, 103, 200, 104, 200, 310, 306, 318},
    {102, 200, 102, 205, 104, 210, 102, 210, 101, 200, 101, 210, 103, 200, 102, 200, 310, 306, 318},
    {102, 200, 102, 220, 104, 210, 104, 210, 102, 200, 102, 210, 103, 200, 103, 200, 310, 306, 318},
    {101, 200, 101, 200, 101, 200, 101, 201, 101, 200, 101, 200, 101, 200, 101, 200, 307, 306, 315},
    {102, 200, 102, 210, 103, 210, 103, 210, 103, 210, 103, 210, 104, 200, 104, 200, 312, 306, 320},
    {105, 206, 104, 212, 103, 212, 102, 210, 102, 206, 102, 218, 104, 218, 106, 212, 312, 308, 320},
    {102, 200, 102, 210, 103, 210, 103, 210, 103, 210, 103, 210, 104, 200, 104, 200, 312, 308, 320},
    {104, 210, 103, 210, 103, 210, 103, 210, 103, 210, 103, 210, 104, 200, 104, 200, 312, 308, 320},
    {102, 200, 102, 205, 102, 200, 102, 205, 107, 210, 107, 210, 104, 200, 104, 200, 312, 308, 320},
    {102, 200, 102, 205, 102, 200, 102, 210, 101, 210, 101, 210, 102, 200, 102, 200, 400, 306, 318},
    {102, 200, 102, 205, 102, 200, 102, 204, 101, 208, 101, 208, 102, 200, 102, 200, 400, 306, 318},
    {102, 200, 102, 205, 102, 200, 102, 205, 105, 206, 102, 206, 102, 200, 101, 200, 308, 304, 316},
    {102, 200, 102, 205, 102, 200, 102, 205, 106, 208, 104, 206, 102, 200, 102, 200, 308, 304, 316},
    {102, 200, 102, 205, 102, 200, 102, 205, 105, 210, 102, 210, 102, 200, 101, 200, 500, 304, 600},
};

// 'Seta', 36x26px
const unsigned char seta[] PROGMEM = {
    0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00,
    0x1f, 0x80, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x07,
    0xfc, 0x00, 0xff, 0xff, 0xff, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 0xff, 0xff,
    0xc0, 0xff, 0xff, 0xff, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0xff, 0xff, 0xff, 0xff, 0x00,
    0x00, 0x00, 0x03, 0xfe, 0x00, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00,
    0x00, 0x1f, 0xc0, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,
    0xf8, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x01, 0x80,
    0x00, 0x00};

void setup()
{
  Serial.begin(9600);
  Timer3.initialize(100000);            //inicializa o Timer3 com periodo de 0,1 segundo (100.000 microssegundos)
  Timer3.attachInterrupt(timerRoutine); //timerRoutine() agora é uma função que faz uma interrupção no código a cada 0,1 segundo

  Timer1.initialize(5000000);    //inicializa o Timer1 com periodo de 5 segundos (5.000.000 microssegundos)
  Timer1.attachInterrupt(erros); //erros() agora é uma função que faz uma interrupção no código a cada50 segundo

  tft.reset();
  tft.begin(tft.readID());
  tft.fillScreen(BLACK);
  tft.setRotation(1);

  pinMode(info, OUTPUT);    //pino que transmitirá a informação para a máquina de lavar
  pinMode(enableR, OUTPUT); //pino que será o enable do register
  pinMode(enableF, OUTPUT); //pino que será o enable da saída do register
  pinMode(pres, INPUT);     //pino que será a entrada de informação do pressostato
  pinMode(tampa, INPUT);    //pino que será a entrada de informação da tampa

  digitalWrite(info, LOW);
  digitalWrite(enableR, LOW);
  digitalWrite(enableF, LOW);

  //desenho do síbolo da HAT
  tft.fillRoundRect(110, 20, 260, 260, 130, WHITE);
  tft.fillRoundRect(115, 25, 250, 250, 125, BLACK);

  tft.fillRect(110, 130, 260, 150, BLACK);

  tft.fillRect(45, 130, 390, 20, WHITE);
  tft.fillRect(50, 135, 380, 10, BLACK);
  //---------------------------
  tft.fillRect(60, 160, 5, 140, WHITE);
  tft.fillRect(155, 160, 5, 140, WHITE);

  tft.fillRect(60, 225, 100, 5, WHITE);
  //---------------------------
  tft.drawLine(190, 300, 236, 160, WHITE);
  tft.drawLine(191, 300, 237, 160, WHITE);
  tft.drawLine(192, 300, 238, 160, WHITE);
  tft.drawLine(193, 300, 239, 160, WHITE);
  tft.drawLine(194, 300, 240, 160, WHITE);

  tft.drawLine(286, 300, 241, 160, WHITE);
  tft.drawLine(287, 300, 242, 160, WHITE);
  tft.drawLine(288, 300, 243, 160, WHITE);
  tft.drawLine(289, 300, 244, 160, WHITE);
  tft.drawLine(290, 300, 245, 160, WHITE);

  tft.fillRect(215, 225, 50, 5, WHITE);
  //---------------------------
  tft.fillRect(320, 160, 100, 5, WHITE);

  tft.fillRect(369, 160, 4, 140, WHITE);
  //---------------------------
  delay(5000);
  tft.fillScreen(BLACK);
  modo(1);
}
void loop()
{
  if (pagina == 6)
  {
    lavagem();
  }
  else if (pagina == 7)
  {
    testes();
    sensor();
  }
  else
  {
    menu();
  }
}

void menu()
{ //função que controla o touch das páginas 1 a 5
  //para ter o valor das coordenadas do touch:
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  digitalWrite(XM, LOW);
  pinMode(YP, OUTPUT);
  digitalWrite(YP, HIGH);
  pinMode(YM, OUTPUT);
  digitalWrite(YM, LOW);
  pinMode(XP, OUTPUT);
  digitalWrite(XP, HIGH);

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    p.x = (map(p.x, TS_MINX, TS_MAXX, tft.height(), 0));
    p.y = (map(p.y, TS_MINY, TS_MAXY, tft.width(), 0));
    //------------------------------------------
    Serial.print("py: ");
    Serial.print(p.y);
    Serial.print(" px: ");
    Serial.println(p.x);
    if (pagina <= 4)
    { //se está nas páginas referentes a seleção de ciclos
      if ((p.x >= 260) && (p.x <= 310) && (p.y >= 10) && (p.y <= 50))
      { //seta voltar
        pagina--;
        if (pagina < 1)
        {
          pagina = 4;
        }
        ciclo = 0;
        tft.fillScreen(BLACK); //apaga o que está na tela
        modo(pagina);          //escreve na tela a página anterior
      }
      if ((p.x >= 260) && (p.x <= 310) && (p.y >= 60) && (p.y <= 100))
      { //seta avançar
        pagina++;
        if (pagina > 4)
        {
          pagina = 1;
        }
        ciclo = 0;
        tft.fillScreen(BLACK); //apaga o que está na tela
        modo(pagina);          //escreve na tela a página anterior
      }
      if ((p.x >= 60) && (p.x <= 87))
      {
        ciclo = (1 + ((pagina - 1) * 5));
        Serial.println(ciclo);
        modo(pagina);
      }
      if ((p.x >= 90) && (p.x <= 117))
      {
        ciclo = (2 + ((pagina - 1) * 5));
        Serial.println(ciclo);
        modo(pagina);
      }
      if ((p.x >= 120) && (p.x <= 147))
      {
        ciclo = (3 + ((pagina - 1) * 5));
        Serial.println(ciclo);
        modo(pagina);
      }
      if ((p.x >= 150) && (p.x <= 177))
      {
        ciclo = (4 + ((pagina - 1) * 5));
        Serial.println(ciclo);
        modo(pagina);
      }
      if ((p.x >= 180) && (p.x <= 207))
      {
        ciclo = (5 + ((pagina - 1) * 5));
        Serial.println(ciclo);
        modo(pagina);
      }
      if ((ciclo == 6) || (ciclo == 13))
      { //nos ciclos tenis(6) e roupa preta(9) duplo enxague está sempre ativo
        enxague = true;
      }
      if (ciclo == 6)
      { //no ciclo tênis(6), o nível de água é sempre baixo
        agua = 1;
      }
      if ((p.x >= 267) && (p.x <= 303) && (p.y >= 310) && (p.y <= 470) && (ciclo != 0))
      {                        //tecla opções
        tft.fillScreen(BLACK); //apaga o que está na tela
        con();
        pagina = 5;
        return; //este return é para sair desta função quando for apertado o botão "opções".
        //Isso é necessário pois como o botão está no mesmo lugar do outro botão para continuar, quando é apertado o primeiro,o segundo também pode ser ativado,
        //devio ao fato de estar logo embaixo no código e só foi feita uma vez a verificação de posição do touch
      }
    }

    if (pagina == 5)
    { //a página 5 é a que se escolhe as configurações da lavagem
      if ((p.x >= 267) && (p.x <= 303) && (p.y >= 10) && (p.y <= 170))
      { //tecla ciclos
        pagina = 1;
        ciclo = 0;
        agua = 0;
        enxague = false;
        passa = false;
        tft.fillScreen(BLACK); //apaga o que está na tela
        modo(pagina);          //volta para tela de escolher os ciclos
      }
      if (ciclo != 6)
      { //no ciclo tênis(6), o nível baixo de água é sempre ativo
        if ((p.x >= 100) && (p.x <= 127) && (p.y <= 256) && (ciclo != 8))
        { //ciclo cobertor/manta(8) não possui seleção automática de água
          agua = 5;
          Serial.println(agua);
          con();
        }
        if ((p.x >= 130) && (p.x <= 157) && (p.y <= 256))
        {
          agua = 4;
          Serial.println(agua);
          con();
        }
        if ((p.x >= 160) && (p.x <= 187) && (p.y <= 256))
        {
          agua = 3;
          Serial.println(agua);
          con();
        }
        if ((p.x >= 190) && (p.x <= 217) && (p.y <= 256))
        {
          agua = 2;
          Serial.println(agua);
          con();
        }
        if ((p.x >= 220) && (p.x <= 247) && (p.y <= 256))
        {
          agua = 1;
          Serial.println(agua);
          con();
        }
      }
      if ((p.y >= 332) && (p.y <= 470) && (p.x >= 70) && (p.x <= 124))
      { //tecla de duplo enxague
        if ((ciclo != 6) && (ciclo != 13))
        { //nos ciclos tenis(6) e roupa preta(9) duplo enxague está sempre ativo
          debug++;
          enxague = (!enxague);
          Serial.println(enxague);
          con();
        }
      }
      if ((p.y >= 332) && (p.y <= 470) && (p.x >= 150) && (p.x <= 204))
      { //tecla de passa fácil
        passa = (!passa);
        Serial.println(passa);
        con();
      }
      if ((p.y >= 238) && (p.y <= 470) && (p.x >= 267) && (p.x <= 303) && (agua != 0))
      {                                  //tecla continuar
        secagem = digitalRead(Tsecagem); //atribui o valor do pino que está conectado no botão da turbo secagem para a variável secagem

        Serial.println("");
        Serial.print("ciclo: ");
        Serial.println(ciclo);
        Serial.print("nível da água: ");
        Serial.println(agua);
        Serial.print("duplo enxague: ");
        Serial.println(enxague);
        Serial.print("passa fácil: ");
        Serial.println(passa);
        Serial.print("turbo secagem: ");
        Serial.println(secagem);
        Serial.println("===============================");
        tft.fillScreen(BLACK); //apaga o que está na tela
        if (ciclo == 1 && debug == 5)
        { //situação que entra na tela de debug
          pagina = 7;
          tft.fillScreen(BLACK);
          displayDebug(1);
        }
        else
        { //contiua para começar a lavagem
          linhaTempo(0);
          pagina = 6;
        }
      }
    }
  }
}

void modo(int pag)
{ //função que controla o display das páginas 1 a 4

  switch (pag)
  {
  case 1:
    tft.drawRoundRect(10, 10, ((6 * 13 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Normal");

    tft.drawRoundRect(10, 60, ((6 * 5 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 1)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Calca");

    tft.drawRoundRect(10, 90, ((6 * 7 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 2)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Moleton");

    tft.drawRoundRect(10, 120, ((6 * 8 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 3)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Camiseta");

    tft.drawRoundRect(10, 150, ((6 * 6 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (150 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 4)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("toalha");

    tft.drawRoundRect(10, 180, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (180 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 5)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("roupa de cama");
    break;

  case 2:
    tft.drawRoundRect(10, 10, ((6 * 15 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Especial");

    tft.drawRoundRect(10, 60, ((6 * 5 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 6)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Tenis");

    tft.drawRoundRect(10, 90, ((6 * 11 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 7)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Tira Mancha");

    tft.drawRoundRect(10, 120, ((6 * 16 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 8)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Edredon/Cobertor");

    tft.drawRoundRect(10, 150, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (150 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 9)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa de mesa");

    tft.drawRoundRect(10, 180, ((6 * 12 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (180 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 10)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Rapido 25min");
    break;

  case 3:
    tft.drawRoundRect(10, 10, ((6 * 15 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Delicado");

    tft.drawRoundRect(10, 60, ((6 * 7 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 11)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Seda/La");

    tft.drawRoundRect(10, 90, ((6 * 9 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 12)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Sintetica");

    tft.drawRoundRect(10, 120, ((6 * 11 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 13)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa preta");

    tft.drawRoundRect(10, 150, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (150 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 14)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa de bebe");

    tft.drawRoundRect(10, 180, ((6 * 12 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (180 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 15)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa intima");
    break;

  case 4:
    tft.drawRoundRect(10, 10, ((6 * 12 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Turbo");

    tft.drawRoundRect(10, 60, ((6 * 5 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 16)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Jeans");

    tft.drawRoundRect(10, 90, ((6 * 16 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 17)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Branco encardido");

    tft.drawRoundRect(10, 120, ((6 * 15 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 18)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Pano de limpeza");

    tft.drawRoundRect(10, 150, ((6 * 11 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (150 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 19)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("pesado sujo");

    tft.drawRoundRect(10, 180, ((6 * 6 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (180 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (ciclo == 20)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("tapete");
    break;
  }
  tft.fillTriangle(50, 260, 50, 310, 10, 285, 0x4E4C);
  tft.fillTriangle(60, 260, 60, 310, 100, 285, 0x4E4C);
  if (ciclo != 0)
  {
    tft.drawRoundRect(310, 267, ((6 * 6 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), 0x7AFD);
    tft.setCursor((310 + (2 * 4)), (267 + (1 * 4)));
    tft.setTextColor(0x7AFD);
    tft.setTextSize(4);
    tft.print("opcoes");
  }
}

void con()
{ //função que controla o display da página 5
  tft.drawRoundRect(10, 10, ((6 * 13 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
  tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.print("configuracoes");

  tft.drawRoundRect(10, 70, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((10 + (2 * 3)), (70 + (1 * 3)));
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.print("nivel de agua");

  tft.drawRoundRect(40, 100, ((6 * 10 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((40 + (2 * 3)), (100 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (agua == 5)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado
  if (ciclo == 8)
  {
    tft.setTextColor(0xB596);
  } //ciclo cobertor/manta(8) não possui seleção automática de água
  tft.setTextSize(3);
  tft.print("automatico");

  tft.drawRoundRect(40, 130, ((6 * 7 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((40 + (2 * 3)), (130 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (agua == 4)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado
  tft.setTextSize(3);
  tft.print("edredon");

  tft.drawRoundRect(40, 160, ((6 * 4 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((40 + (2 * 3)), (160 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (agua == 3)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado
  tft.setTextSize(3);
  tft.print("alto");

  tft.drawRoundRect(40, 190, ((6 * 5 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((40 + (2 * 3)), (190 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (agua == 2)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado
  tft.setTextSize(3);
  tft.print("medio");

  tft.drawRoundRect(40, 220, ((6 * 5 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((40 + (2 * 3)), (220 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (agua == 1)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado
  tft.setTextSize(3);
  tft.print("baixo");

  tft.drawRoundRect(332, 70, ((6 * 7 * 3) + (4 * 3)), (2 * ((7 * 3) + (2 * 3))), (1 * 3), WHITE);
  tft.setCursor((332 + (2 * 3) + (6 * 3)), (70 + (1 * 3)));
  tft.setTextColor(WHITE);
  if ((enxague == true) || (ciclo == 6) || (ciclo == 13))
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado ou se ciclo = 6 ou 9, pois nos ciclos tenis(6) e roupa preta(9) duplo enxague está sempre ativo
  tft.setTextSize(3);
  tft.print("duplo");
  tft.setCursor((332 + (2 * 3)), (70 + (1 * 3) + (7 * 3) + (2 * 3)));
  tft.print("enxague");

  tft.drawRoundRect(332, 150, ((6 * 7 * 3) + (4 * 3)), (2 * ((7 * 3) + (2 * 3))), (1 * 3), WHITE);
  tft.setCursor((332 + (2 * 3) + (6 * 3)), (150 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (passa == true)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo se for clicado
  tft.setTextSize(3);
  tft.print("passa");
  tft.setCursor((332 + (2 * 3) + (6 * 3)), (150 + (1 * 3) + (7 * 3) + (2 * 3)));
  tft.print("facil");

  tft.drawRoundRect(10, 267, ((6 * 6 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), 0x7AFD);
  tft.setCursor((10 + (2 * 4)), (267 + (1 * 4)));
  tft.setTextColor(0x7AFD);
  tft.setTextSize(4);
  tft.print("ciclos");

  if (agua != 0)
  {
    tft.drawRoundRect(238, 267, ((6 * 9 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), 0x7AFD);
    tft.setCursor((238 + (2 * 4)), (267 + (1 * 4)));
    tft.setTextColor(0x7AFD);
    tft.setTextSize(4);
    tft.print("Continuar");
  }
}

//==============================================================================================

void lavagem()
{       //função que controla o funcionamento da máquina de lavar na página 6
  EC(); //<-todos os programas precisam no inico
  //molho longo+agitação
  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 1)
  { //esta verificação ocorre para ser possível avançar e pular partes da lavagem
    Serial.println("molho longo+agitação");
    linhaTempo(fase);
    fase++;
    agitacao(tabela[ciclo][0] % 100);
    molho(tabela[ciclo][1] % 100);
    agitacao(tabela[ciclo][2] % 100);
    molho(tabela[ciclo][3] % 100);
  }

  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 2)
  { //molho+agitação
    Serial.println("molho+agitação");
    linhaTempo(fase);
    fase++;
    agitacao(tabela[ciclo][4] % 100);
    molho(tabela[ciclo][5] % 100);
    agitacao(tabela[ciclo][6] % 100);
    molho(tabela[ciclo][7] % 100);
  }

  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 3)
  { //molho curto+agitação
    Serial.println("molho curto+agitação");
    linhaTempo(fase);
    fase++;
    agitacao(tabela[ciclo][8] % 100);
    molho(tabela[ciclo][9] % 100);
    agitacao(tabela[ciclo][10] % 100);
    molho(tabela[ciclo][11] % 100);
  }

  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 4)
  { //agitação
    Serial.println("agitação");
    linhaTempo(fase);
    fase++;
    agitacao(tabela[ciclo][12] % 100);
    molho(tabela[ciclo][13] % 100);
    agitacao(tabela[ciclo][14] % 100);
    molho(tabela[ciclo][15] % 100);
  }

  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 5)
  { //Enxague
    Serial.println("Enxague");
    linhaTempo(fase);
    fase++;
    //esta parte é igual para todos os ciclos:
    A();
    B();
    C();
    D();
    E();
    if (enxague == true)
    { //esta parte só é acionada no dupo enxague e nos ciclos roupa preta e tenis, onde o duplo enxague é sempre ativo
      EC();
      agitacao(2.5);
      A();
      I();
      E();
    }
    if (ciclo == 10)
    { //para generalizar,´é necessário introduzir esta exeção pois todos os ciclos passam por F_(); e agitacao(2);, menos o "rápido 25min", que a lavagem passa por F(); e agitacao(1);
      FR();
      agitacao(1);
    }
    else
    {
      F_();
      agitacao(2);
    }
  }

  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 6)
  { //Centrifugação
    Serial.println("Centrifugação");
    linhaTempo(fase);
    fase++;
    A();
    B();
    if (ciclo != 10)
    { //para generalizar,´é necessário introduzir esta exeção pois todos os ciclos passam por K();, menos o "rápido 25min", que que pula esta parte
      K();
    }
    if ((passa == false && secagem == false) || (passa == true && secagem == true))
    {
      if ((tabela[ciclo][16] / 100) == 5)
      { //isso significa que é para fazer a função G();
        G();
      }
      if ((tabela[ciclo][16] / 100) == 4)
      { //isso significa que é para fazer a função J();
        J();
      }
      if ((tabela[ciclo][16] / 100) == 3)
      {                                         //isso significa que é para fazer a função agitacao(int tempo);
        centrifugacao(tabela[ciclo][16] % 100); //executa a função centrifugação com o valor dos dois ultimos digitos do valor numero 16 da tabela
      }
    }
    //esta parte do programa é referente aos modos passa fácil e turbo secagem e a função executada com o item numero 18 da lista:
    if ((passa == true) && (secagem == false))
    {
      centrifugacao(tabela[ciclo][17] % 100);
    }
    if ((passa == false) && (secagem == true))
    {
      if (ciclo == 20)
      { //ciclo seda/lâ executa a função L() em vez de centrifugação
        L();
      }
      else
      {
        centrifugacao(tabela[ciclo][18] % 100);
      }
    }

    if (ciclo == 10)
    { //para generalizar,´é necessário introduzir esta exeção pois todos os ciclos passam por H();, menos o "rápido 25min", que passa por HR();
      HR();
    }
    else
    {
      H();
    }
    E();
  }

  if (fase2 != 0)
  {
    tft.fillRect(10, 60, 36, 240, BLACK);
    fase = fase2;
    fase2 = 0;
  }

  if (fase == 7)
  { //fim
    Serial.println("fim");
    linhaTempo(fase);
    //reseta todas as variáveis globais para voltar para o estado inicial de escolher o ciclo, para poder fazer outra lavagem
    pagina = 1;
    ciclo = 0;
    agua = 0;
    enxague = false;
    passa = false;
    secagem = false;
    fase = 1;  //variavel que controla em qual ciclo a lavagem está
    fase2 = 0; //variavel que controla em qual ciclo a lavagem pode ser que esteja na próxima
    velocidade = 1;
    maquina = 0;
    pressao = 400;
    debug = 0;
    tft.fillScreen(BLACK); //apaga o que está na tela
    modo(1);               //para começar tudo de novo.
    return;
  }
}
//funções que controlam as fases contidas nos ciclos em void lavagem(); :
void EC()
{
  int tempo = millis();
  Serial.println("EC");
  if (agua == 5)
  {                                                        //se é o nível automático
    maquinaLavar(false, false, false, false, true, false); //liga a válvula
    delay(5000);                                           //espera 5 segundos
    while (pressostato() < 150)
    { //motor é ligado a cada 5 segundos por 0,4 segundo até que o nível de água seja igual ao nível baixo
      maquinaLavar(false, false, true, false, true, false);
      delay(400);
      maquinaLavar(false, false, false, false, true, false);
      delay(5000);
      if (millis() > (tempo + 90 * 60000))
      {              //se se passou mais de 90 min
        telaErro(2); //erro: água ligada por mais de 90 min
      }
    }
    while (pressostato() < 360)
    { //espera até que atinja o nível de água certo
      if (millis() > (tempo + 90 * 60000))
      {              //se se passou mais de 90 min
        telaErro(2); //erro: água ligada por mais de 90 min
      }
    }
    maquinaLavar(false, false, false, false, false, false); //desliga a válvula
  }

  if (agua == 4)
  {                                                        //se é o nível edredon
    maquinaLavar(false, false, false, false, true, false); //liga a válvula
    while (pressostato() < 360)
    { //espera até que atinja o nível de água certo
      if (millis() > (tempo + 90 * 60000))
      {              //se se passou mais de 90 min
        telaErro(2); //erro: água ligada por mais de 90 min
      }
    }
    maquinaLavar(false, false, false, false, false, false); //desliga a válvula
  }

  if (agua == 3)
  {                                                        //se é o nível alto
    maquinaLavar(false, false, false, false, true, false); //liga a válvula
    while (pressostato() < 340)
    { //espera até que atinja o nível de água certo
      if (millis() > (tempo + 90 * 60000))
      {              //se se passou mais de 90 min
        telaErro(2); //erro: água ligada por mais de 90 min
      }
    }
    maquinaLavar(false, false, false, false, false, false); //desliga a válvula
  }

  if (agua == 2)
  {                                                        //se é o nível médio
    maquinaLavar(false, false, false, false, true, false); //liga a válvula
    while (pressostato() < 240)
    { //espera até que atinja o nível de água certo
      if (millis() > (tempo + 90 * 60000))
      {              //se se passou mais de 90 min
        telaErro(2); //erro: água ligada por mais de 90 min
      }
    }
    maquinaLavar(false, false, false, false, false, false); //desliga a válvula
  }

  if (agua == 1)
  {                                                        //se é o nível baixo
    maquinaLavar(false, false, false, false, true, false); //liga a válvula
    while (pressostato() < 150)
    { //espera até que atinja o nível de água certo
      if (millis() > (tempo + 90 * 60000))
      {              //se se passou mais de 90 min
        telaErro(2); //erro: água ligada por mais de 90 min
      }
    }
    maquinaLavar(false, false, false, false, false, false); //desliga a válvula
  }

  if (passa == true)
  { //se a função passa fácil está ativada
    if (agua == 2)
    {                                                        //só acontece se for o nível médio ou baixo, pois são os únicos em que se aumentar o nível de água ainda fica dentro do limite esperado
      maquinaLavar(false, false, false, false, true, false); //liga a válvula
      while (pressostato() < 290)
      { //adiciona mais 50mm de água
        if (millis() > (tempo + 90 * 60000))
        {              //se se passou mais de 90 min
          telaErro(2); //erro: água ligada por mais de 90 min
        }
      }
      maquinaLavar(false, false, false, false, false, false); //desliga a válvula
    }

    if (agua == 1)
    {                                                        //só acontece se for o nível médio ou baixo, pois são os únicos em que se aumentar o nível de água ainda fica dentro do limite esperado
      maquinaLavar(false, false, false, false, true, false); //liga a válvula
      while (pressostato() < 200)
      { //adiciona mais 50mm de água
        if (millis() > (tempo + 90 * 60000))
        {              //se se passou mais de 90 min
          telaErro(2); //erro: água ligada por mais de 90 min
        }
      }
      maquinaLavar(false, false, false, false, false, false); //desliga a válvula
    }
  }
}
void A()
{
  Serial.println("A");
  int nivel = pressostato();
  int tempo = millis();
  if (agua == 1)
  {                                                        //nível de água baixo
    maquinaLavar(true, false, false, false, false, false); //liga a bomba de drenagem (bool eletrobomba)
    while (pressostato() > 15)
    { //enquanto o pressostatao estiver com mais de 15mm de coluna de água, esperar
      if (((pressostato + 10) >= nivel) && (millis > (tempo + (60 * 10000))))
      { //se demorou mias de 10 min para mudar o nível da água
        telaErro(3);
      }
    }
    delay(35000);                                           //espera mais 35 segundos
    maquinaLavar(false, false, false, false, false, false); //desliga a bomba de drenagem (bool eletrobomba)
  }
  if (agua >= 2)
  {
    maquinaLavar(true, false, false, false, false, false);  //liga a bomba de drenagem (bool eletrobomba)
    delay(45000);                                           //espera 45 segundos
    maquinaLavar(false, false, false, false, false, false); //desliga a bomba de drenagem (bool eletrobomba)
    agitacao(40 / 60);                                      //função agitação por 40 segundos (40/60 de minuto)
    delay(5000);                                            //espera  5 segundos
    maquinaLavar(true, false, false, false, false, false);  //liga a bomba de drenagem (bool eletrobomba)
    while (pressostato() > 15)
    { //enquanto o pressostatao estiver com mais de 15mm de coluna de água, esperar
    }
    delay(35000);                                           //espera mais 35 segundos
    maquinaLavar(false, false, false, false, false, false); //desliga a bomba de drenagem (bool eletrobomba)
  }
}
void B()
{
  Serial.println("B");
  maquinaLavar(false, false, true, false, false, false);
  delay(30000 / velocidade);
  maquinaLavar(false, false, false, false, false, false);
  delay(50000 / velocidade);
}
void C()
{
  Serial.println("C");
  //tabela que controla o movimento do motor mo sentido horáio e anti-horário e a válvula de sabão
  bool tabela[3][75]{
      //1º motor horário, 2ºmotor anti-horário, 3º válvula
      {true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false},
      {false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, false, false, false},
      {true, true, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false, false, false},
  };
  for (int i = 0; i <= 75; i++)
  {
    maquinaLavar(false, false, (tabela[1][i]), (tabela[2][i]), (tabela[3][i]), false); //executa o que está escrito na tabela
    delay(1000 / velocidade);                                                          //delay de 1 segundo
  }
}
void D()
{
  Serial.println("D");
  maquinaLavar(false, false, true, false, false, false);
  delay(30000 / velocidade);
  maquinaLavar(false, false, false, false, false, false);
  delay(100000 / velocidade);
}
void E()
{
  Serial.println("E");
  maquinaLavar(false, true, false, false, false, false);
  delay(5000);
  maquinaLavar(false, true, true, false, false, false);
  delay(5000);
  maquinaLavar(false, false, false, false, false, false);
}
void F_()
{ //por algum motivo void F() dá erro ao compitar, por isso void F_()
  Serial.println("F_");
  maquinaLavar(false, false, false, false, true, false); //liga a valvula de sabão(principal)
  while (pressostato() < 310)
  { //espera até que a coluna dágua seja de até 310 mm
  }
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de sabão(principal)
  agitacao(1);                                            //agitação por 1 min
  maquinaLavar(false, false, false, false, false, true);  //liga a valvula de amaciante
  delay(40000);
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de amaciante
  agitacao(10 / 60);                                      //agitação por 10 segundos (10/60 de minuto)
  maquinaLavar(false, false, false, false, false, true);  //liga a valvula de amaciante
  delay(30000);
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de amaciante
  agitacao(10 / 60);                                      //agitação por 10 segundos (10/60 de minuto)
  maquinaLavar(false, false, false, false, false, true);  //liga a valvula de amaciante
  delay(30000);
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de amaciante
}
void FR()
{
  Serial.println("FR");
  maquinaLavar(false, false, false, false, true, false); //liga a valvula de sabão(principal)
  while (pressostato() < 310)
  { //espera até que a coluna dágua seja de até 310 mm
  }
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de sabão(principal)
  agitacao(30 / 60);                                      //agitação por 30 segundos (30/60 de minuto)
  maquinaLavar(false, false, false, false, false, true);  //liga a valvula de amaciante
  delay(40000);
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de amaciante
  agitacao(10 / 60);                                      //agitação por 10 segundos (10/60 de minuto)
  maquinaLavar(false, false, false, false, false, true);  //liga a valvula de amaciante
  delay(30000);
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de amaciante
  agitacao(10 / 60);                                      //agitação por 10 segundos (10/60 de minuto)
  maquinaLavar(false, false, false, false, false, true);  //liga a valvula de amaciante
  delay(30000);
  maquinaLavar(false, false, false, false, false, false); //desliga a valvula de amaciante
}
void G()
{
  Serial.println("G");
  for (int i = 1; i <= 4; i++)
  {                                                        //repita 4 vezes
    maquinaLavar(true, true, false, true, false, false);   //liga o motor no sentido ant-horário e o freio para poder fazer a centrifugação. também liga a eletrobomb para tirar a água
    delay(50000 / velocidade);                             //delay de 50 segundos
    maquinaLavar(false, true, false, false, false, false); //desliga o motor
    delay(70000 / velocidade);                             //delay de 70 segundos
  }
}
void H()
{
  Serial.println("H");
  maquinaLavar(false, false, false, false, false, false); //desliga o motor
  delay(100000 / velocidade);                             //delay de 100 segundos
}
void HR()
{
  Serial.println("HR");
  maquinaLavar(false, false, false, false, false, false); //desliga o motor
  delay(40000 / velocidade);                              //delay de 40 segundos
}
void I()
{
  Serial.println("I");
  maquinaLavar(false, false, true, false, false, false);  //liga o motor no sentido horário
  delay(30000 / velocidade);                              //delay de 30 segundos
  maquinaLavar(false, false, false, false, false, false); //desliga o motor
  delay(70000 / velocidade);                              //delay de 70 segundos
}
void J()
{
  Serial.println("J");
  for (int i = 1; i <= 5; i++)
  {                                                        //repita 5 vezes
    maquinaLavar(true, true, false, true, false, false);   //liga o motor no sentido ant-horário e o freio para poder fazer a centrifugação.Também liga a eletrobomb para tirar a água
    delay(50000 / velocidade);                             //delay de 50 segundos
    maquinaLavar(false, true, false, false, false, false); //desliga o motor
    delay(70000 / velocidade);                             //delay de 70 segundos
  }
}
void K()
{
  Serial.println("K");
  C();
  C();
}
void L()
{
  Serial.println("L");
  for (int i = 1; i <= 6; i++)
  {                                                        //repita 6 vezes
    maquinaLavar(true, true, false, true, false, false);   //liga o motor no sentido ant-horário e o freio para poder fazer a centrifugação. Também liga a eletrobomb para tirar a água
    delay(50000 / velocidade);                             //delay de 50 segundos
    maquinaLavar(false, true, false, false, false, false); //desliga o motor
    delay(70000 / velocidade);                             //delay de 70 segundos
  }
}
void agitacao(float tempo)
{ //tempo em minutos
  Serial.println("agitação");
  for (int i = 1; i <= (tempo / velocidade); i++)
  { //como cada loop dura 1 minuto, a quanidade de vezes que será feito o loop será o tempo em minutos da função

    int To = millis(); //Tempo inicial

    while (To + 10000 < millis)
    { //enquanto não tiver passsado 10 segundos desde a definiçãa de To(início)
      //toda vez que liga/deliga o motor, o freio e a bomba estarão ligadas:
      if ((ciclo >= 1) && (ciclo <= 10) && (ciclo != 6))
      {                                                       //se é um ciclo normal ou especial que não é o tênis
        maquinaLavar(true, true, false, true, false, false);  //liga motor anti-horário
        delay(400);                                           //delay de 0,4 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(600);                                           //delay de 0,6 segundo
        maquinaLavar(true, true, false, false, true, false);  //liga motor horário
        delay(460);                                           //delay de 0,46 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(600);                                           //delay de 0,6 segundo
      }
      if ((ciclo >= 11) && (ciclo <= 15))
      {                                                       //se é um ciclo delicado
        maquinaLavar(true, true, false, true, false, false);  //liga motor anti-horário
        delay(200);                                           //delay de 0,2 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(700);                                           //delay de 0,7 segundo
        maquinaLavar(true, true, false, false, true, false);  //liga motor horário
        delay(260);                                           //delay de 0,26 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(700);                                           //delay de 0,7 segundo
      }
      if ((ciclo >= 16) && (ciclo <= 20))
      {                                                       //se é um ciclo turbo
        maquinaLavar(true, true, false, true, false, false);  //liga motor anti-horário
        delay(460);                                           //delay de 0,46 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(100);                                           //delay de 0,1 segundo
        maquinaLavar(true, true, false, false, true, false);  //liga motor horário
        delay(560);                                           //delay de 0,56 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(100);                                           //delay de 0,1 segundo
      }
      if (ciclo = 6)
      {                                                       //se é o ciclo de tênis
        maquinaLavar(true, true, false, true, false, false);  //liga motor anti-horário
        delay(300);                                           //delay de 0,2 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(860);                                           //delay de 0,7 segundo
        maquinaLavar(true, true, false, false, true, false);  //liga motor horário
        delay(300);                                           //delay de 0,3 segundo
        maquinaLavar(true, true, false, false, false, false); //desliga motor
        delay(860);                                           //delay de 0,86 segundo
      }
    }

    To = millis(); //Tempo inicial

    while (To + 50000 < millis)
    { //enquanto não tiver passsado 50 segundos desde a definiçãa de To(início)
      //toda vez que liga/deliga o motor, o freio estará ligaado e a bomba estará desligada:
      if ((ciclo >= 1) && (ciclo <= 10) && (ciclo != 6))
      {                                                        //se é um ciclo normal ou especial que não é o tênis
        maquinaLavar(false, true, false, true, false, false);  //liga motor anti-horário
        delay(400);                                            //delay de 0,4 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(600);                                            //delay de 0,6 segundo
        maquinaLavar(false, true, false, false, true, false);  //liga motor horário
        delay(460);                                            //delay de 0,46 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(600);                                            //delay de 0,6 segundo
      }
      if ((ciclo >= 11) && (ciclo <= 15))
      {                                                        //se é um ciclo delicado
        maquinaLavar(false, true, false, true, false, false);  //liga motor anti-horário
        delay(200);                                            //delay de 0,2 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(700);                                            //delay de 0,7 segundo
        maquinaLavar(false, true, false, false, true, false);  //liga motor horário
        delay(260);                                            //delay de 0,26 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(700);                                            //delay de 0,7 segundo
      }
      if ((ciclo >= 16) && (ciclo <= 20))
      {                                                        //se é um ciclo turbo
        maquinaLavar(false, true, false, true, false, false);  //liga motor anti-horário
        delay(460);                                            //delay de 0,46 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(100);                                            //delay de 0,1 segundo
        maquinaLavar(false, true, false, false, true, false);  //liga motor horário
        delay(560);                                            //delay de 0,56 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(100);                                            //delay de 0,1 segundo
      }
      if (ciclo = 6)
      {                                                        //se é o ciclo de tênis
        maquinaLavar(false, true, false, true, false, false);  //liga motor anti-horário
        delay(300);                                            //delay de 0,2 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(860);                                            //delay de 0,7 segundo
        maquinaLavar(false, true, false, false, true, false);  //liga motor horário
        delay(300);                                            //delay de 0,3 segundo
        maquinaLavar(false, true, false, false, false, false); //desliga motor
        delay(860);                                            //delay de 0,86 segundo
      }
    }
  }
}

void molho(float tempo)
{ //tempo em minutos
  Serial.println("molho");
  maquinaLavar(false, true, false, true, false, false);
  delay((tempo * 60000) / velocidade); //como o tempo está em minutos, é necessário multiplicar por 60.000 para comverter em milissegundos
}
void centrifugacao(float tempo)
{ //tempo em minutos
  Serial.println("centrifugação");
  maquinaLavar(true, true, false, true, false, false);    //liga o motor no sentido ant-horário e o freio para poder fazer a centrifugação.  Também liga a eletrobomb para tirar a água
  delay((tempo * 60000) / velocidade);                    //como o tempo está em minutos, é necessário multiplicar por 60.000 para comverter em milissegundos
  maquinaLavar(false, false, false, false, false, false); //desliga o motor  e o freio
}

void timerRoutine()
{ //função que controla o touch da página 6 (função que é acionada a cada 0,1 segundo)
  if (pagina == 6)
  { //se está na página que mostra a linha do tempo(última página)
    Serial.println(millis());
    Serial.println(fase);
    Serial.println(fase2);
    //------------------------------------------
    TSPoint p = ts.getPoint();
    pinMode(XM, OUTPUT);
    digitalWrite(XM, LOW);
    pinMode(YP, OUTPUT);
    digitalWrite(YP, HIGH);
    pinMode(YM, OUTPUT);
    digitalWrite(YM, LOW);
    pinMode(XP, OUTPUT);
    digitalWrite(XP, HIGH);
    //------------------------------------------
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
    {                                                      //se o monitor for tocado
      p.x = (map(p.x, TS_MINX, TS_MAXX, tft.height(), 0)); //mapeia X para estar de acordo com as coordenadas da tela
      p.y = (map(p.y, TS_MINY, TS_MAXY, tft.width(), 0));  //mapeia Y para estar de acordo com as coordenadas da tela

      if ((p.x >= 60) && (p.x <= 88))
      { //se foi tocado em "molho longo+agitação"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 60, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (60 + 14), 13, WHITE);
        fase2 = 1; //faz ir para a fase 1, para que seja executado o "molho longo+agitação"
      }
      if ((p.x >= 90) && (p.x <= 118))
      { //se foi tocado em "molho+agitação"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 90, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (90 + 14), 13, WHITE);
        fase2 = 2; //faz ir para a fase 2, para que seja executado o "molho+agitação"
      }
      if ((p.x >= 120) && (p.x <= 148))
      { //se foi tocado em "molho curto+agitação"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 120, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (120 + 14), 13, WHITE);
        fase2 = 3; //faz ir para a fase 3, para que seja executado o "molho curto+agitação"
      }
      if ((p.x >= 150) && (p.x <= 178))
      { //se foi tocado em "agitação"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 150, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (150 + 14), 13, WHITE);
        fase2 = 4; //faz ir para a fase 4, para que seja executado o "agitação"
      }
      if ((p.x >= 180) && (p.x <= 208))
      { //se foi tocado em "enxague"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 180, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (180 + 14), 13, WHITE);
        fase2 = 5; //faz ir para a fase 5, para que seja executado o "enxague"
      }
      if ((p.x >= 210) && (p.x <= 238))
      { //se foi tocado em "centrifugação"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 210, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (210 + 14), 13, WHITE);
        fase2 = 6; //faz ir para a fase 6, para que seja executado o "centrifugação"
      }
      if ((p.x >= 240) && (p.x <= 298))
      { //se foi tocado em "centrifugação"
        tft.fillRect(10, 60, 36, 240, BLACK);
        tft.drawBitmap(10, 240, seta, 36, 26, WHITE);
        //tft.fillCircle(10 + 14, (240 + 14), 13, WHITE);
        fase2 = 7; //faz ir para a fase 6, para que seja executado o "centrifugação"
      }
    }
    Serial.println(millis());
  }
}

void linhaTempo(int fase)
{ //função que controla o display da página 6
  tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
  tft.setTextColor(WHITE);
  tft.setTextSize(4);

  switch (ciclo)
  {
  case 1:
    tft.print("calca");
    tft.drawRoundRect(10, 10, ((6 * 5 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 2:
    tft.print("moleton");
    tft.drawRoundRect(10, 10, ((6 * 9 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 3:
    tft.print("camiseta");
    tft.drawRoundRect(10, 10, ((6 * 8 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 4:
    tft.print("toalha");
    tft.drawRoundRect(10, 10, ((6 * 6 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 5:
    tft.print("roupa de cama");
    tft.drawRoundRect(10, 10, ((6 * 13 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 6:
    tft.print("tenis");
    tft.drawRoundRect(10, 10, ((6 * 5 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 7:
    tft.print("tira mancha");
    tft.drawRoundRect(10, 10, ((6 * 11 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 8:
    tft.print("edrendon/cobertor");
    tft.drawRoundRect(10, 10, ((6 * 17 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 9:
    tft.print("roupa de mesa");
    tft.drawRoundRect(10, 10, ((6 * 13 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 10:
    tft.print("rapido 25min");
    tft.drawRoundRect(10, 10, ((6 * 12 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 11:
    tft.print("seda/la");
    tft.drawRoundRect(10, 10, ((6 * 7 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 12:
    tft.print("sintetica");
    tft.drawRoundRect(10, 10, ((6 * 9 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 13:
    tft.print("roupa preta");
    tft.drawRoundRect(10, 10, ((6 * 11 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 14:
    tft.print("roupa de bebe");
    tft.drawRoundRect(10, 10, ((6 * 13 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 15:
    tft.print("roupa intima");
    tft.drawRoundRect(10, 10, ((6 * 12 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 16:
    tft.print("jeans");
    tft.drawRoundRect(10, 10, ((6 * 5 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 17:
    tft.print("branco encardido");
    tft.drawRoundRect(10, 10, ((6 * 16 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 18:
    tft.print("pano de limpeza");
    tft.drawRoundRect(10, 10, ((6 * 15 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 19:
    tft.print("pesado sujo");
    tft.drawRoundRect(10, 10, ((6 * 11 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  case 20:
    tft.print("tapete");
    tft.drawRoundRect(10, 10, ((6 * 6 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    break;
  }

  tft.drawRoundRect(50, 60, ((6 * 20 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (60 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 1)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("molho longo+agitacao");

  tft.drawRoundRect(50, 90, ((6 * 14 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (90 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 2)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("molho+agitacao");

  tft.drawRoundRect(50, 120, ((6 * 20 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (120 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 3)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("molho curto+agitacao");

  tft.drawRoundRect(50, 150, ((6 * 8 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (150 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 4)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("agitacao");

  tft.drawRoundRect(50, 180, ((6 * 7 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (180 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 5)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("enxague");

  tft.drawRoundRect(50, 210, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (210 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 6)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("centrifugacao");

  tft.drawRoundRect(50, 240, ((6 * 3 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  tft.setCursor((50 + (2 * 3)), (240 + (1 * 3)));
  tft.setTextColor(WHITE);
  if (fase == 7)
  {
    tft.setTextColor(YELLOW);
  } //torna amarelo quando a máquina entrar nesta fase
  tft.setTextSize(3);
  tft.print("fim");
}
//==================================================

void testes()
{ //função que controla o touch da pagina 7 (de debug)
  //------------------------------------------
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  digitalWrite(XM, LOW);
  pinMode(YP, OUTPUT);
  digitalWrite(YP, HIGH);
  pinMode(YM, OUTPUT);
  digitalWrite(YM, LOW);
  pinMode(XP, OUTPUT);
  digitalWrite(XP, HIGH);
  //------------------------------------------
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE)
  {
    p.x = (map(p.x, TS_MINX, TS_MAXX, tft.height(), 0));
    p.y = (map(p.y, TS_MINY, TS_MAXY, tft.width(), 0));

    Serial.print("py: ");
    Serial.print(p.y);
    Serial.print(" px: ");
    Serial.println(p.x);
    if ((p.x >= 260) && (p.x <= 310) && (p.y >= 10) && (p.y <= 50))
    { //seta voltar
      paginaDebug--;
      if (paginaDebug < 1)
      {
        paginaDebug = 3;
      }
      noInterrupts();
      tft.fillScreen(BLACK); //apaga o que está na tela
      interrupts();
      displayDebug(paginaDebug); //escreve na tela a página anterior
    }
    if ((p.x >= 260) && (p.x <= 310) && (p.y >= 60) && (p.y <= 100))
    { //seta avançar
      paginaDebug++;
      if (paginaDebug > 3)
      {
        paginaDebug = 1;
      }
      noInterrupts();
      tft.fillScreen(BLACK); //apaga o que está na tela
      interrupts();
      displayDebug(paginaDebug); //escreve na tela a página posterior
    }
    if ((p.x >= 267) && (p.x <= 303) && (p.y >= 310) && (p.y <= 470))
    {                        //tecla voltar
      tft.fillScreen(BLACK); //apaga o que está na tela
      pagina = 1;
      ciclo = 0;
      agua = 0;
      enxague = false;
      passa = false;
      secagem = false;
      fase = 1;  //variavel que controla em qual ciclo a lavagem está
      fase2 = 0; //variavel que controla em qual ciclo a lavagem pode ser que esteja na próxima
      maquina = 0;
      pressao = 400;
      debug = 0;
      paginaDebug = 1;
      maquinaDebug = 0;
      maquinaLavar(false, false, false, false, false, false);
      modo(1);
      return;
    }
    if (paginaDebug == 1)
    {
      if ((p.x >= 60) && (p.x <= 87))
      {
        maquinaDebug = maquinaDebug ^ 32; //inverte o bit referente à eletrobomba
        displayDebug(1);
      }
      if ((p.x >= 90) && (p.x <= 117))
      {
        maquinaDebug = maquinaDebug ^ 16; //inverte o bit referente o freio
        displayDebug(1);
      }
      if ((p.x >= 120) && (p.x <= 147))
      {
        maquinaDebug = maquinaDebug ^ 8; //inverte o bit referente ao motor horário
        displayDebug(1);
      }
      if ((p.x >= 150) && (p.x <= 177))
      {
        maquinaDebug = maquinaDebug ^ 4; //inverte o bit referente ao motor anti-horário
        displayDebug(1);
      }
      if ((p.x >= 180) && (p.x <= 207))
      {
        maquinaDebug = maquinaDebug ^ 2; //inverte o bit referente À valvula principal
        displayDebug(1);
      }
      if ((p.x >= 210) && (p.x <= 237))
      {
        maquinaDebug = maquinaDebug ^ 1; //inverte o bit referente À valvula amaciante
        displayDebug(1);
      }
      maquinaLavar((maquinaDebug & 1), (maquinaDebug & 2), (maquinaDebug & 4), (maquinaDebug & 8), (maquinaDebug & 16), (maquinaDebug & 32)); //aciona a máquina com o valor que foi pedido
    }
    if (paginaDebug == 3)
    {
      if ((p.x >= 100) && (p.x <= 127))
      {
        velocidade = 1;
        Serial.println(velocidade);
        displayDebug(3);
      }
      if ((p.x >= 130) && (p.x <= 157))
      {
        velocidade = 2;
        Serial.println(velocidade);
        displayDebug(3);
      }
      if ((p.x >= 160) && (p.x <= 187))
      {
        velocidade = 3;
        Serial.println(velocidade);
        displayDebug(3);
      }
      if ((p.x >= 190) && (p.x <= 217))
      {
        velocidade = 5;
        Serial.println(velocidade);
        displayDebug(3);
      }
      if ((p.x >= 220) && (p.x <= 247))
      {
        velocidade = 10;
        Serial.println(velocidade);
        displayDebug(3);
      }
    }
  }
}

void displayDebug(int Pdebug)
{ //função que controla o display da pagina 7 (de debug)
  tft.fillTriangle(50, 260, 50, 310, 10, 285, 0x4E4C);
  tft.fillTriangle(60, 260, 60, 310, 100, 285, 0x4E4C);

  tft.drawRoundRect(310, 267, ((6 * 6 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), 0x7AFD);
  tft.setCursor((310 + (2 * 4)), (267 + (1 * 4)));
  tft.setTextColor(0x7AFD);
  tft.setTextSize(4);
  tft.print("voltar");

  tft.drawRoundRect(10, 10, ((6 * 5 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
  tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
  tft.setTextColor(WHITE);
  tft.setTextSize(4);
  tft.print("Debug");

  switch (Pdebug)
  {
  case 1:
    tft.drawRoundRect(10, 60, ((6 * 11 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE);
    if ((maquinaDebug & 32) == 32)
    {
      tft.setTextColor(YELLOW);
    }
    tft.setTextSize(3);
    tft.print("eletrobomba");

    tft.drawRoundRect(10, 90, ((6 * 5 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE);
    if ((maquinaDebug & 16) == 16)
    {
      tft.setTextColor(YELLOW);
    }
    tft.setTextSize(3);
    tft.print("freio");

    tft.drawRoundRect(10, 120, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE);
    if ((maquinaDebug & 8) == 8)
    {
      tft.setTextColor(YELLOW);
    }
    tft.setTextSize(3);
    tft.print("motor-horario");

    tft.drawRoundRect(10, 150, ((6 * 18 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (150 + (1 * 3)));
    tft.setTextColor(WHITE);
    if ((maquinaDebug & 4) == 4)
    {
      tft.setTextColor(YELLOW);
    }
    tft.setTextSize(3);
    tft.print("motor-anti-horario");

    tft.drawRoundRect(10, 180, ((6 * 17 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (180 + (1 * 3)));
    tft.setTextColor(WHITE);
    if ((maquinaDebug & 2) == 2)
    {
      tft.setTextColor(YELLOW);
    }
    tft.setTextSize(3);
    tft.print("Valvula principal");

    tft.drawRoundRect(10, 210, ((6 * 17 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (210 + (1 * 3)));
    tft.setTextColor(WHITE);
    if ((maquinaDebug & 1) == 1)
    {
      tft.setTextColor(YELLOW);
    }
    tft.setTextSize(3);
    tft.print("Valvula amaciante");
    break;
  case 2:
    tft.drawRoundRect(10, 60, ((6 * 15 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Pressostato:");

    tft.drawRoundRect(10, 90, ((6 * 17 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("Turbo secagem:");

    tft.drawRoundRect(10, 120, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("tampa:");
    break;
  case 3:
    tft.drawRoundRect(10, 70, ((6 * 10 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((10 + (2 * 3)), (70 + (1 * 3)));
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("velocidade");

    tft.drawRoundRect(40, 100, ((6 * 2 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((40 + (2 * 3)), (100 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (velocidade == 1)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("1x");

    tft.drawRoundRect(40, 130, ((6 * 2 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((40 + (2 * 3)), (130 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (velocidade == 2)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("2x");

    tft.drawRoundRect(40, 160, ((6 * 2 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((40 + (2 * 3)), (160 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (velocidade == 3)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("3x");

    tft.drawRoundRect(40, 190, ((6 * 2 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((40 + (2 * 3)), (190 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (velocidade == 5)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("5x");

    tft.drawRoundRect(40, 220, ((6 * 3 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
    tft.setCursor((40 + (2 * 3)), (220 + (1 * 3)));
    tft.setTextColor(WHITE);
    if (velocidade == 10)
    {
      tft.setTextColor(YELLOW);
    } //torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("10x");
    break;
  }
}
void sensor()
{ //função que controla o dysplay de valores vindos de portas externas para pagina 7 (de debug)
  if ((pagina == 7) && paginaDebug == 2)
  {
    tft.setCursor(((10 + ((6 * 12 * 3) + (2 * 3))) + (2 * 3)), (60 + (1 * 3)));
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(3);
    tft.print(pressostato());
    tft.drawRoundRect(10, 60, ((6 * 15 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);

    tft.setCursor((10 + (6 * 14 * 3) + (2 * 3)), (90 + (1 * 3)));
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(3);
    if (digitalRead(Tsecagem) == HIGH)
    {
      tft.print("sim");
    }
    else
    {
      tft.print("nao");
    }
    tft.drawRoundRect(10, 90, ((6 * 17 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);

    tft.setCursor((10 + (6 * 6 * 3) + (2 * 3)), (120 + (1 * 3)));
    tft.setTextColor(WHITE, BLACK);
    tft.setTextSize(3);
    if (digitalRead(tampa) == HIGH)
    {
      tft.print("aberta");
    }
    else
    {
      tft.print("fechada");
    }
    tft.drawRoundRect(10, 120, ((6 * 13 * 3) + (4 * 3)), ((7 * 3) + (2 * 3)), (1 * 3), WHITE);
  }
}
//==================================================
void maquinaLavar(bool eletrobomba, bool freio, bool motorH, bool motorA, bool VPrincipal, bool VAmaciante)
{                 //função que controla a máguina de lavar a partir do shift register
  noInterrupts(); //aqui não é abilitado interrupções pois é onde ocorre a comunicação com o shift register
  maquina = ((VAmaciante) + (VPrincipal * 2) + (motorA * 4) + (motorH * 8) + (freio * 16) + (eletrobomba * 32));

  digitalWrite(enableR, LOW); //desliga o pino enable do register
  digitalWrite(enableF, LOW); //desliga o pino dde enable que fica depois do register

  digitalWrite(info, VAmaciante); //escreve no pino a informação que está na variável
  digitalWrite(enableR, HIGH);    //liga o pino enable do register
  digitalWrite(enableR, LOW);     //desliga o pino enable do register

  digitalWrite(info, VPrincipal); //escreve no pino a informação que está na variável
  digitalWrite(enableR, HIGH);    //liga o pino enable do register
  digitalWrite(enableR, LOW);     //desliga o pino enable do register

  digitalWrite(info, motorH);  //escreve no pino a informação que está na variável
  digitalWrite(enableR, HIGH); //liga o pino enable do register
  digitalWrite(enableR, LOW);  //desliga o pino enable do register

  digitalWrite(info, motorA);  //escreve no pino a informação que está na variável
  digitalWrite(enableR, HIGH); //liga o pino enable do register
  digitalWrite(enableR, LOW);  //desliga o pino enable do register

  digitalWrite(info, freio);   //escreve no pino a informação que está na variável
  digitalWrite(enableR, HIGH); //liga o pino enable do register
  digitalWrite(enableR, LOW);  //desliga o pino enable do register

  digitalWrite(info, eletrobomba); //escreve no pino a informação que está na variável
  digitalWrite(enableR, HIGH);     //liga o pino enable do register
  digitalWrite(enableR, LOW);      //desliga o pino enable do register

  digitalWrite(enableF, HIGH); //liga o pino dde enable que fica depois do register
  digitalWrite(enableF, LOW);  //desliga o pino dde enable que fica depois do register
  interrupts();
}

int pressostato()
{                   //função que retorna o valor do pressostato em mm
  float coluna = 0; //variável que guarda o valor da coluna d'água em milímetros
  coluna = analogRead(pres);
  Serial.print(coluna);
  //  coluna = map(coluna, 0, 1023, 0, 5);//convertendo o valor lido na porta (0 a 1023) para 0-5V
  // Serial.print(coluna);
  coluna = map(coluna, 204.6, 1023, 0, 400); //convertendo o valor de 204,6(1V)-1023(5V) que o sensor dará para um valor de 0-400mm
  Serial.println("");
  Serial.print("coluna d'agua (mm): ");
  Serial.print(coluna);
  return coluna; //retorna o valor do mapeamento da variável coluna, que está na escala de 1 a 5, para um valor de 0 a 400
}

void erros()
{ //função que determina se houve um erro no processo de lavagem
  if (pagina == 6)
  { //se está na página que mostra a linha do tempo(última página)
    bool tampaAberta = digitalRead(tampa);
    if (tampaAberta == true)
    {                                                         //se a tampa está aberta
      int estadoAnterior = maquina;                           //guarda o valor de maquina para estadoAnterior
      maquinaLavar(false, false, false, false, false, false); //desliga tudo da máquina de lavar
      telaErro(1);
      while (tampaAberta == true)
      { //espera a´te que a tampa esteja fechada
      }
      linhaTempo(fase);
      maquinaLavar((estadoAnterior & 1), (estadoAnterior & 2), (estadoAnterior & 4), (estadoAnterior & 8), (estadoAnterior & 16), (estadoAnterior & 32)); //religa o que estava ligado a partir do valor da variável estadoAnterior
    }
    if (maquina & 32 == 0)
    { //se não está ligada a drenagem
      if (pressostato() > (pressao + 15))
      {                                                        //se o nível de água abaixou
        int estadoAnterior = maquina;                          //guarda o valor de maquina para estadoAnterior
        maquinaLavar(true, false, false, false, false, false); //liga a drenagem
        telaErro(4);
        while (pressostato() > (pressao + 15))
        {
        } //espera até que o nível de água volte ao normal
        linhaTempo(fase);
        maquinaLavar((estadoAnterior & 1), (estadoAnterior & 2), (estadoAnterior & 4), (estadoAnterior & 8), (estadoAnterior & 16), (estadoAnterior & 32)); //religa o que estava ligado a partir do valor da variável estadoAnterior
      }
      pressao = pressostato(); //guarda valor da coluna d'água para usar na próxima chegagem
    }
  }
}

void telaErro(int erro)
{ //função que escreve na tela o erro que houve no processo de laavagem

  tft.fillScreen(BLACK);

  switch (erro)
  {
  case 1:
    tft.drawRoundRect(10, 10, ((6 * 12 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("porta aberta");

  case 2:
    maquinaLavar(false, false, false, false, false, false);
    tft.drawRoundRect(10, 10, ((6 * 24 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("enchimento demorou muito");
    while (1 == 2)
    {
    } //para

  case 3:
    maquinaLavar(false, false, false, false, false, false);
    tft.drawRoundRect(10, 10, ((6 * 22 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("esvaziar-demorou muito");

  case 4:
    tft.drawRoundRect(10, 10, ((6 * 22 * 4) + (4 * 4)), ((7 * 4) + (2 * 4)), (1 * 4), WHITE);
    tft.setCursor((10 + (2 * 4)), (10 + (1 * 4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("drenando agua sobrando");
  }
}
