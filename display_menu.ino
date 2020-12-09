#include <MCUFRIEND_kbv.h>
#include <Adafruit_GFX.h> //Biblioteca grafica
#include <TouchScreen.h>  //Biblioteca Touch
#include <TimerThree.h> //Biblioteca para gerenciar o timer 3 do arduino mega

#define YP A3 // Y+ is on Analog1
#define XM A2 // X- is on Analog2
#define YM 9 // Y- is on Digital7
#define XP 8 // X+ is on Digital6

#define TS_MINX 108
#define TS_MINY 93
#define TS_MAXX 903
#define TS_MAXY 955

#define LCD_RESET A4 //Pode ser conectado ao pino reset do Arduino
#define LCD_CS A3   // Chip Select
#define LCD_CD A2  // Command/Data
#define LCD_WR A1  // LCD Write
#define LCD_RD A0 // LCD Read

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

MCUFRIEND_kbv tft;

#define MINPRESSURE 10
#define MAXPRESSURE 1000

//Definicao de cores
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

int pagina = 1;
int ciclo = 0;
int agua = 0;
bool enxague = false;
bool passa = false;
bool secagem = false;
int fase = 1;//variavel que controla em qual ciclo a lavagem está
int fase2 = 0; //variavel que controla em qual ciclo a lavagem pode ser que esteja na próxima
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
int tabela[21][19]{//21-> como começa do 0, para facilitar é usado 21 para 20 elementos pulando o 0. 19->existem 19 termos em cada ciclos que não foram generalizados e precisam ficar na lista
{},
{102,200,102,205,102,200,102,205,104,206,101,206,103,200,103,200,310,306,318},
{102,200,102,205,104,205,101,205,104,205,101,205,103,200,103,200,312,308,320},
{102,200,102,205,102,204,102,204,102,204,102,203,101,200,101,200,310,306,318},
{102,200,102,205,104,205,101,205,104,205,101,205,103,200,103,200,312,308,320},
{102,200,102,205,104,205,101,205,104,205,101,205,103,200,103,200,312,308,320},
{103,211,103,204,103,205,104,216,104,209,104.5,206,103,200,101,200,312,306,318},
{104,210,103,210,103,210,103,210,103,210,103,210,103,200,104,200,310,306,318},
{102,200,102,205,104,210,102,210,101,200,101,210,103,200,102,200,310,306,318},
{102,200,102,220,104,210,104,210,102,200,102,210,103,200,103,200,310,306,318},
{100.5,200,100.5,200,100.5,200,100.5,200.5,100.5,200,100.5,200,101,200,101,200,307,306,314.5},
{102,200,102,210,102.5,210,102.5,210,102.5,210,102.5,210,104,200,104,200,312,306,320},
{105,206,104,212,103,212,101.5,210,102,206,102,218,104,218,106,212,312,308,320},
{102,200,102,210,102.5,210,102.5,210,102.5,210,102.5,210,104,200,104,200,312,308,320},
{104,210,103,210,103,210,103,210,103,210,103,210,104,200,104,200,312,308,320},
{102,200,102,205,102,200,102,205,107,210,107,210,104,200,104,200,312,308,320},
{102,200,102,205,102,200,102,210,101,210,101,210,102,200,102,200,400,306,318},
{102,200,102,205,102,200,102,204,101,208,101,208,102,200,102,200,400,306,318},
{102,200,102,205,102,200,102,205,105,206,102,206,102,200,101,200,308,304,316},
{102,200,102,205,102,200,102,205,106,208,104,206,102,200,102,200,308,304,316},
{102,200,102,205,102,200,102,205,105,210,102,210,102,200,101,200,500,304,600},
};

void setup()
{
  Serial.begin(9600);
  Timer3.initialize(1000000);//inicializa o Timer3 com periodo de 1segundo
  Timer3.attachInterrupt(timerRoutine);//timerRoutine() agora é uma função que faz uma interrupção no código a cada 1 segundo
 
  tft.reset();
  tft.begin(tft.readID());
  tft.fillScreen(BLACK);
  tft.setRotation(1); 

  modo(1);
}
void loop()
{
menu();
  if (pagina == 6){
   lavagem();
  }
}

