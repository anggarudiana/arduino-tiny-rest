#include <WiServer.h>
#include <WProgram.h>
#include <EEPROM.h>
#include <stdio.h>

#include "TinyREST.h"

#ifdef HAVE_SUBSCRIBE

// Find an existing server by its identifier and return a pointer
// to the server structure, or NULL if not found.
server_t *TinyREST::findServer(uint8_t id)
{
  for (uint8_t i=0; i<server_count; i++) {
    if (servers[i].id == id) {
      return &servers[i];
    }
  }
  
  return NULL;
}

// Add a new server to our list of known server.  If there is
// already a server with that identifier, the value will be 
// updated instead.  The ip string should be a properly formatted
// IPv4 address.
server_t *TinyREST::addServer(uint8_t id, char *ip, unsigned short port)
{
  server_t *srv = findServer(id);
  
  if (srv == NULL) {
    if (server_count < MAX_SERVERS) {
      srv = &servers[server_count++];
    }
  }
  
  if (srv != NULL) {
    // Update the server structure.  We arrange for "hostName" to
    // be a print of the IP address, this will be used when
    // performing GET request on callbacks.
    srv->id = id;
    sscanf(ip,"%u.%u.%u.%u",
              &srv->ip[0], &srv->ip[1], &srv->ip[2], &srv->ip[3]);
    sprintf(srv->hostName, "%u.%u.%u.%u",
            srv->ip[0], srv->ip[1], srv->ip[2], srv->ip[3]);
    srv->port = port;
  }
  return srv;
}

// REmove a server given its identifier.
boolean TinyREST::removeServer(uint8_t id)
{
  boolean found = false;
  for (uint8_t i=0; i<server_count; i++) {
    if (servers[i].id == id) {
      found = true;
      server_count --;
    }
    if (found && (i<server_count))
      servers[i] = servers[i+1];
  }
  
  return found;
}
#endif
