#include <WiFiS3.h>
#include <Servo.h>
#include <WiFiSSLClient.h>
#include <ArduinoHttpClient.h>


const char* ssid     = "ssid"; //ssid for Wi-Fi network
const char* password = "password";// password of Wi-Fi network

#define MIN_DUTY 120
#define MOT_RA 2   
#define MOT_RB 3   
#define EN_A   9  
#define MOT_LA 4   
#define MOT_LB 5   
#define EN_B   10  
#define detector 8

WiFiServer server(80);
Servo myServo;
int servoPulse = 1500;

float ipLatitude = 0.0;
float ipLongitude = 0.0;

#define port 8000



#define supabaseURL "supabase url" // Supabase url
#define port 443
#define tablename  "data"

String tableEndpoint = String("/rest/v1/") + tablename;

#define APIkey "api_key"// api key for Supabase 

WiFiSSLClient sslClient;
HttpClient httpClient(sslClient, supabaseURL, port);

void sendDataToSupabase(float lat, float lon) { 
  String payload = "{";
  payload += "\"latitude\":" + String(lat, 6) + ",";
  payload += "\"longtitude\":" + String(lon, 6);
  payload += "}";

  Serial.println("Sending data to Supabase...");
  Serial.print("Payload: ");
  Serial.println(payload);

  httpClient.beginRequest();
  httpClient.post(tableEndpoint);

  httpClient.sendHeader("Content-Type", "application/json");
  httpClient.sendHeader("apikey", APIkey);
  httpClient.sendHeader("Authorization", String("Bearer ") + APIkey);
  httpClient.sendHeader("Content-Length", payload.length());

  httpClient.beginBody();
  httpClient.print(payload);
  httpClient.endRequest();

  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}




class SimpleDCMotor {
  public:
    SimpleDCMotor(int dirPin1, int dirPin2, int enPin, int minDuty, bool invert = false)
      : _dirPin1(dirPin1), _dirPin2(dirPin2), _enPin(enPin), _minDuty(minDuty), _invert(invert) {}
    void begin() {
      pinMode(_dirPin1, OUTPUT);
      pinMode(_dirPin2, OUTPUT);
      pinMode(_enPin, OUTPUT);
      stop();
    }
    void setSpeed(int speed) {
      if (_invert){
         speed = -speed;
      }
      if (speed > 0) {
        int pwm = (speed < _minDuty) ? _minDuty : speed;
        digitalWrite(_dirPin1, HIGH);
        digitalWrite(_dirPin2, LOW);
        analogWrite(_enPin, pwm);
      } 
      else if (speed < 0) {
        int pwm = ((-speed) < _minDuty) ? _minDuty : -speed;
        digitalWrite(_dirPin1, LOW);
        digitalWrite(_dirPin2, HIGH);
        analogWrite(_enPin, pwm);
      } 
      else {
        stop();
      }
    }
    void stop() {
      digitalWrite(_dirPin1, LOW);
      digitalWrite(_dirPin2, LOW);
      analogWrite(_enPin, 0);
    }
  private:
    int _dirPin1, _dirPin2, _enPin, _minDuty;
    bool _invert;
};

SimpleDCMotor motorL(MOT_LA, MOT_LB, EN_B, MIN_DUTY, false);
SimpleDCMotor motorR(MOT_RA, MOT_RB, EN_A, MIN_DUTY, true);

int speedLevel = 1;

int getCurrentSpeed() {
  return map(speedLevel, 1, 6, MIN_DUTY, 255);
}
void moveForward() {
  int spd = getCurrentSpeed();
  motorL.setSpeed(spd);
  motorR.setSpeed(spd);
  Serial.print("Forward - Speed: ");
  Serial.println(spd);
}
void moveBackward() {
  int spd = getCurrentSpeed();
  motorL.setSpeed(-spd);
  motorR.setSpeed(-spd);
  Serial.print("Backward - Speed: ");
  Serial.println(spd);
}
void turnLeft() {
  int spd = getCurrentSpeed();
  motorL.setSpeed(-spd);
  motorR.setSpeed(spd);
  Serial.print("Turn Left - Speed: ");
  Serial.println(spd);
}
void turnRight() {
  int spd = getCurrentSpeed();
  motorL.setSpeed(spd);
  motorR.setSpeed(-spd);
  Serial.print("Turn Right - Speed: ");
  Serial.println(spd);
}
void moveForwardLeft() {
  int spd = getCurrentSpeed();
  int reduced = (int)(spd * 0.7);
  motorL.setSpeed(reduced);
  motorR.setSpeed(spd);
  Serial.print("Forward Left - Speed: ");
  Serial.println(spd);
}
void moveForwardRight() {
  int spd = getCurrentSpeed();
  int reduced = (int)(spd * 0.7);
  motorL.setSpeed(spd);
  motorR.setSpeed(reduced);
  Serial.print("Forward Right - Speed: ");
  Serial.println(spd);
}
void moveBackwardLeft() {
  int spd = getCurrentSpeed();
  int reduced = (int)(spd * 0.7);
  motorL.setSpeed(-reduced);
  motorR.setSpeed(-spd);
  Serial.print("Backward Left - Speed: ");
  Serial.println(spd);
}
void moveBackwardRight() {
  int spd = getCurrentSpeed();
  int reduced = (int)(spd * 0.7);
  motorL.setSpeed(-spd);
  motorR.setSpeed(-reduced);
  Serial.print("Backward Right - Speed: ");
  Serial.println(spd);
}
void stopMotors() {
  motorL.stop();
  motorR.stop();
  Serial.println("Stop");
}




