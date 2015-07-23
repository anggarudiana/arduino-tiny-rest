# Introduction #

This project implements a REST-like API server on top of the Arduino [WiShield](http://asynclabs.com/wiki/index.php?title=AsyncLabsWiki#Documentation) library. In its basic instantiation, the library performs two major kinds of operations:

  * Reading and writing from the (A/D) pins of the Arduino and from the EEPROM.
  * Subscribing to value changes on these pins and EEPROM and arranging for remote HTTP servers to be notified of changes.

Apart from the basic set of commands, the library provides an API to add commands that would be specific to your Arduino project.


# Command Formatting Details #

Incoming commands are taken from the incoming HTTP GET requests and where the command and its possible arguments are separated by slashes. This implements a REST-like interface to the server. The first argument is the name of the command, and this name will be followed by a number of arguments, all of them being separated by slashes. All responses are [JSON-formatted](http://www.json.org/) for easy Web integration and parsing.

For example, the following GET request:

```
GET /dpin_read/3
```

Will read the content of the digital pin 3 on your Arduino (using [digitalRead()](http://arduino.cc/en/Reference/DigitalRead)) and will return its value as a JSON expression, for example, as follows:

```
{"result":1}
```


# Callback Mechanisms #

Because of internal limitations in the length of incoming HTTP requests in the WiShield library, subscribing to value (changes) via callbacks is a two-steps process. You will have first to describe a server which contains an IP address and a port number (no DNS allowed). Secondly, you will have to bind this server to a pin (or position in the EEPROM) and arrange for a path within that server to receive the value whenever it changes.  At each callback, the value will automatically be appended to the path.

Callbacks are also associated to a frequency at which a specific value is polled.  This is particularly useful for watching the content of the EEPROM since reading can be a lengthy operation.


# Shared Array #

In addition to being able to read and watch values at the A/D pins and the EEPROM of your arduino, the library also offers a data agnostic array to which you will be able to read, from which you will be able to write and which values you will be able to watch in the same manner. The idea is to use this array between federations of "servers" that are related to your project, a common simplistic data store.  This feature can easily be turned off.


# Default Commands #

## dpin\_mode ##

`dpin_mode` takes two arguments:
  * The identifier of the digital pin to change the mode for, e.g. 3.
  * The mode of operation of the pin, `0` will set the pin in `OUTPUT` mode, while any other number will set the pin in `INPUT` mode.

More information about the digital pins modes can be found in the [Arduino manual](http://arduino.cc/en/Tutorial/DigitalPins).

## dpin\_read ##

`dpin_read` takes one argument:
  * The identifier of the digital pin to read.

It returns a JSON expression with the current value of the pin.

## dpin\_write ##

`dpin_write` takes two arguments:
  * The identifier of the digital pin to write to.
  * The value to write to the pin, a positive integer will be understood as `HIGH` while `0` will be understood as `LOW`.

## apin\_read ##

`apin_read` takes one argument:
  * The identifier of the analogue pin to read.

It returns a JSON expression with the current value of the pin.

## apin\_write ##

`apin_write` takes two arguments:
  * The identifier of the analogue pin to write to.
  * The value to write to the pin.

## eeprom\_read ##

If called with a single argument, `eeprom_read` will be followed by the position at which you want to read in the EEPROM and the command will return a JSON expression with the current value.

If called with two arguments, `eeprom_read` will be followed by the start and stop position at which you want to read in the EEPROM and the command will return a JSON array with all the values between that range.

## eeprom\_write ##

`eeprom_write` takes two arguments:
  * The position within the EEPROM at which to write.
  * The value to write to the EEPROM.

## shared\_read ##

If called with a single argument, `shared_read` will be followed by the position at which you want to read in the shared array and the command will return a JSON expression with the current value.

If called with no arguments, `shared_read` will return a JSON array with all the content of the shared array.

## shared\_write ##

`shared_write` takes two arguments:
  * The position within the shared array at which to write.
  * The value to write to the shared array.

## server\_add ##

`server_add` takes three arguments:
  * An identifier (unsigned byte) for the server
  * The IPv4 address of the server
  * The port number of the server.

## server\_remove ##

`server_remove` takes as a single argument the identifier of a server that was declared with `server_add`.  There is at present no check within the subscription arrays for a server that you would be removing.

## subscribe ##

`subscribe` takes 4 or 5 arguments:
  * The type of a value that you want to watch. The recognised types are:
    * `0` to watch for changes onto a digital pin.
    * `1` to watch for changes onto an analogue pin.
    * `2` to watch for changes in the EEPROM.
    * `3` to watch for changes in the shared array.
  * The position of the pin or the position within the array/EEPROM as an integer
  * The identifier of the server.  This server **must** have been created with `server_add`.
  * An optional frequency, expressed in milliseconds, at which the current value should be read for deciding to deliver the callback or not.
  * An encoded path within the HTTP server at which changes will be delivered. The value will be added to that path whenever it has been detected to have changed.


## unsubscribe ##

`unsubscribe` removes a subscription and takes two arguments:
  * The type of a value that you want to watch. The recognised types are:
    * `0` to watch for changes onto a digital pin.
    * `1` to watch for changes onto an analogue pin.
    * `2` to watch for changes in the EEPROM.
    * `3` to watch for changes in the shared array.
  * The position of the pin or the position within the array/EEPROM as an integer

# Integrating with WiShield #

The library is meant to be easily integrated to the WiShield library, and particularly to the WiServer generic implementation that it contains.

You should start by creating an instance of the class at the beginning of your sketch, using a declaration similar to the following one:
```
TinyREST rest = TinyREST();
```

Then, in the `setup()` function, you should arrange to initiate the library by calling the `init()` method. This is exemplified further below:
```
rest.init();
```

In the page serving function of the WiServer, you should make sure that the incoming `URL` is passed further to the `handleURL()` method of the library, as in the example below:
```
boolean sendMyPage(char* URL) {
  return rest.handleURL(URL);
}
```

Finally, for the callbacks to be able to work properly, you should arrange to call the `loop()` method of the library from the `loop()` function of your sketch, as in the example below:
```
void loop(){
  // Run WiServer
  WiServer.server_task();
  rest.loop();
  
  delay(10);
}
```

# Turning on and off features #

You can turn on and off some of the features of the libraries by removing the constants `HAVE_SHARED` and `HAVE_SUBSCRIBE` from the header file.