#include <WiServer.h>
#include <WProgram.h>
#include <EEPROM.h>
#include <stdio.h>

#include "TinyREST.h"

// The following structure contains the data necessary to perform the
// REST callbacks whenever a value is changing (and when there is a 
// watch for that value!). It binds the return path for the URL and the
// server declared with addServer().
#ifdef HAVE_SUBSCRIBE
#define MAXPATH (48)
struct cb_info {
  char path[MAXPATH];
  uint8_t srv_id;
};
#endif


// A set of SDRAM strings for constructing the JSON answers of the
// server. Most of these are used when returning back the status of the
// server.
const char JSON_RESPONSE[] PROGMEM = {"{\"result\":"};
const char JSON_TRUE[] PROGMEM = { "true}" };
const char JSON_FALSE[] PROGMEM = { "false}" };

const char JSON_COMMANDS[] PROGMEM = {"\"commands\":"};
const char JSON_SHARED[] PROGMEM = {"\"shared\":"};
const char JSON_COMMAND[] PROGMEM = {"\"command\":"};
const char JSON_ARGUMENTS[] PROGMEM = {"\"arguments\":"};
const char JSON_NEXT_VALUE[] PROGMEM = {"\","};
const char JSON_NEXT_OBJECT[] PROGMEM = {"},"};
const char JSON_NEXT_ARRAY[] PROGMEM = {"],"};
#ifdef HAVE_SUBSCRIBE
const char JSON_WATCHS[] PROGMEM = {"\"subscriptions\":"};
const char JSON_POSITION[] PROGMEM = {"\"position\":"};
const char JSON_VALUE[] PROGMEM = {"\"value\":"};
const char JSON_TYPE[] PROGMEM = {"\"type\":"};
const char JSON_PATH[] PROGMEM = {"\"path\":\""};
const char JSON_FREQUENCY[] PROGMEM = {"\"frequency\":"};
const char JSON_TYPE_APIN[] PROGMEM = {"\"apin\","};
const char JSON_TYPE_DPIN[] PROGMEM = {"\"dpin\","};
#ifdef HAVE_SHARED
const char JSON_TYPE_SHARED[] PROGMEM = {"\"shared\","};
#endif
const char JSON_TYPE_EEPROM[] PROGMEM = {"\"eeprom\","};
const char JSON_SERVERS[] PROGMEM = {"\"servers\":"};
const char JSON_IP[] PROGMEM = {"\"ip\":\""};
const char JSON_PORT[] PROGMEM = {"\"port\":"};
const char JSON_ID[] PROGMEM = {"\"id\":"};
#endif

