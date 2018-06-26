# Projeto-Primeiro-Semestre

César Abraham Flores Cisneros Filho, 14.00800-9

Guilherme Mucciolo Davanço, 15.00020-6

Vinnicius Bernacchi de Souza, 15.00437-6

Nesse projeto, construiremos um vaso inteligente capaz de automaticamente regar a planta quando a umidade da terra estiver baixa

Utilizaremos os seguintes componentes:

* Display OLED 128X32, que será utilizado para exibir dados atuais da planta, tais como temperatura e umidade;
* Bomba de água com ponte-H, usada para regar a planta quando acionada;
* Módulo Bluetooth, utilizado para enviar avisos e informações ao celular do dono;
* Sensor de umidade do solo SKU:SEN0114;
* Sensor de luz (LDR);
* Sensor de umidade/temperatura do ar;
* Acelerômetro, utilizado para detectar presença de ventos fortes;
* Microcontrolador ATSAM4SD32C com a placa SAM4S-EK2;

Obteremos as informações dos sensores e exibiremos no display OLED. 

Caso a temperatura saia do intervalo de 20 a 26ºC, um aviso de temperatura inadequada será enviado para o módulo Bluetooth.

Caso a luminosidade seja menor que 30% por mais de 8 horas seguidas, ativaremos um LED de alto brilho para iluminar a planta, e um alerta de baixa luminosidade contínua será enviado, dizendo que o LED foi aceso. Quando a luminosidade passar de 40%, ou ficar ligado por mais de 8 horas seguidas, o LED será desligado.

Quando a umidade do solo atingir certo limiar acionaremos a bomba d’água via PWM para regar a planta. Além disso, haverá um comando para acionamento manual da bomba via bluetooth.

A bomba é desacionada quando a umidade passar de determinado valor máximo.

Caso o acelerômetro apresente uma variação na aceleração maior que 0.5g em qualquer eixo, ou uma inclinação anormal do giroscópio, um alerta de alta movimentação será enviado para o módulo bluetooth. Isso pode indicar tanto a presença de ventos fortes, que podem danificar a planta, como também possível furto ou acidente.

Ao enviar um comando via bluetooth, todas as medições atuais serão enviadas para o usuário.

Caso haja presença de vento forte, indicado pelo acelerômetro e giroscópio, em conjunto com baixa umidade do ar, a umidade máxima liberada pela bomba será aumentada, pois ventos fortes e secos intensificam a evapotranspiração, aumentando a demanda da planta por água.

As medições serão atualizadas de 10 em 10 minutos com a bomba não acionada. Quando a bomba for acionada, a medição da umidade do solo será realizada a cada 1 segundo, para maior precisão no desacionamento da bomba.