bool getIPLocation() {
  WiFiClient client;
  HttpClient ipClient(client, "ip-api.com", 80);

  ipClient.get("/json");

  int statusCode = ipClient.responseStatusCode();
  String response = ipClient.responseBody();

  if (statusCode == 200) {
    int latIndex = response.indexOf("\"lat\":");
    int lonIndex = response.indexOf("\"lon\":");

    if (latIndex != -1 && lonIndex != -1) {
      ipLatitude = response.substring(latIndex + 6, response.indexOf(",", latIndex)).toFloat();
      ipLongitude = response.substring(lonIndex + 6, response.indexOf(",", lonIndex)).toFloat();

      return true;
    }
  }

  Serial.println("Failed to get location via IP.");
  return false;
}




void setup() {
  Serial.begin(115200);
  motorL.begin();
  motorR.begin();
  myServo.attach(6);
  myServo.writeMicroseconds(servoPulse);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  // int detectorState = digitalRead(detector);
  if (digitalRead(detector) == HIGH) {
    getIPLocation();
  
    Serial.println("Found");
    sendDataToSupabase(ipLatitude,ipLongitude);
  }

  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  String request = "";
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      request += c;
      if (c == '\n' && request.endsWith("\r\n\r\n")) {
      break;
      }
    }
  }
  if (request.indexOf("GET /forward") != -1 && request.indexOf("GET /fwd_left") == -1 && request.indexOf("GET /fwd_right") == -1){
    moveForward();
  }
  else if (request.indexOf("GET /backward") != -1 && request.indexOf("GET /back_left") == -1 && request.indexOf("GET /back_right") == -1){
    moveBackward();
  }
  else if (request.indexOf("GET /left") != -1){
    turnLeft();
  }
  else if (request.indexOf("GET /right") != -1){
    turnRight();
  }
  else if (request.indexOf("GET /fwd_left") != -1){
    moveForwardLeft();
    }
  else if (request.indexOf("GET /fwd_right") != -1){
    moveForwardRight();
    }
  else if (request.indexOf("GET /back_left") != -1){
    moveBackwardLeft();
    }
  else if (request.indexOf("GET /back_right") != -1){
    moveBackwardRight();
    }
  else if (request.indexOf("GET /stop") != -1){
    stopMotors();
    }
  if (request.indexOf("GET /speed1") != -1) {
    speedLevel = 1;
    Serial.println("Speed Level set to 1");
  } 
  else if (request.indexOf("GET /speed2") != -1) {
    speedLevel = 2;
    Serial.println("Speed Level set to 2");
  } 
  else if (request.indexOf("GET /speed3") != -1) {
    speedLevel = 3;
    Serial.println("Speed Level set to 3");
  }
   else if (request.indexOf("GET /speed4") != -1) {
    speedLevel = 4;
    Serial.println("Speed Level set to 4");
  } 
  else if (request.indexOf("GET /speed5") != -1) {
    speedLevel = 5;
    Serial.println("Speed Level set to 5");
  } 
  else if (request.indexOf("GET /speed6") != -1) {
    speedLevel = 6;
    Serial.println("Speed Level set to 6");
  }
  if (request.indexOf("GET /q") != -1) {
    if (servoPulse < 2000) servoPulse += 50;
    myServo.writeMicroseconds(servoPulse);
    Serial.print("Servo up: ");
    Serial.println(servoPulse);
  } 
  else if (request.indexOf("GET /e") != -1) {
    if (servoPulse > 1000) servoPulse -= 50;
    myServo.writeMicroseconds(servoPulse);
    Serial.print("Servo down: ");
    Serial.println(servoPulse);
  }
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html><head><meta charset='utf-8'><title>Tank Control</title>");
  client.println("<style>");
  client.println("body { margin: 0; padding: 0; text-align: center; font-family: sans-serif; }");
  client.println(".container { display: flex; flex-direction: column; align-items: center; margin-top: 20px; }");
  client.println(".row { display: flex; justify-content: center; margin: 10px; }");
  client.println(".button { padding: 20px 40px; margin: 5px; font-size: 20px; background-color: #4CAF50; color: white; border: none; border-radius: 8px; cursor: pointer; text-decoration: none; }");
  client.println(".button:hover { background-color: #45a049; }");
  client.println(".small { font-size: 16px; padding: 10px 20px; }");
  client.println("</style>");
  client.println("<script>");
  client.println("let keys = {};");
  client.println("function updateMotion() {");
  client.println("  let w = keys['w'] || false, a = keys['a'] || false, s = keys['s'] || false, d = keys['d'] || false;");
  client.println("  if (!w && !a && !s && !d) {");
  client.println("    fetch('/stop', {cache:'no-store'});");
  client.println("    return;");
  client.println("  }");
  client.println("  if (w && !s) {");
  client.println("    if (a && !d) fetch('/fwd_left', {cache:'no-store'});");
  client.println("    else if (d && !a) fetch('/fwd_right', {cache:'no-store'});");
  client.println("    else fetch('/forward', {cache:'no-store'});");
  client.println("  } else if (s && !w) {");
  client.println("    if (a && !d) fetch('/back_left', {cache:'no-store'});");
  client.println("    else if (d && !a) fetch('/back_right', {cache:'no-store'});");
  client.println("    else fetch('/backward', {cache:'no-store'});");
  client.println("  } else if (a && !d) {");
  client.println("    fetch('/left', {cache:'no-store'});");
  client.println("  } else if (d && !a) {");
  client.println("    fetch('/right', {cache:'no-store'});");
  client.println("  } else {");
  client.println("    fetch('/stop', {cache:'no-store'});");
  client.println("  }");
  client.println("}");
  client.println("document.addEventListener('keydown', function(e) {");
  client.println("  let key = e.key.toLowerCase();");
  client.println("  if(['w','a','s','d'].includes(key)){ keys[key] = true; updateMotion(); e.preventDefault(); }");
  client.println("  if(['1','2','3','4','5','6'].includes(e.key)){ fetch('/speed' + e.key, {cache:'no-store'}); }");
  client.println("  if(e.key.toLowerCase() === 'q'){ fetch('/q', {cache:'no-store'}); }");
  client.println("  if(e.key.toLowerCase() === 'e'){ fetch('/e', {cache:'no-store'}); }");
  client.println("});");
  client.println("document.addEventListener('keyup', function(e) {");
  client.println("  let key = e.key.toLowerCase();");
  client.println("  if(['w','a','s','d'].includes(key)){ keys[key] = false; updateMotion(); e.preventDefault(); }");
  client.println("});");
  client.println("</script>");
  client.println("</head><body>");
  client.println("<div class='container'>");
  client.println("<h1>Tank Control</h1>");
  client.println("<p>Use WASD for motion (diagonals supported), 1-6 to set speed, Q to move servo up, E to move servo down.</p>");
  client.println("<div class='row'><a href='/forward' class='button'>Forward</a></div>");
  client.println("<div class='row'>");
  client.println("<a href='/left' class='button'>Left</a>");
  client.println("<a href='/stop' class='button'>Stop</a>");
  client.println("<a href='/right' class='button'>Right</a>");
  client.println("</div>");
  client.println("<div class='row'><a href='/backward' class='button'>Backward</a></div>");
  client.println("<hr>");
  client.println("<div class='row'>");
  client.println("<a href='/speed1' class='button small'>Speed 1</a>");
  client.println("<a href='/speed2' class='button small'>Speed 2</a>");
  client.println("<a href='/speed3' class='button small'>Speed 3</a>");
  client.println("<a href='/speed4' class='button small'>Speed 4</a>");
  client.println("<a href='/speed5' class='button small'>Speed 5</a>");
  client.println("<a href='/speed6' class='button small'>Speed 6</a>");
  client.println("</div>");
  client.println("<div class='row'>");
  client.println("<a href='/q' class='button small'>Servo Up (Q)</a>");
  client.println("<a href='/e' class='button small'>Servo Down (E)</a>");
  client.println("</div>");
  client.println("</div>");
  client.println("</body></html>");
  delay(1);
  client.stop();
}
