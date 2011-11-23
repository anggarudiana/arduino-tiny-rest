#include <WiServer.h>
#include <WProgram.h>
#include <EEPROM.h>
#include <stdio.h>

#include "TinyREST.h"

#ifdef HAVE_SUBSCRIBE
// Add a value watch, i.e. arrange for a given value (see below)
// to be watched with a given frequency (expressed in millisecs)
// and to produce a callback every time the value has changed.
// the recognised types are DPIN, APIN, EEPROM and SHARED. The
// last one being a shared array that can be used for central
// storage in distributed settings. Returns a pointers to the
// structure that will keep the state of the watch, or NULL
// on problems.
vwatch_t *TinyREST::addWatch(int position, uint8_t type, unsigned long freq, ValueWatchCallback cb, void *blind)
{
  if (watch_count < MAX_WATCHS) {
    watchs[watch_count].position = position;
    watchs[watch_count].type = type;
    watchs[watch_count].freq = freq;
    watchs[watch_count].callback = cb;
    watchs[watch_count].blind = blind;
    getWatch(&watchs[watch_count], 0);
    return &watchs[watch_count++];
  }
  return NULL;
}

vwatch_t *TinyREST::addWatch(int position, uint8_t type, ValueWatchCallback cb, void *blind)
{
  return addWatch(position, type, 0, cb, blind);
}

vwatch_t *TinyREST::addWatch(int position, uint8_t type, unsigned long freq, ValueWatchCallback cb)
{
  return addWatch(position, type, freq, cb, NULL);
}

vwatch_t *TinyREST::addWatch(int position, uint8_t type, ValueWatchCallback cb)
{
  return addWatch(position, type, cb, NULL);
}

// Remove an existing watch, note that comparison is done on
// pointers so you will probably have to call findWatch before
// removing.
boolean TinyREST::removeWatch(vwatch_t *w) {
  boolean found = false;
  for (uint8_t i=0; i<watch_count; i++) {
    if (&watchs[i] == w) {
      found = true;
      watch_count --;
    }
    if (found && (i<watch_count))
      watchs[i] = watchs[i+1];
  }
  
  return found;
}

// Find an existing watch.
vwatch_t *TinyREST::findWatch(int position, uint8_t type) {
  for (uint8_t i=0; i<watch_count; i++) {
    if (watchs[i].type == type && watchs[i].position == position) {
      return &watchs[i];
    }
  }
  
  return NULL;
}


// Actualise the value of a watch, depending on its type.  If the
// time (now) is 0, the method will first get the current time
// to mark at which time it was actualised.
int TinyREST::getWatch(vwatch_t *w, unsigned long now) {
  if (now == 0) now = millis();
  switch (w->type) {
    case VALUE_WATCH_DPIN:
      w->value = digitalRead(w->position);
      w->lastChecked = now;
      break;
    case VALUE_WATCH_APIN:
      w->value = analogRead(w->position);
      w->lastChecked = now;
      break;
    case VALUE_WATCH_EEPROM:
      w->value = EEPROM.read(w->position);
      w->lastChecked = now;
      break;
    case VALUE_WATCH_SHARED:
      w->value = this->shared[w->position];
      w->lastChecked = now;
      break;
  }
  return w->value;
}

// Test a watch, i.e. test if it is time to actualise, 
// actualise if it was so and, in relevant cases, perform
// the callback.  The current time is supposed to be passed
// as an argument.
boolean TinyREST::testWatch(vwatch_t *w, unsigned long now)
{
  // Don't do anything if it's not time yet...
  if (w->lastChecked + w->freq < now) {
    // remember old value and actualise the watch.
    int oldval = w->value;
    Serial.println("testing: ");
    getWatch(w, now);
    // If it has changed, perform the callback if we have one.
    if (w->value != oldval && w->callback) {
      Serial.println("CHANGED");
      w->callback(this, w->position, w->value, w->type, w->blind);
      return true;
    }
  }
  
  return false;
}

// Test a watch after having asked the system for the current
// time.
boolean TinyREST::testWatch(vwatch_t *w)
{
  return testWatch(w, millis());
}

#endif

