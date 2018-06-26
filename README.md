# Projeto-Primeiro-Semestre

C�sar Abraham Flores Cisneros Filho, 14.00800-9

Guilherme Mucciolo Davan�o, 15.00020-6

Vinnicius Bernacchi de Souza, 15.00437-6

Nesse projeto, construiremos um vaso inteligente capaz de automaticamente regar a planta quando a umidade da terra estiver baixa

Utilizaremos os seguintes componentes:

* Display OLED 128X32, que ser� utilizado para exibir dados atuais da planta, tais como temperatura e umidade;
* Bomba de �gua com ponte-H, usada para regar a planta quando acionada;
* M�dulo Bluetooth, utilizado para enviar avisos e informa��es ao celular do dono;
* Sensor de umidade do solo SKU:SEN0114;
* Sensor de luz (LDR);
* Sensor de umidade/temperatura do ar;
* Aceler�metro, utilizado para detectar presen�a de ventos fortes;
* Microcontrolador ATSAM4SD32C com a placa SAM4S-EK2;

Obteremos as informa��es dos sensores e exibiremos no display OLED. 

Caso a temperatura saia do intervalo de 20 a 26�C, um aviso de temperatura inadequada ser� enviado para o m�dulo Bluetooth.

Caso a luminosidade seja menor que 30% por mais de 8 horas seguidas, ativaremos um LED de alto brilho para iluminar a planta, e um alerta de baixa luminosidade cont�nua ser� enviado, dizendo que o LED foi aceso. Quando a luminosidade passar de 40%, ou ficar ligado por mais de 8 horas seguidas, o LED ser� desligado.

Quando a umidade do solo atingir certo limiar acionaremos a bomba d��gua via PWM para regar a planta. Al�m disso, haver� um comando para acionamento manual da bomba via bluetooth.

A bomba � desacionada quando a umidade passar de determinado valor m�ximo.

Caso o aceler�metro apresente uma varia��o na acelera��o maior que 0.5g em qualquer eixo, ou uma inclina��o anormal do girosc�pio, um alerta de alta movimenta��o ser� enviado para o m�dulo bluetooth. Isso pode indicar tanto a presen�a de ventos fortes, que podem danificar a planta, como tamb�m poss�vel furto ou acidente.

Ao enviar um comando via bluetooth, todas as medi��es atuais ser�o enviadas para o usu�rio.

Caso haja presen�a de vento forte, indicado pelo aceler�metro e girosc�pio, em conjunto com baixa umidade do ar, a umidade m�xima liberada pela bomba ser� aumentada, pois ventos fortes e secos intensificam a evapotranspira��o, aumentando a demanda da planta por �gua.

As medi��es ser�o atualizadas de 10 em 10 minutos com a bomba n�o acionada. Quando a bomba for acionada, a medi��o da umidade do solo ser� realizada a cada 1 segundo, para maior precis�o no desacionamento da bomba.
