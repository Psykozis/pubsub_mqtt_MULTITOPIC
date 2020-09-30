/*
* pubsub_mqtt_MULTITOPIC.INO
* Marlon Soares Sigales
* 
* Código para NodeMCU se subscrever e publicar em tópicos em um Broker controlando suas saídas digitais
* Code for NodeMCU to publish and subscribe topics in an Broker, controling your digital IOs.  https://www.instructables.com/id/IoT-With-NodeMCU-and-MQTT/
* 30/09/2020
* 
* Utilizado/Uses lib PubSubClient=> pubsubclient.knolleary.net/
* 
*
* Este projeto pode ser utilizado, modificado sem fins comerciais desde que referênciado.
* This project can be used, modified without commercial purposes as long as referenced.
*/

//---------------includes--------------------------------
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>
#include <user_interface.h> //lib to use timer 
  
//----------------WiFi------------------------------------ 
const char* SSID =  "AAAAAAAAAAAAA";          //WiFi name SSID            MODIFIE HERE!!!!! 
const char* PASSWORD = "*******************";    //WIFi password          MODIFIE HERE!!!!!
WiFiClient wifiClient; 
//----------------------Mqtt-------------------------------
const char* BROKER_MQTT = "xxx.xxx.0.xx";  //broker MQTT IP or URL         MODIFIE HERE!!!!!
int BROKER_PORT = 1883;                    // Broker MQTT port

//------ Instance MQTT client - with object espClient/wifiClient ------
//PubSubClient client (BROKER_MQTT, BROKER_PORT, recebePacote, wifiClient); //full config with subscription
PubSubClient client (BROKER_MQTT, BROKER_PORT, wifiClient); //instace fully config


//-------- defines -------------------------------------------
#define ID_MQTT  "AAA01"             //ID for this node                  MODIFIE HERE!!!!! only with 3 letters an 2 numbers
#define TOPIC_PUBLISH0   "PAAA01_0"  //button topic 
#define TOPIC_PUBLISH3   "PAAA01_A"  //analogic topic
//pin define
#define D8 15//D8
#define D0 16//D0
#define D1 5 //D1
#define D2 4 //D2
#define D3 0 //D3
#define D4 2 //D4
#define D5 14//D5
#define D6 12//D6
#define D7 13//D7


os_timer_t tmr0; //create timer named tmr0
//-------------global var-------------------------------------
bool cont = LOW;    //counter for timer                               MODIFIE HERE!!!!! the AAA is the ID_mqtt
const char* substopic[] ={"SAAA01_0",   //#0    
                          "SAAA01_1",   //#1
                          "SAAA01_2",   //#2
                          "SAAA01_3",   //#3
                          "SAAA01_4",   //#4
                          "SAAA01_5",   //#5
                          "SAAA01_6",   //#6
                          "SAAA01_7"    //#7
                                           };

//----------functions n' interruptions-------------------------- 
void mantemConexoes() { //keep conection broker and wifi
    if (!client.connected()) {       
       conectaMQTT(); 
     }
    conectaWiFi(); //re do wifi conection if this drop
}

void conectaWiFi() { //conect wifi

  if (WiFi.status() == WL_CONNECTED) {
     return;
  }
    

  WiFi.begin(SSID, PASSWORD); // Conect  WI-FI  
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.println("conecting to wifi, still waiting");
  }
  Serial.println("Conected to wifi ");

}

void conectaMQTT() { //conect to broker
    while (!client.connected()) {
        if (client.connect(ID_MQTT)) {
            Serial.println("broker concetion sucesfully!");
            client.publish("debug","conectou");
            for (int i = 0; i < (sizeof(substopic)/sizeof(int)); i++){
                client.subscribe(substopic[i]);
                client.loop();
                }
        } 
        else {
            Serial.println("broker try new conection in 5 seconds");
            delay(5000);
        }
    }
}

void recebePacote(char* topic, byte* payload, unsigned int length){

  String topico;
  String IDnoh = ID_MQTT;
  String msg;
  static bool flag = HIGH;

  
    for(int i = 0; i < length; i++){
       char c = (char)payload[i];
       msg += c;
    }
  
    if(msg == "0"){ flag = LOW;}
    else{ flag = HIGH;  }

    topico=topic;
    Serial.print("recebido: "+topico.substring(1,6)+"; Esperado: "+IDnoh);
    if(topico.substring(1,6)== IDnoh){  
    topico=topic[7];//last char represents port to write
    Serial.println(" tornando o estado de porta D"+topico+" em "+ msg);
        switch (topico.toInt()){
              case 0:
               digitalWrite(D0,flag);
              break;
              case 1:
               digitalWrite(D1,flag);               
              break;
              case 2:
                digitalWrite(D2,flag);                
              break;
              case 3:
                digitalWrite(D3,flag);
              break;
              case 4:
                digitalWrite(D4,flag);
              break;
              case 5:
                digitalWrite(D5,flag);
              break;
              case 6:
                digitalWrite(D6,flag);
              break;
              case 7:
                digitalWrite(D7,flag);
              break;
          }
    }
}

void timer(void*z){//call back for timer     
     cont=HIGH;
}


//----------------main function----------------------------------

void setup() {
  //ESP.restart(); //restart core
  
  pinMode( D0, OUTPUT); //pino saída 0
  pinMode( D1, OUTPUT); //pino saída 1 
  pinMode( D2, OUTPUT); //pino saída 2
  pinMode( D3, OUTPUT); //pino saída 3
  pinMode( D4, OUTPUT); //pino saída 4
  pinMode( D5, OUTPUT); //pino saída 5
  pinMode( D6, OUTPUT); //pino saída 6
  pinMode( D7, OUTPUT); //pino saída 7
  pinMode( D8, INPUT);  //input button in d8       
  pinMode( A0, INPUT);  //input analog in A0   

  Serial.begin(115200);
  
  conectaWiFi();  // do wifi conection

  os_timer_setfn(&tmr0,timer, NULL);  //callback for timer
  os_timer_arm(&tmr0, 15000, true);   //(timer, time for timer ms, repeat timer? (loop = true)

  client.setCallback(recebePacote);
}

void loop() {
static bool ED8 = HIGH;        //actual digital input d8
static bool D8a = HIGH;        //past digital input d8
static int Ain = 0;            //Avarage analog input
static char message[5];        //var to send analog

    mantemConexoes();//keep conections 

    //------------------------------------------------send values--------------------------------------------------------
    if (!client.connected()) {Serial.println("off loop0"); mantemConexoes();} // test to see where disconect

    
  

    ED8 = digitalRead(D8); //actual input digital
    if (ED8 ^ D8a) { //only send button when the actual state is different from last state
        while(!client.publish(TOPIC_PUBLISH0, (ED8? "0" : "1"))){ if (!client.connected()) mantemConexoes();}//send with retry if not connected
        D8a = ED8;//last assumes actual state
        }
    if (!client.connected()) {Serial.println("off before button test "); mantemConexoes();} // test to see where disconect
  

    snprintf (message, 5, "%d", ((system_adc_read()+Ain)>>1)); //transform in char array
    Ain=atoi(message);
    delay(300);
    if (cont!=LOW){//at every 15 seconds send analog
        //snprintf (message, 5, "%d", analogRead(A0)); //transform in char array
        while(!client.publish(TOPIC_PUBLISH3, message)){if (!client.connected()) mantemConexoes();}//send with retry if not connected
        cont=LOW;
        }
      
    if (!client.connected()) {Serial.println("off end");mantemConexoes();}// test to see where disconect

     //-------------------------------------------------------------------------------------------------------------------------------------------           

     client.loop();           
}