// Respond the status of the server, this means the list of commands
// that it implements, but also, the list of subscriptions that are
// currently register, as well as the list of servers for reception
// of value changes that are known.
void TinyREST::respond_status() {
  WiServer.println('{');

  // Send back list of commands supported by the server.
  WiServer.print_P(JSON_COMMANDS);
  WiServer.println('[');
  for (int i=0; i<this->cmd_count;i++) {
    WiServer.print('{');
    WiServer.print_P(JSON_COMMAND);
    WiServer.print('\"');
    WiServer.print(this->cmds[i].cmd);
    WiServer.print_P(JSON_NEXT_VALUE);
    WiServer.print_P(JSON_ARGUMENTS);
    sprintf(buffer, "%u", this->cmds[i].len);
    WiServer.print(buffer);
    if (i<this->cmd_count-1) {
      WiServer.println_P(JSON_NEXT_OBJECT);
    } else {
      WiServer.println('}');
    }
  }
  // Send back the list of subscription and their details
#ifdef HAVE_SUBSCRIBE
  WiServer.println_P(JSON_NEXT_ARRAY);
  
  WiServer.print_P(JSON_WATCHS);
  WiServer.println('[');
  for (int i=0; i<this->watch_count;i++) {
    WiServer.print('{');
    WiServer.print_P(JSON_TYPE);
    switch(this->watchs[i].type) {
      case VALUE_WATCH_DPIN:
        WiServer.print_P(JSON_TYPE_DPIN);
        break;
      case VALUE_WATCH_APIN:
        WiServer.print_P(JSON_TYPE_APIN);
        break;
#ifdef HAVE_SHARED
      case VALUE_WATCH_SHARED:
        WiServer.print_P(JSON_TYPE_SHARED);
        break;
#endif
      case VALUE_WATCH_EEPROM:
        WiServer.print_P(JSON_TYPE_EEPROM);
        break;
    }
    WiServer.print_P(JSON_POSITION);
    sprintf(buffer, "%u,", this->watchs[i].position);
    WiServer.print(buffer);
    WiServer.print_P(JSON_FREQUENCY);
    sprintf(buffer, "%u,", this->watchs[i].freq);
    WiServer.print(buffer);
    WiServer.print_P(JSON_PATH);
    WiServer.print(((struct cb_info *)this->watchs[i].blind)->path);
    WiServer.print_P(JSON_NEXT_VALUE);
    WiServer.print_P(JSON_VALUE);
    sprintf(buffer, "%u", this->watchs[i].value);
    WiServer.print(buffer);
    if (i<this->watch_count-1) {
      WiServer.println_P(JSON_NEXT_OBJECT);
    } else {
      WiServer.println('}');
    }
  }
  
  // Send back the list of servers defined for the reception of
  // value changes.
  WiServer.println_P(JSON_NEXT_ARRAY);
  
  WiServer.print_P(JSON_SERVERS);
  WiServer.println('[');
  for (int i=0; i<this->server_count;i++) {
    WiServer.print('{');
    WiServer.print_P(JSON_ID);
    sprintf(buffer, "%u,", this->servers[i].id);
    WiServer.print(buffer);
    WiServer.print_P(JSON_IP);
    WiServer.print(this->servers[i].hostName);
    WiServer.print_P(JSON_NEXT_VALUE);
    WiServer.print_P(JSON_PORT);
    sprintf(buffer, "%u", this->servers[i].port);
    WiServer.print(buffer);
    if (i<this->server_count-1) {
      WiServer.println_P(JSON_NEXT_OBJECT);
    } else {
      WiServer.println('}');
    }
  }
#endif
  WiServer.println(']');  
  
  WiServer.println('}');
}


void TinyREST::respond_read_eeprom(int start, int end) {
  WiServer.print_P(JSON_RESPONSE);
  if (start == end) {
    sprintf(buffer, "%u", EEPROM.read(start));
    WiServer.print(buffer);
  } else {
    WiServer.print('[');
  
    for (int i = start; i<=end; i++) {
      sprintf(buffer, "%u", EEPROM.read(i));
      WiServer.print(buffer);
      if ((i + 1) <= (end))
        WiServer.print(',');
    }
  
    // Close brackets
    WiServer.print(']');
  }
  WiServer.print('}');
}

void TinyREST::send_true() {
  WiServer.print_P(JSON_RESPONSE);
  WiServer.print_P(JSON_TRUE);
}

void TinyREST::send_false() {
  WiServer.print_P(JSON_RESPONSE);
  WiServer.print_P(JSON_FALSE);
}

void TinyREST::send_int(int val) {
  WiServer.print_P(JSON_RESPONSE);
  sprintf(buffer, "%u", val);
  WiServer.print(buffer);
  WiServer.print('}');
}

void TinyREST::send_int_arr(unsigned int* arr, int len) {
  WiServer.print_P(JSON_RESPONSE);
  WiServer.print('[');
  for (int i = 0; i<len; i++) {
    sprintf(buffer, "%u", *(arr+i));
    WiServer.print(buffer);
    if ((i + 1) < (len))
      WiServer.print(',');
  }

  // Close brackets
  WiServer.print(']');
  WiServer.print('}');
}

