#include "headers.h"

enum HTTPMethod { UNKNOWN,
                  GET,
                  POST };

enum HTTPVariable { VAR_UNKNOWN,
                    IP,
                    PORT,
                    TOPIC };
struct HTTPData {
  HTTPMethod method;
  int contentLength;
};

const char configHTML[] PROGMEM = {
  "<!DOCTYPE html><html><head><title>Garden Controller</title></head>"
  "<body><h2>Garden Controller Setup</h2><form action=\"/\" method=\"post\">%3"
  "<label for=\"i\">MQTT Server:</label><br><input type=\"text\" id=\"i\" name=\"i\" value=\"%0\"><br>"
  "<label for=\"p\">MQTT Port:</label><br><input type=\"number\" id=\"p\" name=\"p\" value=\"%1\"><br>"
  "<label for=\"t\">MQTT Topic:</label><br><input type=\"text\" id=\"t\" name=\"t\" value=\"%2\"><br><br>"
  "<input type=\"submit\" value=\"Save\">&nbsp;<input type=\"button\" value=\"Refresh\" onclick=\"window.location.href='/'\"></form>"
  "<p>Details <a href=\"https://github.com/cshbicos/gardencontrol\">here</a></p></body></html>"
};
const char errorHTML[] PROGMEM = { "<!DOCTYPE html><html><body>Error</body></html>" };


void setHTMLMQTTVariable(enum HTTPVariable readVariable, Runtime *runtime, String *data);

void runHTTPServerLoop(Runtime *runtime) {
  // listen for incoming clients
  EthernetClient client = runtime->httpServer->available();
  if (!client)
    return;

  DEBUG_LINE(F("new HTTP client"));
  bool isHTTPPOST = false;
  String curLine = "";
  HTTPData httpData = { UNKNOWN, 0 };

  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      DEBUG_STRING(c);
      // if you've gotten to the end of the line (received a newline
      // character) and the line is blank, the HTTP request has ended,
      // so you can send a reply
      if (c == '\n') {
        if (parseHTTPHeaderLine(&curLine, &httpData)) {
          switch (httpData.method) {
            case GET:
              sendHTMLPage(&client, runtime);
              break;
            case POST:
              parseHTMLForm(&client, runtime, &httpData);
              sendHTMLPage(&client, runtime);
              break;
            default:
              sendHTMLErrorPage(&client);
              break;
          }
          break;
        } else {
          curLine = "";
        }
      } else if (c != '\r') {
        curLine += c;
      }
    }
  }
  // give the web browser time to receive the data
  delay(1);
  // close the connection:
  client.stop();
}

bool parseHTTPHeaderLine(String *curLine, struct HTTPData *httpData) {
  if (curLine->length() == 0)
    return true;

  if (curLine->startsWith("GET / ") && httpData->method == UNKNOWN) {
    httpData->method = GET;
  } else if (curLine->startsWith("POST / ") && httpData->method == UNKNOWN) {
    httpData->method = POST;
  } else if (curLine->startsWith("Content-Length:")) {
    String lengthStr = curLine->substring(16);
    lengthStr.trim();
    httpData->contentLength = lengthStr.toInt();
  }
  return false;
}

void parseHTMLForm(EthernetClient *client, Runtime *runtime, HTTPData *httpData) {
  DEBUG_LINE(F("POST data received"));
  int bytesRead = 0;
  enum HTTPVariable readVariable = VAR_UNKNOWN;
  String data = "";

  while (bytesRead < httpData->contentLength && client->connected()) {
    if (client->available()) {
      char c = client->read();
      if (readVariable == VAR_UNKNOWN) {
        switch (c) {
          case 'i': readVariable = IP; break;
          case 'p': readVariable = PORT; break;
          case 't': readVariable = TOPIC; break;
          default: readVariable = VAR_UNKNOWN; break;
        }
      } else if (c == '=') {
        //we can ignore this one
      } else if (c == '&') {
        //that's it
        setHTMLMQTTVariable(readVariable, runtime, &data);
        data = "";
        readVariable = VAR_UNKNOWN;
      } else {
        data += c;
      }
      bytesRead++;
    }
  }
  setHTMLMQTTVariable(readVariable, runtime, &data);

  if (isMQTTSettingsValid(runtime)) {
    writePersistedSettings(runtime);
    runtime->mqttRuntime->state = NEW;
  } else {
    runtime->mqttRuntime->state = INVALID;
  }
}

