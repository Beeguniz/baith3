#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_arduino_version.h>
#include "project_config.h"
#include "controller.h"
#include "web_page.h"

namespace {
WebServer server(80);
Channel channels[] = {{"LED1"}, {"LED2"}, {"MOTOR1"}};
bool i2cReady = false, sampleValid = false, haveSample = false;
uint32_t lastPollMs = 0, lastSampleMs = 0;
float temperature = 0, humidity = 0;
constexpr uint32_t kStaleMs = 6000;

hw_timer_t* periodicTimer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint32_t timerTicks = 0;
uint32_t reportedTicks = 0;
bool ledReady[2] = {false, false};
uint32_t ledApplied[2] = {UINT32_MAX, UINT32_MAX};
bool motorReady = false;
uint32_t motorApplied = UINT32_MAX;

void ARDUINO_ISR_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  timerTicks = timerTicks + 1;
  portEXIT_CRITICAL_ISR(&timerMux);
}
uint32_t getTimerTicks() {
  portENTER_CRITICAL(&timerMux);
  const uint32_t ticks = timerTicks;
  portEXIT_CRITICAL(&timerMux);
  return ticks;
}
void beginPeriodicTimer() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  periodicTimer = timerBegin(1000000);
  if (!periodicTimer) return;
  timerAttachInterrupt(periodicTimer, &onTimer);
  timerAlarm(periodicTimer, 1000000, true, 0);
#else
  periodicTimer = timerBegin(0, 80, true);
  if (!periodicTimer) return;
  timerAttachInterrupt(periodicTimer, &onTimer, true);
  timerAlarmWrite(periodicTimer, 1000000, true);
  timerAlarmEnable(periodicTimer);
#endif
}
void applyOutputs() {
  // Explicitly scoped to two LEDs and MOTOR1; never access MOTOR2.
  digitalWrite(config::kMotor1In2, LOW);
  const uint32_t motorDuty = channels[2].state == State::Running
      ? (channels[2].duty * 255U + 50U) / 100U : 0;
  if (motorReady && motorDuty != motorApplied) {
    if (!ledcWrite(config::kMotor1In1, motorDuty)) {
      ledcDetach(config::kMotor1In1);
      digitalWrite(config::kMotor1In1, LOW);
      pinMode(config::kMotor1In1, OUTPUT);
      digitalWrite(config::kMotor1In1, LOW);
      motorReady = false;
      faultChannel(channels[2], "PWM_ERROR");
    } else motorApplied = motorDuty;
  }
  for (unsigned i=0;i<2;++i) {
    if (!ledReady[i]) continue;
    const uint32_t duty = channels[i].state == State::Running
        ? (channels[i].duty * 255U + 50U) / 100U : 0;
    if (duty == ledApplied[i]) continue;
    if (!ledcWrite(config::kLedPins[i], duty)) {
      ledcDetach(config::kLedPins[i]);
      digitalWrite(config::kLedPins[i], LOW);
      pinMode(config::kLedPins[i], OUTPUT);
      digitalWrite(config::kLedPins[i], LOW);
      ledReady[i] = false;
      faultChannel(channels[i], "PWM_ERROR");
    } else ledApplied[i] = duty;
  }
}
void beginLeds() {
  for (unsigned i=0;i<2;++i) {
    const int pin = config::kLedPins[i];
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
    ledReady[i] = ledcAttach(pin, config::kLedPwmHz, config::kLedPwmBits);
    if (!ledReady[i]) {
      digitalWrite(pin, LOW);
      faultChannel(channels[i], "PWM_ERROR");
    }
  }
  applyOutputs();
}
void beginMotor1() {
  digitalWrite(config::kMotor1In1, LOW);
  digitalWrite(config::kMotor1In2, LOW);
  pinMode(config::kMotor1In1, OUTPUT);
  pinMode(config::kMotor1In2, OUTPUT);
  channels[2].duty = 100; // Preserve the previous full-speed START default.
  motorReady = ledcAttach(config::kMotor1In1, config::kMotorPwmHz, 8);
  if (!motorReady) {
    digitalWrite(config::kMotor1In1, LOW);
    faultChannel(channels[2], "PWM_ERROR");
  }
  applyOutputs();
}