// This is the function that returns the content of the web
// pages.  It analyses the URL (path) passed as an argument.
// In that path, function names and arguments are separated by
// slashes, the command being the keyword in that list.  If the
// command and its number of arguments is recognised, the 
// matching callback is executed.  When no command is given, 
// the servers sends back a JSON description of its current status
// including the list of commands that it supports, and the
// list of subscriptions currently in registered.
boolean TinyREST::handleURL(char* URL) {
  command_t *match;

  // Agnostically parsing the command by making sure req and
  // req_args point to the command and arguments separated by
  // the slashes in the path.  Note that this INSERTs string
  // endings in the incoming path to save memory.
  if (parseCommand(URL, &this->req, this->req_args) >= 0) {
#ifdef TINY_REST_DEBUG
    printCommand(&this->req, "Incoming REST call", this->req_args);
#endif
    // Find the command among those that the server knows of
    // and perform the callback.
    match = findCommand(this->req.cmd, this->req.len);
    if (match) {
      int res;
#ifdef TINY_REST_DEBUG
      printCommand(match, "Command was recognised!", this->req_args);
#endif
      res = match->callback(this, match->cmd, this->req.len, this->req_args, match->blind);
      switch (res) {
        case RESPONSE_OK:
          send_true();
          return true;
          break;
        case RESPONSE_ERROR:
          send_false();
          return false;
          break;
        case RESPONSE_INLINE_OK:
          return true;
          break;
        case RESPONSE_INLINE_ERROR:
          return false;
          break;
      }
    } else if (strcmp(this->req.cmd, "")==0) {
      // Unrecognised and empty command will return the status
      respond_status();
      return true;
    }
  }
  return false;
}


#ifdef HAVE_SUBSCRIBE
// We will hold a table of so-called responders, these are basically the
// context needed to perform GET requests whenever a value that we watch
// has changed and should trigger a GET.  We need to have the GETrequest()
// objects on the stack, which make the following code rather ugly.
#define MAXRESPONDERS 3  // This must match the nb of __response vars below!!!
static uint8_t nullIP[] = {0,0,0,0};
GETrequest __response_1(nullIP, 0, "", "");
GETrequest __response_2(nullIP, 0, "", "");
GETrequest __response_3(nullIP, 0, "", "");
struct __responder {
  GETrequest *r;    // Pointer to (stack) GETrequest
  char *path;       // Complete URL path for the request (with value!) 
};
static struct __responder responses[MAXRESPONDERS];
static boolean __responder_initialised = false;

// Find a responder that is available to perform a GET request and return
// a pointer to it.  The function will take the opportunity to perform some
// garbage collection by freeing all memory by the responses that have
// already been performed. It returns NULL if no responder can be found!
static struct __responder *findResponder() {
  struct __responder *rsp = NULL;
  
  // Free response URL path for inactive responders.
  for (int i=0;i<MAXRESPONDERS;i++) {
    rsp = &responses[i];  // Helper variable
    
    // Only free memory in inactive responders otherwise we
    // would trash the memory before the GET resquest has been
    // executed.
    if (!rsp->r->isActive()) {
      if (rsp->path!=NULL) {
        free(rsp->path);
        rsp->path = NULL;
      }
    }
  }

  // Now look for a responder with a GET response that can be used, i.e. 
  // a get response that is not active at present.
  for (int i=0;i<MAXRESPONDERS;i++) {
    rsp = &responses[i];
    if (!rsp->r->isActive()) {
      return rsp;
    }
  }
  
  return NULL;  // No responder could be found.
}

