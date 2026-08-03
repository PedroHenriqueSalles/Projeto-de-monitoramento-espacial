# Projeto-de-monitoramento-espacial

Olá, gostaria de compartilhar um projeto que estive trabalhando recentemente, fiz um sistema monitoramento de sinal utilizando ESP 32, também fiz um dashboard para facilitar a visualização dos dados.
para deixar o projeto mais lúdico estou usando uma "temática" espacial, simulando um cenário de monitoramento de uma nave espacial tripulada.

Nesse projeto eu usei o WOKWI para fazer o circuito e sua programação e o ADAFRUIT IO para conseguir fazer um DASHBOARD. o projeto funciona apenas no wokwi também caso vocês queiram testar apenas o circuito.

O sistema monitora 3 tanques, 1 de combustível, 1 de água e 1 de suprimentos como alimentos.
também monitora a bateria da nave, pressão e temperatura da cabine da tripulação.

No circuito eu utilizei:
para o circuito eu utilizei os seguintes componentes:
1x ESP 32 com wifi scanning
3x sensores hc-sr04 (para os tanques)
1x modulo photoresistor-sensor (para simular a bateria, já que no wokwi não tenho modulo de bateria em si)
1x board-bmp180 Barometric Pressure Sensor (para pressão e temperatura)
1x buzzer (como indicador de alerta critico)

Montagem:
para facilitar na hora de montar o circuito estarei disponibilizando uma imagem do circuito logo abaixo:

<img width="1185" height="715" alt="image" src="https://github.com/user-attachments/assets/7f9d67ad-0cbb-4cab-ac2e-71013a3a3ebf" />



Código:
Estarei disponibilizando os códigos que utilizei nesse projeto. os nomes dos arquivos estão separados de acordo com a aba que deverão ser colados
certifiquem-se de que todas as abas (wifi-scan.ino, diagram.json e libraries.txt) estejam preenchidas

Agora para fazer o dashboard:
Acesse o site do ADAFRUIT IO: https://io.adafruit.com faça o login.
vá até a aba dashboard e clique em "new dashboard"
<img width="1847" height="587" alt="image" src="https://github.com/user-attachments/assets/e1a6e42a-aa10-4ac9-8c57-7b98eea4d153" />

vá até a aba feeds e crie um novo feed
<img width="1847" height="885" alt="image" src="https://github.com/user-attachments/assets/88606199-e821-471d-9b3a-fee1ba779b43" />

De um nome e descrição ao feed.
volte ao dashboard

clique em new block:
<img width="1847" height="535" alt="image" src="https://github.com/user-attachments/assets/ab428455-7a45-422c-851d-695f231b198c" />

escolha o tipo/design de bloco de você preferir
<img width="502" height="707" alt="image" src="https://github.com/user-attachments/assets/6d1dba38-9d57-439c-925d-97ece0213c64" />

atribua o feed ao bloco e clique em next step
<img width="805" height="797" alt="image" src="https://github.com/user-attachments/assets/36424335-bece-4de3-b29b-55711735a8ab" />

após criar todos os seus feeds
no código por volta das linhas 36 à 42 está a parte do código que você atribui os sensores ao feed 
"Adafruit_MQTT_Publish pAgua = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/agua");
Adafruit_MQTT_Publish pComb = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/combustivel");
Adafruit_MQTT_Publish pMant = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/mantimentos");
Adafruit_MQTT_Publish pBat  = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/bateria");
Adafruit_MQTT_Publish pTemp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperatura");
Adafruit_MQTT_Publish pPres = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressao");
Adafruit_MQTT_Publish pStat = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/status");"

Nas linhas (16-21) está a parte do código que conecta o wokwi ao adafruit IO, nessa parte vocês devem inserir as SUAS credenciais do site do adafruit para conectar o circuito ao dashboard que será criado!
onde você deverá colocar suas credenciais do adafruit

"#define WLAN_SSID       ("Wokwi-GUEST")
#define WLAN_PASS       ""
#define AIO_SERVER      ("io.adafruit.com")
#define AIO_SERVERPORT  (inserir)
#define AIO_USERNAME    ("inserir")
#define AIO_KEY         ("inserir")"

suas credenciais estarão no seguinte icone:
<img width="1847" height="552" alt="image" src="https://github.com/user-attachments/assets/ebd69d34-a6ab-4165-9a4d-f681bcb42063" />

<img width="676" height="397" alt="image" src="https://github.com/user-attachments/assets/a8c0a610-ec79-4eb2-9564-74853c27a798" />