void menu(){
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
    if(pagina<=4){
     if((p.x >= 260) && (p.x <= 310) && (p.y >= 10)&& (p.y <= 50)){//seta voltar
       pagina --;
       if (pagina < 1) {
         pagina = 4;
       }
       ciclo = 0;
       modo(pagina);
     }
     if((p.x >= 260) && (p.x <= 310) && (p.y >= 60)&& (p.y <= 100)){//seta avançar
       pagina ++;
       if (pagina > 4) {
         pagina = 1;
       }
       ciclo = 0;
       modo(pagina);
     }
     if((p.x>=60) && (p.x <= 87)){
       ciclo = (1+((pagina-1)*5));
       Serial.println(ciclo);
       modo(pagina);
     }
     if((p.x>=90) && (p.x <= 117)){
       ciclo = (2+((pagina-1)*5));
       Serial.println(ciclo);
       modo(pagina);
     }
     if((p.x>=120) && (p.x <= 147)){
       ciclo = (3+((pagina-1)*5));
       Serial.println(ciclo);
       modo(pagina);
     }
     if((p.x>=150) && (p.x <= 177)){
       ciclo = (4+((pagina-1)*5));
       Serial.println(ciclo);
       modo(pagina);
     }
     if((p.x>=180) && (p.x <= 207)){
       ciclo = (5+((pagina-1)*5));
       Serial.println(ciclo);
       modo(pagina);
     }
     if((ciclo == 6) || (ciclo == 13)){//nos ciclos tenis(6) e roupa preta(9) duplo enxague está sempre ativo
      enxague = true;
     }
     if (ciclo == 6){//no ciclo tênis(6), o nível de água é sempre baixo
      (agua = 1);
     }
     if((p.x >= 267) && (p.x <= 303) && (p.y >= 310) && (p.y <= 470)&&(ciclo != 0)){ //tecla opções
       con();
       delay(1000);
       pagina = 5;
       return;//este return é para sair desta função quando for apertado o botão "opções".
       //Isso é necessário pois como o botão está no mesmo lugar do outro botão para continuar, quando é apertado o primeiro,o segundo também pode ser ativado,
       //devio ao fato de estar logo embaixo no código e só foi feita uma vez a verificação de posição do touch
     }
    }
    if(pagina == 5){
     if((p.x >= 267) && (p.x <= 303) && (p.y >= 10)&& (p.y <= 170)){//tecla ciclos
      pagina = 1;
      ciclo = 0;
      agua = 0;
      enxague = false;
      passa = false;
      modo(pagina);
     }
     if(ciclo != 6){//no ciclo têncis(6), o nível baixo de água é sempre ativo
     if((p.x>=100) && (p.x <= 127) && (p.y <= 256) && (ciclo !=8)){//ciclo cobertor/manta(8) não possui seleção automática de água
       agua = 5;
       Serial.println(agua);
       con();
     }
     if((p.x>=130) && (p.x <= 157) && (p.y <= 256)){
       agua = 4;
       Serial.println(agua);
       con();
     }
     if((p.x>=160) && (p.x <= 187) && (p.y <= 256)){
       agua = 3;
       Serial.println(agua);
       con();
     }
     if((p.x>=190) && (p.x <= 217) && (p.y <= 256)){
       agua = 2;
       Serial.println(agua);
       con();
     }
     if((p.x>=220) && (p.x <= 247) && (p.y <= 256)){
       agua = 1;
       Serial.println(agua);
       con();
     }
     }
     if((p.y>=332) && (p.y <= 470) && (p.x >= 70) && (p.x <= 124)){ //tecla de duplo enxague
       if((ciclo != 6) && (ciclo != 13)){//nos ciclos tenis(6) e roupa preta(9) duplo enxague está sempre ativo
       enxague = (!enxague);
       Serial.println(enxague);
       con();
       }
     }
     if((p.y>=332) && (p.y <= 470) && (p.x >= 150) && (p.x <= 204)){//tecla de passa fácil
       passa = (!passa);
       Serial.println(passa);
       con();
     }
     if((p.y>=238) && (p.y <= 470) && (p.x >= 267) && (p.x <= 303) && (agua != 0)){//tecla continuar
      Serial.println("");
      Serial.print("ciclo: ");
      Serial.println(ciclo);
      Serial.print("nível da água: ");
      Serial.println(agua);
      Serial.print("duplo enxague: ");
      Serial.println(enxague);
      Serial.print("passa fácil: ");
      Serial.println(passa);
      Serial.println("===============================");
      tft.fillScreen(BLACK);
      linhaTempo(0);
      pagina = 6;
      }
    }
    
  }
}