bool sensorValid(uint32_t now) {
  return i2cReady && haveSample && sampleValid && uint32_t(now-lastSampleMs)<kStaleMs;
}
const char* stateName(State s) {
  return s==State::Running ? "RUNNING" : s==State::Fault ? "FAULT" : "STOPPED";
}
void updateState() {
  const uint32_t now=millis();
  State oldStates[3];
  for(unsigned i=0;i<3;++i) {
    auto& c=channels[i];
    oldStates[i]=c.state;
    updateChannel(c,now,sensorValid(now));
  }
  applyOutputs(); // Evaluate MOTOR1 and cut drive before any channel logging.
  for(unsigned i=0;i<3;++i) {
    const auto& c=channels[i];
    if(oldStates[i]!=c.state) Serial.printf("[%lu] %s %s -> %s reason=%s\n",
      static_cast<unsigned long>(now),c.name,stateName(oldStates[i]),stateName(c.state),c.reason);
  }
}
void readUno() {
  uint8_t bytes[config::kPacketSize]={};
  const size_t received=Wire.requestFrom(config::kUnoAddress,config::kPacketSize,true);
  size_t count=0;
  while(Wire.available() && count<sizeof(bytes)) bytes[count++]=Wire.read();
  while(Wire.available()) Wire.read();
  if(received!=sizeof(bytes)||count!=sizeof(bytes)) {
    sampleValid=false;
    updateState();
    Serial.println("I2C_ERROR: incomplete packet");
    return;
  }
  temperature=((uint16_t(bytes[0])<<8)|bytes[1])/10.0f;
  humidity=((uint16_t(bytes[2])<<8)|bytes[3])/10.0f;
  // Provisional plausibility limits, not validation of UNO's unknown protocol.
  sampleValid=temperature<=80.0f && humidity<=100.0f;
  if(sampleValid) {haveSample=true;lastSampleMs=millis();}
  updateState();
  Serial.printf("RAW=%02X %02X %02X %02X ASSUMED T=%.1f RH=%.1f valid=%d\n",
    unsigned(bytes[0]),unsigned(bytes[1]),unsigned(bytes[2]),unsigned(bytes[3]),
    temperature,humidity,sampleValid);
}
void sendStatus() {
  const uint32_t now=millis();
  String json="{\"hardwareEnabled\":true,\"sensorValid\":";
  json+=sensorValid(now)?"true":"false";
  json+=",\"temperature\":"+String(temperature,1)+",\"humidity\":"+String(humidity,1);
  json+=",\"timerReady\":"+String(periodicTimer ? "true" : "false");
  json+=",\"timerTicks\":"+String(getTimerTicks());
  json+=",\"channels\":[";
  for(unsigned i=0;i<3;++i) {
    const auto& c=channels[i];
    const uint32_t elapsed=now-c.started;
    const uint32_t remaining=c.state==State::Running && elapsed<kRunLimitMs?kRunLimitMs-elapsed:0;
    if(i)json+=',';
    json+="{\"name\":\""+String(c.name)+"\",\"state\":\""+stateName(c.state);
    json+="\",\"reason\":\""+String(c.reason)+"\",\"revision\":"+String(c.revision);
    json+=",\"hardwareEnabled\":"+String((i==2 ? motorReady : ledReady[i]) ? "true" : "false");
    json+=",\"duty\":"+String(c.duty)+",\"remainingMs\":"+String(remaining)+"}";
  }
  json+="]}";
  server.sendHeader("Cache-Control","no-store");
  server.send(200,"application/json",json);
}
bool parseRevision(const String& s,uint32_t& result) {
  if(s.length()==0 || s.length()>10)return false;
  uint64_t n=0;
  for(unsigned i=0;i<s.length();++i){if(s[i]<'0'||s[i]>'9')return false;n=n*10+(s[i]-'0');}
  if(n>UINT32_MAX)return false;
  result=static_cast<uint32_t>(n);return true;
}
void invalidCommand() {
  for(auto& c:channels)faultChannel(c,"INVALID_COMMAND");
  applyOutputs();
  Serial.println("INVALID_COMMAND: MOTOR1 off; all channels FAULT -> STOPPED");
  server.send(400,"application/json","{\"error\":\"Lenh sai; da dung 3 kenh\"}");
}
void control() {
  // Timeout and sensor errors take priority over requests, including queued STARTs.
  updateState();
  const String target=server.arg("channel"), action=server.arg("action");
  if(target=="all" && action=="STOP") {
    for(auto& c:channels)stopChannel(c,"STOP");
    applyOutputs();sendStatus();return;
  }
  int index=-1;
  if(target=="0")index=0;else if(target=="1")index=1;else if(target=="2")index=2;
  if(index<0){invalidCommand();return;}
  Channel& c=channels[index];
  if(action=="STOP") {stopChannel(c,"STOP");applyOutputs();sendStatus();return;}
  if(!(index==2 ? motorReady : ledReady[index])) {
    server.send(503,"application/json","{\"error\":\"PWM initialization failed\"}");return;
  }
  if(action!="START" && action!="PWM"){invalidCommand();return;}
  uint32_t revision;
  if(!parseRevision(server.arg("revision"),revision)){invalidCommand();return;}
  if(action=="PWM") {
    const String duty=server.arg("duty");
    const bool validDuty = index==2
        ? (duty=="25" || duty=="50" || duty=="75" || duty=="100")
        : (duty=="25" || duty=="50" || duty=="90");
    if(!validDuty){invalidCommand();return;}
    if(revision!=c.revision || c.state==State::Fault) {
      server.send(409,"application/json","{\"error\":\"Trang thai da doi; thu lai\"}");return;
    }
    c.duty=duty.toInt();++c.revision;
  } else if(!startChannel(c,millis(),sensorValid(millis()),revision)) {
    server.send(409,"application/json","{\"error\":\"Du lieu UNO loi hoac trang thai da doi\"}");return;
  }
  applyOutputs();
  Serial.printf("%s %s hardware=1\n",c.name,action.c_str());
  sendStatus();
}
}
void setup() {
  beginMotor1(); // Off before Serial, I2C, timer, or Wi-Fi initialization.
  beginLeds();
  Serial.begin(config::kSerialBaud);
  Serial.println("TH03: LED1/LED2 active HIGH PWM; MOTOR1 hardware enabled.");
  beginPeriodicTimer();
  if (!periodicTimer) Serial.println("TIMER_INIT_ERROR");
  i2cReady=Wire.begin(config::kSdaPin,config::kSclPin,config::kI2cFrequencyHz);
  if(i2cReady)Wire.setTimeOut(config::kI2cTimeoutMs);
  else Serial.println("I2C_INIT_ERROR");
  lastPollMs=millis();
  WiFi.mode(WIFI_AP);
  if(!WiFi.softAP("TH03-Greenhouse","th03demo123"))Serial.println("AP_INIT_ERROR");
  Serial.print("Web: http://");Serial.println(WiFi.softAPIP());
  server.on("/",HTTP_GET,[]{server.send_P(200,"text/html; charset=utf-8",kWebPage);});
  server.on("/api/status",HTTP_GET,[]{updateState();sendStatus();});
  server.on("/api/control",HTTP_POST,control);
  server.onNotFound([]{server.send(404,"application/json","{\"error\":\"Not found\"}");});
  server.begin();
}
void loop() {
  updateState();
  const uint32_t now=millis();
  if(i2cReady && uint32_t(now-lastPollMs)>=config::kPollIntervalMs) {
    lastPollMs=now;readUno();updateState();
  }
  const uint32_t ticks = getTimerTicks();
  if (ticks != reportedTicks) {
    reportedTicks = ticks;
    Serial.printf("TIMER ticks=%lu handled_at_ms=%lu (not ISR timestamp)\n",
      static_cast<unsigned long>(ticks), static_cast<unsigned long>(millis()));
  }
  server.handleClient();
  updateState();
  yield();
}