// This function is executed whenever a value callback should
// be issued, i.e. whenever a value that we are watching has
// changed and it is time to mediate this change to remote web
// servers.  The function builds a GET request by adding the value
// to the URL of the callback and attempts to perform that
// request.
static boolean value_callback(TinyREST *srv, int position, int value, int type, void *blind) {
  struct cb_info *cb = (struct cb_info * )blind;
  server_t *s = NULL;
  
  // Initialise the responder array, this is ugly, but was the only
  // solution that I could find, given that there are no way to 
  // dynamically create new objects...
  if (!__responder_initialised) {
    responses[0].r = &__response_1;
    responses[0].path = NULL;
    responses[1].r = &__response_2;
    responses[1].path = NULL;
    responses[2].r = &__response_3;
    responses[2].path = NULL;
    __responder_initialised = true;
  }
  
  // Look among our known servers for the one that matches the
  // identifier of the server associated to the callback.  
  s = srv->findServer(cb->srv_id);
  if (s == NULL) {
#ifdef TINY_REST_DEBUG
    Serial.println("Could not find server associated to callback!");
#endif
  } else {
    struct __responder *rsp = NULL;
    
    // Find an available responder context to perform the GET request
    // associated to the callback.
    rsp = findResponder();
    if (rsp == NULL) {
#ifdef TINY_REST_DEBUG
      Serial.println("Could not find any available responder!");
#endif
    } else {
      int len = strlen(cb->path);
    
      // Construct the returning path with current value, we append the
      // value to the (static) path that was given at the time of the
      // registration.
      rsp->path = (char *)malloc(len+8);
      strcpy(rsp->path, cb->path);
      sprintf(&rsp->path[len], "%u", value);
      
      // Fill in the GETrequest object witht the necessary values. This is
      // particularily ugly since it requires a knowledge of how these
      // objects are actually constructed. But, but, I couldn't find any
      // other nicer way since there are no functions to initiate these
      // properly.
      uip_ipaddr(&rsp->r->ipAddr, s->ip[0], s->ip[1], s->ip[2], s->ip[3]);
      rsp->r->port = htons(s->port);
      rsp->r->hostName = s->hostName;
      rsp->r->URL = rsp->path;
      rsp->r->setReturnFunc(NULL);
      // Submit the get request on the queue.
      rsp->r->submit();
      return true;
    }
  }
  
  return false;
}

// Convert an encoded URL to its unencoded form.  The function only
// recognises the %xx form for the escapes.  It unescape DIRECTLY in the
// string for saving memory.
static char *unescape_url(char *url) {
  char *src;
  char *dest = url;
  char *escape = NULL;
  
  for (src=url; *src; src++) {
    if (*src == '%') {
      escape = src;
    } else if (escape == NULL) {
      *dest = *src;
      dest++;
    } else {
      if (src-escape == 2) {
        char ascii_buf[3];
        int ascii_code;
        ascii_buf[0] = escape[1];
        ascii_buf[1] = escape[2];
        ascii_buf[2] = '\0';
        sscanf(ascii_buf, "%x", &ascii_code);
        *dest = char(ascii_code);
        dest++;
        escape = NULL;
      }
    }
  }
  *dest = '\0';
    
  return url;
}
#endif

// Below are the SDRAM strings for the commands that the server
// supports by default.
#ifdef HAVE_SHARED
static char cmd_read_shared[] = {"shared_read"};
static char cmd_write_shared[] = {"shared_write"};
#endif
static char cmd_read_eeprom[] = {"eeprom_read"};
static char cmd_write_eeprom[] = {"eeprom_write"};
static char cmd_read_dpin[] = {"dpin_read"};
static char cmd_write_dpin[] = {"dpin_write"};
static char cmd_dpin_mode[] = {"dpin_mode"};
static char cmd_read_apin[] = {"apin_read"};
#ifdef HAVE_SUBSCRIBE
static char cmd_subscribe[] = {"subscribe"};
static char cmd_unsubscribe[] = {"unsubscribe"};
static char cmd_add_server[] = {"server_add"};
static char cmd_remove_server[] = {"server_remove"};
#endif

