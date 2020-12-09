# upgrade-maquina-de-lavar
transformando uma máquina de lavar Eletrolux LS12 usando 3.5" TFT LCD shield em um arduino mega 2560

Coisas úteis para lembrar:

Quando for escrever alguma coisa com uma caixa envolta:

  tft.drawRoundRect(x, y, ((6*nºLetras*tamanho)+(4*tamanho)), ((7*tamanho)+ (2*tamanho)), (1*tamanho),WHITE);
 
 tft.setCursor((X + (2*tamanho)), (Y + (1*tamanho)))

ex:
 
 tft.drawRoundRect(10, 10, ((6*13*4)+(4*4)), ((7*4)+ (2*4)), (1*4),WHITE);
 
 tft.setCursor((10 + (2*4)), (10 + (1*4)));
 
Link útil para ter como exemplo o uso das bibliotecas:
https://www.arduinoecia.com.br/display-tft-touch-3-5-para-arduino/
