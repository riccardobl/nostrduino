# Nostrduino

A [nostr-tools](https://github.com/nbd-wtf/nostr-tools) inspired library for Arduino (and others).

This was created using [arduino-nostr](https://github.com/lnbits/arduino-nostr) as starting point, most of the code was rewritten and the library was expanded to include more features.

## Compatibility 
This library is compatible with esp32 based boards out of the box.
But it can be ported to other boards by reimplementing the platform specific code in the `src/esp32`.

## Features

+ [Nip01](https://github.com/nostr-protocol/nips/blob/master/01.md) : Basic protocol flow
+ [Nip04](https://github.com/nostr-protocol/nips/blob/master/04.md) : Encrypted messages
+ [Nip47](https://github.com/nostr-protocol/nips/blob/master/47.md) : Nostr Wallet Connect
+ [(bonus) Lightning Address](https://lightningaddress.com/) : Can request invoices from lightning addresses
+ [NWC managed service](src/services/NWC.h) : A managed service to interact with NWC 


## Philosophy
This library is built with a DIY philosophy in mind, similar to nostr-tools, this library provides helpers to create, sign and verify messages and to use some of the NIPs features (see the `src/Nip*` files).

In addition to that, the library provides some "managed" services (in `src/services` folder) that you can use in your sketches without worrying about the details and complexity of the nostr protocol.

## Installation

Check the [platformio registry page](https://registry.platformio.org/libraries/rblb/Nostrduino/installation) for instructions.

### ESP32 dependencies
This library depends on the following additional libraries for the esp32 platform:
- ArduinoJson (>=7.1.0)
- uBitcoin (>=0.2.0)
- WebSockets (>=2.4.1)


## Usage
See the example scripts in `examples/` for simple examples of how to use this library.


## Development
### Setting up the build environment

```bash
# create venv
python3 -m venv venv
# activate venv
source venv/bin/activate
# install platformio
pip install platformio
```



### Compiling Tests

#### Test NIP01
Edit `examples/ESP32TestNip01/ESP32TestNip01.cpp` and set the `WIFI_SSID`,  `WIFI_PASS` and `WIFI_CHANNEL`.

Then compile with:
```
pio run -e ESP32TestNip01
```


### Test NIP04

Compile with:
```
pio run -e ESP32TestNip04
```


### Test NWC
Edit `examples/ESP32TestNWC/ESP32TestNWC.cpp` and set the `WIFI_SSID`,  `WIFI_PASS` , `WIFI_CHANNEL` and `NWC_URL`.
NB: you can get a nwc url from [https://nwc.getalby.com/](https://nwc.getalby.com/) or [https://app.mutinywallet.com/settings/connections](https://app.mutinywallet.com/settings/connections).

Then compile with:
```
pio run -e ESP32TestNWC
```

### Running tests
You can upload the compiled tests to your ESP32 with:
```
pio run -e TEST_NAME -t upload
```

or use an emulator like [wokwi](https://wokwi.com/).