// This function implements the "big-switch", i.e. it dispatch
// the incoming command to its relevant function in the code of
// the server.
static int cmd_dispatcher(TinyREST *srv, char *cmd, int len, char **args, void *blind) {
  if (strcmp(cmd, cmd_read_dpin)==0) {
    srv->send_int(digitalRead(atoi(args[0])));
    return RESPONSE_INLINE_OK;
  } else if (strcmp(cmd, cmd_read_apin)==0) {
    srv->send_int(analogRead(atoi(args[0])));
    return RESPONSE_INLINE_OK;
#ifdef HAVE_SHARED
  } else if (strcmp(cmd, cmd_read_shared)==0) {
    if (len == 0) {
      srv->send_int_arr(srv->shared, SHARED_LEN);
      return RESPONSE_INLINE_OK;
    } else if (len == 1) {
      srv->send_int(srv->shared[atoi(args[0])]);
      return RESPONSE_INLINE_OK;
    } else {
      return RESPONSE_ERROR;
    }
#endif
  } else if (strcmp(cmd, cmd_read_eeprom)==0) {
    if (len == 1) {
      srv->respond_read_eeprom(atoi(args[0]), atoi(args[0]));
      return RESPONSE_INLINE_OK;
    } else if (len == 2) {
      srv->respond_read_eeprom(atoi(args[0]), atoi(args[1]));
      return RESPONSE_INLINE_OK;
    } else {
      return RESPONSE_ERROR;
    }
#ifdef HAVE_SHARED
  } else if (strcmp(cmd, cmd_write_shared)==0) {
    if (atoi(args[0]) < SHARED_LEN) {
      srv->shared[atoi(args[0])] = atoi(args[1]);
    } else {
      return RESPONSE_ERROR;
    }
#endif
  } else if (strcmp(cmd, cmd_write_dpin)==0) {
    if (atoi(args[1])) { 
      digitalWrite(atoi(args[0]), HIGH);
    } else {
      digitalWrite(atoi(args[0]), LOW);
    }
  } else if (strcmp(cmd, cmd_write_eeprom)==0) {
    EEPROM.write(atoi(args[0]), atoi(args[1]));
    return RESPONSE_INLINE_OK;
  } else if (strcmp(cmd, cmd_dpin_mode)==0) {
    if (atoi(args[1])) {
      pinMode(atoi(args[0]), INPUT);
    } else {
      pinMode(atoi(args[0]), OUTPUT);
    }
#ifdef HAVE_SUBSCRIBE
  } else if (strcmp(cmd, cmd_subscribe)==0) {
    int type = atoi(args[0]);

    // Discard not-allowed types, this really is an if-statement written
    // as a switch to allow for an #ifdef around HAVE_SHARED.
    switch (type) {
      case VALUE_WATCH_DPIN:
      case VALUE_WATCH_APIN:
      case VALUE_WATCH_EEPROM:
#ifdef HAVE_SHARED
      case VALUE_WATCH_SHARED:
#endif
        break;
      default:
        return RESPONSE_ERROR;
    }  	
    if (len == 4) {
		// subscribe <type> <position> <server> <path>
      struct cb_info *nfo = (struct cb_info *)malloc(sizeof(struct cb_info));
      strlcpy(nfo->path, unescape_url(args[3]), MAXPATH);
      nfo->srv_id = atoi(args[2]);
      vwatch_t *w = srv->addWatch(atoi(args[1]), type, value_callback, nfo);
	  if (w==NULL)
		return RESPONSE_ERROR;
    } else if (len == 5) {
		// subscribe <type> <position> <server> <freq> <path>
      struct cb_info *nfo = (struct cb_info *)malloc(sizeof(struct cb_info));
      strlcpy(nfo->path, unescape_url(args[4]), MAXPATH);
      nfo->srv_id = atoi(args[2]);
      vwatch_t *w = srv->addWatch(atoi(args[1]), type, atoi(args[3]), value_callback, nfo);
	  if (w==NULL)
		return RESPONSE_ERROR;
    } else {
      return RESPONSE_ERROR;
    }
  } else if (strcmp(cmd, cmd_unsubscribe)==0) {
	// unsubscribe <type> <position>
	vwatch_t *w = srv->findWatch(atoi(args[1]), atoi(args[0]));
    if (w) {
      free(w->blind);
      srv->removeWatch(w);
    } else {
      return RESPONSE_ERROR;
    }
  } else if (strcmp(cmd, cmd_add_server)==0) {
    server_t *s = srv->addServer(atoi(args[0]), args[1], atoi(args[2]));
	if (s==NULL) return RESPONSE_ERROR;
  } else if (strcmp(cmd, cmd_remove_server)==0) {
    srv->removeServer(atoi(args[0]));
#endif
  } else {
    return RESPONSE_ERROR;
  }
  
  return RESPONSE_OK;
}

