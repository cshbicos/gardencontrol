
//#define DS(...) Serial.print(__VA_ARGS__);
//#define DL(...) Serial.print(__VA_ARGS__); Serial.print("\n");
#define DL(...)
#define DS(...)

#define RELAY_COUNT 8
#define RELAY_OFFSET 2

#define LEN_SERIAL_BUFFER 200

#define CMD_UNKNOWN 0
#define CMD_INIT 1
#define CMD_RELAY_BASE 2
#define CMD_RELAY_MAX 9

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

  //get additional information if required (depending on command)
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

/**
 * Parse the command and return a number that is easier to work with
 * 
 */
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

/**
 * Read the next value from serial
 *  - A "value" is a entire string line, terminated by "\n"
 */
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

/**
 * Execute a command
 *  - parsed command integer as per method parseCommand()
 *  - commandValue as a string
 */
void executeCommand(const int parsedCmd, const char* commandValue){
  if(parsedCmd >= 2 && parsedCmd <= CMD_RELAY_MAX){
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

/**
 * Send initialization information for each relay and
 * subsequently subscibe to the MQTT channel
 * 
 */
void initAll(){
  DL("Initializing...");
  Serial.print("/initStart\n");
  
  int mode;
  for(int i = 0; i < RELAY_COUNT; i++){
    mode = digitalRead(RELAY_OFFSET + i);
    //publish the current value
    Serial.print(">relay");
    Serial.print(i);
    Serial.print("\n");
    if(mode == HIGH){
      Serial.print("ON\n");
    }else{
      Serial.print("OFF\n");
    }

    //subscribe to the value moving forward
    Serial.print("+relay");
    Serial.print(i);
    Serial.print("\n");
  }
}

/**
 * Set the relay status based on what the MQTT channel wanted
 */
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
