#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Servo.h>
#include <FirebaseESP8266.h>


ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

Servo servo;

FirebaseData firebaseData;

#define FIREBASE_KEY "ZFiASLVIt2fEMHnh3oJmyuhDxhf1Z7msGL2vYPQQ"
#define FIREBASE_HOST "https://asm2-76e0d.firebaseio.com/"

char* ssid = "Pika True";
char* password = "sapa123lc";


char webpage[] PROGMEM = R"=====(
<html>
<head>
</head>
<script>
  var Socket;
  function init(){
    Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
    Socket.onmessage = function(event){
      console.log(event);
      console.log(event.data);
       document.querySelector("#waterSensor").innerHTML = ""+event.data;
    }
  }
document.addEventListener("DOMContentLoaded",() => {
  document.querySelectorAll('.btn').forEach(button => {
    button.onclick = function(){
      const url = button.dataset.url;
      const request = new XMLHttpRequest();
      request.open('GET', url);
      request.onload = () => {
        button.parentNode.querySelector('span').innerHTML = request.responseText;
      }
      request.send();
      }
     return false;
});
});
</script>
<body onload = "javascript:init()">
<div>
<button class="btn" data-url="/change-led">
Chage state led
</button>
<p>LED state:<span>ON</span></p>
</div>
</hr>
</br>
</br>
<div>
<button class = "btn" data-url="/change-vol">
Chage state volumn
</button>
<p>Volumn state:<span>ON</span> </p>
</div>
</hr>
</br>
</br>
<div>
<button class = "btn" data-url="/open-tap">
Open the tap
</button>
<span></span>
</div>
</hr>
</br>
</br>
<div>
<button class = "btn" data-url="/change-servo">
Change system state
</button>
<p>System state:<span>ON</span> </p>
</div>
</hr>
</br>
</br>
<p>Amount of water remaining: 
<p id="waterSensor">1</p>
</body>
</html>
)=====";


bool status_led = true;
bool status_vol = true;
bool status_servo= true;

void setup() {
  pinMode(5, INPUT);
  servo.attach(4);
  Serial.begin(115200);
  pinMode(14,OUTPUT);
  pinMode(12, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi: ");
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
    }
   Serial.println();
   Serial.print("Connected with IP: ");
   Serial.println(WiFi.localIP());
   Serial.println();

 server.on("/",[](){
    server.send_P(200, "text/html", webpage);  
  });
  server.on("/change-led", getStateLed);
  server.on("/change-vol", getStateVol);
  server.on("/open-tap", openTap);
  server.on("/change-servo", changeServo);
  server.begin();
  webSocket.begin();

  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);
  Firebase.reconnectWiFi(true);

}

void loop() {
    unsigned long time;
    time = millis();  

    String ts = String(time);
     webSocket.loop();
     server.handleClient();
    int waterSensor = analogRead(A0);
    int sensorValue = digitalRead(5);

    
    if(status_servo == false){
      digitalWrite(12,0);
      digitalWrite(14,0);
      
    }else{
    if(waterSensor > 600){
      digitalWrite(12,0);
      if(sensorValue == 0){
        delay(100);
        servo.write(180);
        delay(1000);
//        Firebase.setInt(firebaseData, "/"+ts+"-hand", sensorValue);
      }
        servo.write(0);
        delay(1000);
      }else{
        if(status_led){
            digitalWrite(12,1);
          }
       if(sensorValue == 0 ){
          if(status_vol){
          digitalWrite(14,1);
          delay(50);
        }
        digitalWrite(14,0);
        delay(50);
       }
     }
    }
//    Firebase.setInt(firebaseData, "/"+ts, waterSensor);
      
    String s="";
    s += waterSensor;
    char ct[5];
    s.toCharArray(ct,5);
    webSocket.broadcastTXT(ct, sizeof(ct));
}


void getStateLed(){
  if(!status_servo){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "System is OFF");
    }else{
  changeLED();
  String led_state = digitalRead(12) ? "ON" : "OFF";
  if(led_state == "OFF"){
    status_led = false;
    }else{
      status_led = true;}
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", led_state);
    }
}

void changeLED(){
  digitalWrite(12, !digitalRead(12));
}


void getStateVol(){
  if(!status_servo){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "System is OFF");
    }else{
  if(status_vol){
    status_vol = false;
    }else{
      status_vol = true;}
  String vol_state = status_vol ? "ON" : "OFF";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", vol_state);
    }
  }
void openTap(){
  if(!status_servo){
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "System is OFF");
    }else{
  delay(100);
  servo.write(180);
  delay(1000);
  servo.write(0);
  delay(1000);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "");
    }
  }

void changeServo(){
  if(status_servo){
    status_servo = false;
    }else{
      status_servo = true;}
  String servo_state = status_servo ? "ON" : "OFF";
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", servo_state);
  }