#ifdef TINY_REST_DEBUG
void TinyREST::printCommand(command_t *r, char *header, char *args[])
{
  if (header) Serial.println(header);
  Serial.print("CMD: ");
  Serial.println(r->cmd);
  sprintf(buffer, "%u", r->len);
  Serial.print(buffer);
  if (args == NULL) {
    Serial.println(" args.");
  } else {
    Serial.print(" ARGS: ");
    for (int i=0; i<r->len; i++) {
      if (args[i]) {
        Serial.print(args[i]);
        if (i<r->len - 1) Serial.print(", ");
      }
    }
    Serial.println("");
  }
}
#endif

// Add a new command to the server, arranging for a callback to
// be called whenever a matching command with a matching number
// of arguments is received. Return a pointer to the new command
// structure or NULL if problems.
command_t *TinyREST::addCommand(char *cmd, uint8_t len, CommandCallback cb, void *blind)
{
  if (cmd_count < MAX_CMDS) {
    cmds[cmd_count].cmd = cmd;  // Note we COPY the pointer for
                                // saving memory!
    cmds[cmd_count].len = len;
    cmds[cmd_count].callback = cb;
    cmds[cmd_count].blind = blind;
#ifdef TINY_REST_DEBUG
    printCommand(&cmds[cmd_count], "Added new command", NULL);
#endif
    return &cmds[cmd_count++];
  }
  return NULL;
}


// Add a new command to the server, arranging for a callback to
// be called whenever a matching command with a matching number
// of arguments is received. Return a pointer to the new command
// structure or NULL if problems.
command_t *TinyREST::addCommand(char *cmd, uint8_t len, CommandCallback cb) {
  return addCommand(cmd, len, cb, NULL);
}


// Remove an existing command from the list of commands that we
// are listening to.  Note that we test on pointers, so you will
// have to use findCommand() before removing a command in most
// cases.
boolean TinyREST::removeCommand(command_t *cmd) {
  boolean found = false;
  for (uint8_t i=0; i<cmd_count; i++) {
    if (&cmds[i] == cmd) {
      found = true;
      cmd_count --;
    }
    if (found && (i<cmd_count))
      cmds[i] = cmds[i+1];
  }
  
  return found;
}

// find a command which name and number of arguments matches the
// arguments to the method and return a pointer to its structure,
// NULL if not found or no match.
command_t *TinyREST::findCommand(char *cmd, uint8_t len) {
  for (uint8_t i=0; i<cmd_count; i++) {
    if (strcasecmp(cmds[i].cmd, cmd)==0 && cmds[i].len == len) {
      return &cmds[i];
    }
  }
  
  return NULL;
}

// find a command which name matches the
// arguments to the method and return a pointer to its structure,
// NULL if not found or no match.  If several commands with the
// same name exist (but with different number of arguments),
// return the first matching one.
command_t *TinyREST::findCommand(char *cmd) {
  for (uint8_t i=0; i<cmd_count; i++) {
    if (strcasecmp(cmds[i].cmd, cmd)==0) {
      return &cmds[i];
    }
  }
  
  return NULL;
}

