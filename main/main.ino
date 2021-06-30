
//#define DS(...) Serial.print(__VA_ARGS__);
//#define DL(...) Serial.println(__VA_ARGS__);
#define DL(...)
#define DS(...)

#define RELAY_COUNT 8
#define RELAY_OFFSET 2

#define LEN_SERIAL_BUFFER 200

#define CMD_UNKNOWN 0
#define CMD_INIT 1
#define CMD_RELAY_BASE 2
//... until CMD_RELAY 9

const int status_led = LED_BUILTIN;


void setup(){
  Serial.begin(115200); 

  for(int i = 0; i < RELAY_COUNT; i++){
    pinMode(RELAY_OFFSET + i, OUTPUT);

    DS("set relay on pin")
    DL(RELAY_OFFSET + i)
  }
}

void loop(){
  int len;
  char command[LEN_SERIAL_BUFFER];
  char commandValue[LEN_SERIAL_BUFFER];
  
  if(!Serial.available())
    return;

  len = Serial.readBytesUntil('\n', command, LEN_SERIAL_BUFFER);
  if(len <= 0){
    DL("Serial read did not produce a result");
    return;
  }
  command[len] = '\0';
  
  int parsedCmd = parseCommand(command);
  if(parsedCmd <= 0){
    DL("Command unkown");
    return;
  }
  DS("Found command ");
  DL(parsedCmd);
  
  switch(command[0]){
    case '>':
      if (getValueFromSerial(commandValue, LEN_SERIAL_BUFFER) <= 0) {
        DL("Value could not be read");
        return;
      }
      break;
    case '/':
      break;
    default:
      DL("Command could not be understood");
  }

  executeCommand(parsedCmd, commandValue);
}

int parseCommand(const char* command){
  if(strncmp("/init", command, LEN_SERIAL_BUFFER) == 0){
    return CMD_INIT;
  }else if(strncmp(">relay", command, 5) == 0 && command[6] >= '0' && command[6] <= '7'){
    DS("relay command number ");
    DL(command[6]);
    return command[6] - '0' + CMD_RELAY_BASE;
  }else{
    return CMD_UNKNOWN;
  }
}

int getValueFromSerial(char *valueBuffer, unsigned int len){
  while(!Serial.available()){
    continue;
  }
    
  int rc = Serial.readBytesUntil('\n', valueBuffer, len);
  if(rc > 0){
    valueBuffer[rc] = '\0';
  }
  return rc;
}

void executeCommand(const int parsedCmd, const char* commandValue){
  if(parsedCmd >= 2 && parsedCmd <= 9){
    setRelay(parsedCmd - CMD_RELAY_BASE, commandValue);
    return;
  }
  
  switch(parsedCmd){
    case 1:
      initAll();
      break;
    default:
      DL("Command unkown");
  }
}

void initAll(){
  DL("Initializing...");
  int mode;
  for(int i = 0; i < RELAY_COUNT; i++){
    mode = digitalRead(RELAY_OFFSET + i);
    Serial.print(">relay");
    Serial.println(i);
    if(mode == HIGH){
      Serial.println("ON");
    }else{
      Serial.println("OFF");
    }
    Serial.print("+relay");
    Serial.println(i);
  }
}

void setRelay(int relay, const char *commandValue){
  
  int mode;
  if(strncmp("ON", commandValue, 2) == 0){
    mode = HIGH;
  }else{
    mode = LOW;
  }
  
  DS("Setting relay ");
  DS(relay);
  DS(" to ");
  DL(mode);
  digitalWrite(RELAY_OFFSET + relay, mode);
}