void setHTMLMQTTVariable(enum HTTPVariable readVariable, Runtime *runtime, String *data) {

  switch (readVariable) {
    case IP:
      runtime->mqttRuntime->ip = urlDecode(data);
      DEBUG_LINE("MQTT IP set to " + runtime->mqttRuntime->ip);
      break;
    case PORT:
      runtime->mqttRuntime->port = urlDecode(data);
      DEBUG_LINE("MQTT Port set to " + runtime->mqttRuntime->port);
      break;
    case TOPIC:
      runtime->mqttRuntime->topic = urlDecode(data);
      DEBUG_LINE("MQTT Topic set to " + runtime->mqttRuntime->topic);
      break;
  }
}

String urlDecode(String *str) {
  String decoded = "";
  char c;
  char code0;
  char code1;

  for (int i = 0; i < str->length(); i++) {
    c = str->charAt(i);

    if (c == '+') {
      decoded += ' ';
    } else if (c == '%') {
      // Get the next two characters as hex digits
      i++;
      code0 = str->charAt(i);
      i++;
      code1 = str->charAt(i);
      c = (hexToInt(code0) << 4) | hexToInt(code1);
      decoded += c;
    } else {
      decoded += c;
    }
  }

  return decoded;
}

int hexToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

void sendHTMLErrorPage(EthernetClient *client) {
  client->println(F("HTTP/1.1 404 Not Found"));
  int htmlContentLength = strlen_P(errorHTML);
  client->print(F("Content-Length: "));
  client->println(htmlContentLength);
  client->println("");
  for (int i = 0; i < htmlContentLength; i++) {
    client->print(pgm_read_byte_near(errorHTML + i));
  }
}

void sendHTMLPage(EthernetClient *client, Runtime *runtime) {
  // send a standard HTTP response header
  client->println(F("HTTP/1.1 200 OK"));
  client->println(F("Content-Type: text/html"));
  client->println(F("Connection: close"));  // the connection will be closed after completion of the response

  int htmlContentLength = strlen_P(configHTML) - 8;  //8 is length of %0,%1,%2,%3
  htmlContentLength += runtime->mqttRuntime->ip.length();
  htmlContentLength += runtime->mqttRuntime->port.length();
  htmlContentLength += runtime->mqttRuntime->topic.length();

  String connected = "";
  switch (runtime->mqttRuntime->state) {
    case CONNECTED:
      connected = F("<p style=\"color:green\">Connection Successful</p>");
      break;
    case DISCONNECTED:
      connected = F("<p style=\"color:red\">Connection Not Successful</p>");
      break;
    case INVALID:
      connected = F("<p style=\"color:red\">Settings Not Valid</p>");
      break;
    case NEW:
      connected = F("<script>window.setTimeout(()=>{window.location.href='/'},1000)</script>");
      break;
  }
  htmlContentLength += connected.length();

  client->print(F("Content-Length: "));
  client->println(htmlContentLength);
  client->println();

  htmlContentLength = strlen_P(configHTML);
  bool replaceNext = false;
  for (int i = 0; i < htmlContentLength; i++) {
    char c = pgm_read_byte_near(configHTML + i);
    if (replaceNext) {
      replaceNext = false;
      switch (c) {
        case '0': client->print(runtime->mqttRuntime->ip); break;
        case '1': client->print(runtime->mqttRuntime->port); break;
        case '2': client->print(runtime->mqttRuntime->topic); break;
        case '3': client->print(connected); break;
      }

      continue;
    } else if (c == '%') {
      replaceNext = true;
      continue;
    }
    client->print(c);
  }
}