// This class implements a REST-like server with, in its
// basic installation, the ability to perform two major kind
// of operations: writing/reading from the pins and the EEPROM,
// and subscribing to value changes on these pins and the
// EEPROM.
// 
// Incoming commands are separated by slashes, implementing a
// REST-like interface. The first arguments is the name of the
// command and the following arguments the argument that are
// passed to the command.
//
// Responses are JSON-formatted for easy integration and parsing
//
// Calling the server without any argument at all will return
// a JSON expression describing the commands that its implements
// and its current status when it comes to suscriptions.
//
// Because of limitations in the length of the incoming HTTP
// requests, subscribing to value (changes) is a two-steps
// process. You will have to first describe a server which 
// contains the IP address and port number of your receiving
// server (no DNS allowed!).  Secondly, you will bound this server
// to a pin or EEPROM position and arrange for a path within
// that (HTTP) server to receive the value whenever it changes.
// The value will automatically be appended to the path for
// each callback.
//
// In addition to the EEPROM and the analogue and digital pins, the
// servers contains an agnostic array that you can write and read from.
// The idea is to be able to use this array as a buffer between different
// servers, a central place where you would store application-relevant
// data.
//
// The commands that the server implements are the following:
//
// subscribe <type> <position> <server> <path>
//   where  <type> is one of  0: digital pin
//                            1: analogue pin
//                            2: EEPROM
//                            3: shared array
//          <position> is a positive integer specifying which value to watch
//          <server> is the identifier of the server, see below
//          <path> is an escaped path where to receive the callback at server
// subscribe <type> <position> <server> <freq> <path>
// unsubscribe <type> <position>
// 
// The class provides an API for adding new commands if ever
// you wanted to do that.

#include <WiServer.h>
#include <WiShield.h>
#include <EEPROM.h>

#ifndef TinyREST_h
#define TinyREST_h

//#define TINY_REST_DEBUG

// When defined, the HAVE_SUBSCRIBE constant enables subscriptions to
// value changes and server definitions.
#define HAVE_SUBSCRIBE
// When defined, the HAVE_SHARED constant controls the ability for the
// server to have a shared array for sharing data between federations of
// servers.
#define HAVE_SHARED

#ifdef HAVE_SHARED
#define SHARED_LEN   (16)
#endif
#define MAX_CMDS     (18)
#define MAX_WATCHS   (5)
#define MAX_SERVERS  (2)
#define BUFSIZE      (16)  // Size of buffer for conversions, room for an IP adr

class TinyREST;

#define RESPONSE_OK           (0)
#define RESPONSE_ERROR        (1)
#define RESPONSE_INLINE_OK    (2)
#define RESPONSE_INLINE_ERROR (3)
typedef int (*CommandCallback)(TinyREST *, char *, int, char **, void *);

#define MAXARGS    5          // Maximum number of arguments to commands.
typedef struct command {
  char *cmd;                  // Command
  uint8_t len;                // Number of arguments to command
  CommandCallback callback;   // Function to callback on match
  void *blind;                // Blind argument
} command_t;


#ifdef HAVE_SUBSCRIBE
#define VALUE_WATCH_DPIN (0)
#define VALUE_WATCH_APIN (1)
#define VALUE_WATCH_EEPROM (2)
#ifdef HAVE_SHARED
#define VALUE_WATCH_SHARED (3)
#endif

typedef boolean (*ValueWatchCallback)(TinyREST *, int, int, int, void *);
typedef struct vwatch {
  int position;
  int value;
  uint8_t type;
  unsigned long freq;
  ValueWatchCallback callback;    // Function to callback on match
  void *blind;                    // Blind argument
  unsigned long lastChecked;      // Last time the value was checked.
} vwatch_t;


typedef struct server {
  uint8_t id;
  uint8_t ip[4];
  char hostName[20];
  unsigned short port;
} server_t;
#endif


class TinyREST {
public:
#ifdef HAVE_SHARED
  unsigned int shared[SHARED_LEN];
#endif
  char buffer[BUFSIZE];           // Buffer for conversions to strings.

  void init();
  boolean handleURL(char *URL);
  void loop();
  
  // Handling of recognised commands
  command_t *addCommand(char *cmd, uint8_t len, CommandCallback cb, void *blind);
  command_t *addCommand(char *cmd, uint8_t len, CommandCallback cb);
  boolean removeCommand(command_t *cmd);
  command_t *findCommand(char *cmd);
  command_t *findCommand(char *cmd, uint8_t len);

#ifdef HAVE_SUBSCRIBE
  // Handling of watches on values
  vwatch_t *addWatch(int, uint8_t, unsigned long, ValueWatchCallback, void *); 
  vwatch_t *addWatch(int, uint8_t, ValueWatchCallback, void *); 
  vwatch_t *addWatch(int, uint8_t, unsigned long, ValueWatchCallback); 
  vwatch_t *addWatch(int, uint8_t, ValueWatchCallback); 
  boolean removeWatch(vwatch_t *watch);
  vwatch_t *findWatch(int position, uint8_t type);

  // Handling of remote servers for subscriptions
  server_t *addServer( uint8_t, char *, unsigned short);
  boolean removeServer(uint8_t);
  server_t *findServer(uint8_t);
#endif
  
  TinyREST();
  void respond_status();
  void respond_read_eeprom(int start, int end);
  void send_true();
  void send_false();
  void send_int(int val);
  void send_int_arr(unsigned int* arr, int len);

private:
  command_t cmds[MAX_CMDS];       // Commands supported by server
  command_t req;                  // Last incoming request.
  char *req_args[MAXARGS];        // Arguments to last incoming request.
  int cmd_count;                  // Number of commands in server
#ifdef HAVE_SUBSCRIBE
  vwatch_t watchs[MAX_WATCHS];    // Value watched by the server
  int watch_count;                // Number of values watched
  struct server servers[MAX_SERVERS];  // List of known servers for callbacks.
  int server_count;               // Current number of servers.
#endif

  int parseCommand(char *URL, command_t *req, char *args[]);
#ifdef TINY_REST_DEBUG
  void printCommand(command_t *c, char *header, char *args[]);
#endif
#ifdef HAVE_SUBSCRIBE
  int getWatch(vwatch_t *w, unsigned long now);
  boolean testWatch(vwatch_t *, unsigned long now);
  boolean testWatch(vwatch_t *);
#endif
};

#endif