// Parse an incoming path into a command and its arguments.  The
// function will insert string endings in the incoming path and 
// arrange for req and args to point into the incoming path.  This
// is potentially dangerous, but on purpose to avoid copies and
// save memory.
int TinyREST::parseCommand(char *URL, command_t *req, char *args[])
{
  // Empty arguments and command
  req->cmd = NULL;
  req->len = 0;
  for (int i=0; i<MAXARGS; i++)
    args[i] = NULL;
    
  // Check if the commanded matches starts with a "/"
  if (strncmp(URL, "/", 1) == 0) {
    unsigned int slash_count = 0;
    char c;
 
    // advance character by character in the incoming URL,
    // dumping result of analysis in the request and args and
    // inserting '0' instead of the slashes.
    for(int i=0; (c = *(URL+i)); ++i) {
      if (c == '/') {
        // parse cmd
        if (slash_count == 0) {
          req->cmd = (URL+i+1);
        } else if (slash_count > MAXARGS) {
          // Exit, we are at the maximum number of supported
          // arguments.
          *(URL+i) = '\0';
          break;
        } else {
          args[req->len] = (URL+i+1);
          *(URL+i) = '\0';
          req->len ++;
        }
        slash_count++;
      }
    }
    return req->len;
  }
  
  return -1;
}


// This method should be called for each loop.  It will check
// what the current time is and perform the appropriate 
// subscription callbacks. Calling this is essential if you want
// to be able to receive some callbacks.
void TinyREST::loop() {
#ifdef HAVE_SUBSCRIBE
  unsigned long now = millis();
  
  for (uint8_t i=0; i<watch_count; i++) {
    testWatch(&watchs[i], now);
  }
#endif
}

void TinyREST::init() {
  // Add standard set of commands.
#ifdef HAVE_SHARED
  command_t *read_shared = this->addCommand(cmd_read_shared, 0, cmd_dispatcher);
  command_t *read_shared_single = this->addCommand(cmd_read_shared, 1, cmd_dispatcher);
  command_t *write_shared = this->addCommand(cmd_write_shared, 2, cmd_dispatcher);
#endif
  command_t *read_eeprom = this->addCommand(cmd_read_eeprom, 2, cmd_dispatcher);
  command_t *read_eeprom_single = this->addCommand(cmd_read_eeprom, 1, cmd_dispatcher);
  command_t *write_eeprom = this->addCommand(cmd_write_eeprom, 2, cmd_dispatcher);
  command_t *read_dpin = this->addCommand(cmd_read_dpin, 1, cmd_dispatcher);
  command_t *write_dpin = this->addCommand(cmd_write_dpin, 2, cmd_dispatcher);
  command_t *dpin_mode = this->addCommand(cmd_dpin_mode, 2, cmd_dispatcher);
  command_t *read_apin = this->addCommand(cmd_read_apin, 1, cmd_dispatcher);
#ifdef HAVE_SUBSCRIBE
  command_t *subscribe = this->addCommand(cmd_subscribe, 5, cmd_dispatcher);
  command_t *subscribe_nofreq = this->addCommand(cmd_subscribe, 4, cmd_dispatcher);
  command_t *unsubscribe = this->addCommand(cmd_unsubscribe, 2, cmd_dispatcher);
  command_t *add_server = this->addCommand(cmd_add_server, 3, cmd_dispatcher);
  command_t *remove_server = this->addCommand(cmd_remove_server, 1, cmd_dispatcher);
#endif
}

TinyREST::TinyREST() {
  this->cmd_count = 0;
#ifdef HAVE_SUBSCRIBE
  this->watch_count = 0;
  this->server_count = 0;
#endif
#ifdef HAVE_SHARED
  for (int i=0; i<SHARED_LEN; i++)
    shared[i] = 0;
#endif
};
