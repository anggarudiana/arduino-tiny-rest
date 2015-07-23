This project implements a REST-like API server on top of the Arduino [WiShield](http://asynclabs.com/wiki/index.php?title=AsyncLabsWiki#Documentation) library. In its basic instantiation, the library performs two major kinds of operations:

  * Reading and writing from the (A/D) pins of the Arduino and from the EEPROM.
  * Subscribing to value changes on these pins and EEPROM and arranging for remote HTTP servers to be notified of changes.

The library is a far cousin of the [JSON](http://code.google.com/p/arduino-json/) library. The code has been so heavily modified, amended and with enough additions that I felt that it deserved a project on its own.

The library hooks into the [WiServer](http://asynclabs.com/wiki/index.php?title=WiServer) implementation and implements a number of commands to read, write and subscribe to value changes. In incoming commands, mediated as HTTP GETs, arguments are separated by slashes. Answers are JSON formatted.

So, for example, and if your arduino was running at IP address 192.168.0.156, the following incoming request will ask for the current value of the digital pin 3:

```
http://192.168.0.156/dpin_read/3
```

You would receive back something similar to the following:

```
{"result":1}
```

When called with no command at all, the server will respond a JSON description of its current state, similar to the following one:

```
{
"commands":[
{"command":"shared_read","arguments":0},
{"command":"shared_read","arguments":1},
{"command":"shared_write","arguments":2},
{"command":"eeprom_read","arguments":2},
{"command":"eeprom_read","arguments":1},
{"command":"eeprom_write","arguments":2},
{"command":"dpin_read","arguments":1},
{"command":"dpin_write","arguments":2},
{"command":"dpin_mode","arguments":2},
{"command":"apin_read","arguments":1},
{"command":"subscribe","arguments":5},
{"command":"subscribe","arguments":4},
{"command":"unsubscribe","arguments":2},
{"command":"server_add","arguments":3},
{"command":"server_remove","arguments":1}
],
"subscriptions":[
{"type":"dpin","position":3,"frequency":0,"path":"callback/dpin.php?pos=3&val=","value":0}
],
"servers":[
{"id":0,"ip":"192.168.0.39","port":8080}
]
}
```

Read the lengthier [manual](http://code.google.com/p/arduino-tiny-rest/wiki/Manual) for more information about the commands that are implemented in the basic instantiation and how to add your own commands to the set of commands that already exist.


---


This project is coming to you thanks to the [Me3GAS EU project](http://www.me3gas.eu/).