void modo (int pag){

   switch (pag){
    case 1:
    tft.fillScreen(BLACK);
    tft.drawRoundRect(10, 10, ((6*13*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    tft.setCursor((10 + (2*4)), (10 + (1*4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Normal");
  
    tft.drawRoundRect(10, 60, ((6*5*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (60 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 1){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Calca");

    tft.drawRoundRect(10, 90, ((6*7*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (90 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 2){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Moleton");

    tft.drawRoundRect(10, 120, ((6*8*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (120 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 3){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Camiseta");

    tft.drawRoundRect(10, 150, ((6*6*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (150 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 4){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("toalha");

    tft.drawRoundRect(10, 180, ((6*13*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (180 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 5){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("roupa de cama");
    break;

    case 2:
    tft.fillScreen(BLACK);
    tft.drawRoundRect(10, 10, ((6*15*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    tft.setCursor((10 + (2*4)), (10 + (1*4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Especial");
  
    tft.drawRoundRect(10, 60, ((6*5*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (60 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 6){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Tenis");

    tft.drawRoundRect(10, 90, ((6*11*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (90 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 7){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Tira Mancha");

    tft.drawRoundRect(10, 120, ((6*16*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (120 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 8){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Edredon/Cobertor");

    tft.drawRoundRect(10, 150, ((6*13*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (150 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 9){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa de mesa");

    tft.drawRoundRect(10, 180, ((6*12*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (180 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 10){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Rapido 25min");
    break;
    
    case 3:
    tft.fillScreen(BLACK);
    tft.drawRoundRect(10, 10, ((6*15*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    tft.setCursor((10 + (2*4)), (10 + (1*4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Delicado");
  
    tft.drawRoundRect(10, 60, ((6*7*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (60 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 11){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Seda/La");

    tft.drawRoundRect(10, 90, ((6*9*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (90 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 12){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Sintetica");

    tft.drawRoundRect(10, 120, ((6*11*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (120 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 13){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa preta");

    tft.drawRoundRect(10, 150, ((6*13*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (150 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 14){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa de bebe");

    tft.drawRoundRect(10, 180, ((6*12*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (180 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 15){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Roupa intima");
    break;

    case 4:
    tft.fillScreen(BLACK);
    tft.drawRoundRect(10, 10, ((6*12*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    tft.setCursor((10 + (2*4)), (10 + (1*4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("Modo - Turbo");
  
    tft.drawRoundRect(10, 60, ((6*5*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (60 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 16){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Jeans");

    tft.drawRoundRect(10, 90, ((6*16*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (90 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 17){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Branco encardido");

    tft.drawRoundRect(10, 120, ((6*15*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (120 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 18){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("Pano de limpeza");

    tft.drawRoundRect(10, 150, ((6*11*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (150 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 19){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("pesado sujo");

    tft.drawRoundRect(10, 180, ((6*6*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (180 + (1*3)));
    tft.setTextColor(WHITE);
    if (ciclo == 20){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("tapete");
    break;
   }
  tft.fillTriangle(50, 260, 50, 310, 10, 285, 0x4E4C);
  tft.fillTriangle(60, 260, 60, 310, 100, 285, 0x4E4C);
  if(ciclo != 0){   
    tft.drawRoundRect(310, 267, ((6*6*4)+(4*4)), ((7*4)+ (2*4)), (1*4),0x7AFD);
    tft.setCursor((310 + (2*4)), (267 + (1*4)));
    tft.setTextColor(0x7AFD);
    tft.setTextSize(4);
    tft.print("opcoes");
  }
}

void con(){
    tft.fillScreen(BLACK);
    
    tft.drawRoundRect(10, 10, ((6*13*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    tft.setCursor((10 + (2*4)), (10 + (1*4)));
    tft.setTextColor(WHITE);
    tft.setTextSize(4);
    tft.print("configuracoes");

    tft.drawRoundRect(10, 70, ((6*13*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((10 + (2*3)), (70 + (1*3)));
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("nivel de agua");

    tft.drawRoundRect(40, 100, ((6*10*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE); 
    tft.setCursor((40 + (2*3)), (100 + (1*3)));
    tft.setTextColor(WHITE);
    if (agua == 5){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    if (ciclo == 8){ tft.setTextColor(0xB596);}//ciclo ccobertor/manta(8) não possui seleção automática de água
    tft.setTextSize(3);
    tft.print("automatico");

    tft.drawRoundRect(40, 130, ((6*7*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((40 + (2*3)), (130 + (1*3)));
    tft.setTextColor(WHITE);
    if (agua == 4){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("edredon");

    tft.drawRoundRect(40, 160, ((6*4*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((40 + (2*3)), (160 + (1*3)));
    tft.setTextColor(WHITE);
    if (agua == 3){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("alto");

    tft.drawRoundRect(40, 190, ((6*5*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((40 + (2*3)), (190 + (1*3)));
    tft.setTextColor(WHITE);
    if (agua == 2){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("medio");

    tft.drawRoundRect(40, 220, ((6*5*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((40 + (2*3)), (220 + (1*3)));
    tft.setTextColor(WHITE);
    if (agua == 1){ tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("baixo");

    tft.drawRoundRect(332, 70, ((6*7*3)+(4*3)), (2*((7*3)+ (2*3))), (1*3),WHITE); 
    tft.setCursor((332 + (2*3)+ (6*3)), (70 + (1*3)));
    tft.setTextColor(WHITE);
    if ((enxague == true) || (ciclo == 6) || (ciclo == 13)){tft.setTextColor(YELLOW);}//torna amarelo se for clicado ou se ciclo = 6 ou 9, pois nos ciclos tenis(6) e roupa preta(9) duplo enxague está sempre ativo
    tft.setTextSize(3);
    tft.print("duplo");
    tft.setCursor((332 + (2*3)), (70 + (1*3)+(7*3)+ (2*3)));
    tft.print("enxague");

    tft.drawRoundRect(332, 150, ((6*7*3)+(4*3)), (2*((7*3)+ (2*3))), (1*3),WHITE); 
    tft.setCursor((332 + (2*3)+ (6*3)), (150 + (1*3)));
    tft.setTextColor(WHITE);
    if (passa == true){tft.setTextColor(YELLOW);}//torna amarelo se for clicado
    tft.setTextSize(3);
    tft.print("passa");
    tft.setCursor((332 + (2*3)+ (6*3)), (150 + (1*3)+(7*3)+ (2*3)));
    tft.print("facil");
     
    tft.drawRoundRect(10, 267, ((6*6*4)+(4*4)), ((7*4)+ (2*4)), (1*4),0x7AFD);
    tft.setCursor((10 +   (2*4)), (267 + (1*4)));
    tft.setTextColor(0x7AFD);
    tft.setTextSize(4);
    tft.print("ciclos");

    if(agua != 0){   
    tft.drawRoundRect(238, 267, ((6*9*4)+(4*4)), ((7*4)+ (2*4)), (1*4),0x7AFD);
    tft.setCursor((238 +   (2*4)), (267 + (1*4)));
    tft.setTextColor(0x7AFD);
    tft.setTextSize(4);
    tft.print("Continuar");
    }
}

void linhaTempo(int fase){
   tft.setCursor((10 + (2*4)), (10 + (1*4)));
   tft.setTextColor(WHITE);
   tft.setTextSize(4);
    
   switch (ciclo){
    case 1:
    tft.print("calca");
    tft.drawRoundRect(10, 10, ((6*5*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 2:
    tft.print("moleton");
    tft.drawRoundRect(10, 10, ((6*9*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 3:
    tft.print("camiseta");
    tft.drawRoundRect(10, 10, ((6*8*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 4:
    tft.print("toalha");
    tft.drawRoundRect(10, 10, ((6*69*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 5:
    tft.print("roupa de cama");
    tft.drawRoundRect(10, 10, ((6*13*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 6:
    tft.print("tenis");
    tft.drawRoundRect(10, 10, ((6*5*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 7:
    tft.print("tira mancha");
    tft.drawRoundRect(10, 10, ((6*11*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 8:
    tft.print("edrendon/cobertor");
    tft.drawRoundRect(10, 10, ((6*17*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 9:
    tft.print("roupa de mesa");
    tft.drawRoundRect(10, 10, ((6*13*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 10:
    tft.print("rapido 25min");
    tft.drawRoundRect(10, 10, ((6*12*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 11:
    tft.print("seda/la");
    tft.drawRoundRect(10, 10, ((6*7*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 12:
    tft.print("sintetica");
    tft.drawRoundRect(10, 10, ((6*9*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 13:
    tft.print("roupa preta");
    tft.drawRoundRect(10, 10, ((6*11*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 14:
    tft.print("roupa de bebe");
    tft.drawRoundRect(10, 10, ((6*13*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 15:
    tft.print("roupa intima");
    tft.drawRoundRect(10, 10, ((6*12*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 16:
    tft.print("jeans");
    tft.drawRoundRect(10, 10, ((6*5*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 17:
    tft.print("branco encardido");
    tft.drawRoundRect(10, 10, ((6*16*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 18:
    tft.print("pano de limpeza");
    tft.drawRoundRect(10, 10, ((6*15*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 19:
    tft.print("pesado sujo");
    tft.drawRoundRect(10, 10, ((6*11*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    case 20:
    tft.print("tapete");
    tft.drawRoundRect(10, 10, ((6*6*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
    break;
    }
    
    tft.drawRoundRect(50, 60, ((6*20*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (60 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 1){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("molho longo+agitacao");

    tft.drawRoundRect(50, 90, ((6*14*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (90 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 2){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("molho+agitacao");

    tft.drawRoundRect(50, 120, ((6*20*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (120 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 3){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("molho curto+agitacao");

    tft.drawRoundRect(50, 150, ((6*8*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (150 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 4){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("agitacao");

    tft.drawRoundRect(50, 180, ((6*7*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (180 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 5){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("enxague");

    tft.drawRoundRect(50, 210, ((6*13*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (210 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 6){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("centrifugacao");

    tft.drawRoundRect(50, 240, ((6*3*3)+(4*3)), ((7*3)+ (2*3)), (1*3),WHITE);
    tft.setCursor((50 + (2*3)), (240 + (1*3)));
    tft.setTextColor(WHITE);
    if (fase == 7){ tft.setTextColor(YELLOW);}//torna amarelo quando a máquina entrar nesta fase
    tft.setTextSize(3);
    tft.print("fim");
}

void EC(){
  delay(1500);
  }
void A(){
  delay(1500);
  }
void B(){
  delay(1500);
  }
void C(){
  delay(1500);
  }
void D(){
  delay(1500);
  }
void E(){
  delay(1500);
  }
void F_(){
  delay(1500);
  } //por algum motivo void F() dá erro ao compitar, por isso void F_()
void FR(){
  delay(1500);
  }
void G(){
  delay(1500);
  }
void H(){
  delay(1500);
  }
void HR(){
  delay(1500);
  }
void I(){
  delay(1500);
  }
void J(){
  delay(1500);
  }
void K(){
  delay(1500);
  }
void L(){
  delay(1500);
  }
void agitacao(int tempo){
  delay(1500);
  }
void molho(int tempo){
  delay(1500);
  }
void centrifugacao(int tempo){
  delay(1500);
  }
  
void lavagem(){
  EC(); //<-todos os programas precisam no inico
  //molho longo+agitação
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
    
  if(fase == 1){//esta verificação ocorre para ser possível avançar e pular partes da lavagem
    Serial.println("molho longo+agitação");
    linhaTempo(fase);
    fase ++;    
    agitacao(tabela[ciclo][0]%100);
    molho(tabela[ciclo][1]%100);
    agitacao(tabela[ciclo][2]%100);
    molho(tabela[ciclo][3]%100);
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
  if(fase == 2){ //molho+agitação
    Serial.println("molho+agitação");
    linhaTempo(fase);
    fase ++;
    agitacao(tabela[ciclo][4]%100);
    molho(tabela[ciclo][5]%100);
    agitacao(tabela[ciclo][6]%100);
    molho(tabela[ciclo][7]%100);
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
  if(fase == 3){ //molho curto+agitação
    Serial.println("molho curto+agitação");
    linhaTempo(fase);
    fase ++;
    agitacao(tabela[ciclo][8]%100);
    molho(tabela[ciclo][9]%100);
    agitacao(tabela[ciclo][10]%100);
    molho(tabela[ciclo][11]%100);
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
  if(fase == 4){ //agitação
    Serial.println("agitação");
    linhaTempo(fase);
    fase ++;
    agitacao(tabela[ciclo][12]%100);
    molho(tabela[ciclo][13]%100);
    agitacao(tabela[ciclo][14]%100);
    molho(tabela[ciclo][15]%100);
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
  if(fase == 5){ //Enxague
    Serial.println("Enxague");
    linhaTempo(fase);
    fase ++;
    //esta parte é igual para todos os ciclos:
    A();
    B();
    C();
    D();
    E();
    if(enxague == true){//esta parte só é acionada no dupo enxague e nos ciclos roupa preta e tenis, onde o duplo enxague é sempre ativo
      EC();
      agitacao(2.5);
      A();
      I();
      E();
    }
    if(ciclo == 10){//para generalizar,´é necessário introduzir esta exeção pois todos os ciclos passam por F_(); e agitacao(2);, menos o "rápido 25min", que a lavagem passa por F(); e agitacao(1);
      FR();
      agitacao(1);
    }
    else{
      F_();
      agitacao(2);
    }
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
  if(fase == 6){//Centrifugação
    Serial.println("Centrifugação");
    linhaTempo(fase);
    fase ++;
    A();
    B();
    if(ciclo != 10){//para generalizar,´é necessário introduzir esta exeção pois todos os ciclos passam por K();, menos o "rápido 25min", que que pula esta parte
      K(); 
    }
    if (passa == false && secagem == false){
    if((tabela[ciclo][16]/100) == 5){//isso significa que é para fazer a função G();
      G();
    }
    if((tabela[ciclo][16]/100) == 4){//isso significa que é para fazer a função J();
      J();
    }
    if((tabela[ciclo][16]/100) == 3){//isso significa que é para fazer a função agitacao(int tempo);
      agitacao(tabela[ciclo][16]%100);//executa a função agitacao com o valor dos dois ultimos digitos do valor numero 16 da tabela
    }
    }
    //esta parte do programa é referente aos modos passa fácil e turbo secagem e a função executada com o item numero 18 da lista:
    if((passa == true) && (secagem == false)){
      centrifugacao(tabela[ciclo][17]%100);
    }
    if((passa == false) && (secagem == true)){
      if(ciclo == 20){//ciclo seda/lâ executa a função L() em vez de centrifugação
        L();
      }
      else{
        centrifugacao(tabela[ciclo][18]%100);
      } 
    }
    
    if(ciclo == 10){//para generalizar,´é necessário introduzir esta exeção pois todos os ciclos passam por H();, menos o "rápido 25min", que passa por HR();
      HR(); 
    }
    else{
      H();
    }
    E();
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
  if(fase == 7){//fim
    Serial.println("fim");
    linhaTempo(fase);
  }
  if (fase2 != 0){
    tft.fillRect(10, 60, 30, 240, BLACK);
    fase = fase2;
    fase2 = 0;
    }
}

void timerRoutine(){
  Serial.println(millis());
  Serial.println(fase);
  Serial.println(fase2);
  if (pagina == 6){//se está na página que mostra a linha do tempo(última página)
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
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE){//se o monitor for tocado
      p.x = (map(p.x, TS_MINX, TS_MAXX, tft.height(), 0)); //mapeia X para estar de acordo com as coordenadas da tela
      p.y = (map(p.y, TS_MINY, TS_MAXY, tft.width(), 0));  //mapeia Y para estar de acordo com as coordenadas da tela

      if((p.x >= 60) && (p.x<= 88)){//se foi tocado em "molho longo+agitação"    
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (60 + 14), 13, WHITE);
        fase2 = 1;//faz ir para a fase 1, para que seja executado o "molho longo+agitação"
      }
      if((p.x >= 90) && (p.x<= 118)){//se foi tocado em "molho+agitação"
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (90 + 14), 13, WHITE);
        fase2 = 2;//faz ir para a fase 2, para que seja executado o "molho+agitação"
      }
      if((p.x >= 120) && (p.x<= 148)){//se foi tocado em "molho curto+agitação"
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (120 + 14), 13, WHITE);
        fase2 = 3;//faz ir para a fase 3, para que seja executado o "molho curto+agitação"
      }
      if((p.x >= 150) && (p.x<= 178)){//se foi tocado em "agitação"
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (150 + 14), 13, WHITE);
        fase2 = 4;//faz ir para a fase 4, para que seja executado o "agitação"
      }
      if((p.x >= 180) && (p.x<= 208)){//se foi tocado em "enxague"
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (180 + 14), 13, WHITE);
        fase2 = 5;//faz ir para a fase 5, para que seja executado o "enxague"
      }
      if((p.x >= 210) && (p.x<= 238)){//se foi tocado em "centrifugação"
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (210 + 14), 13, WHITE);
        fase2 = 6;//faz ir para a fase 6, para que seja executado o "centrifugação"
      }
      if((p.x >= 240) && (p.x<= 298)){//se foi tocado em "centrifugação"
        tft.fillRect(10, 60, 30, 240, BLACK);
        tft.fillCircle(10 + 14, (240 + 14), 13, WHITE);
        fase2 = 7;//faz ir para a fase 6, para que seja executado o "centrifugação"
      }
    }
  }
  Serial.println(millis());
}
