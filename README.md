# Nostrduino

A [nostr-tools](https://github.com/nbd-wtf/nostr-tools) inspired Nostr library for Arduino ESP32.

This was created using [arduino-nostr](https://github.com/lnbits/arduino-nostr) as starting point, most of the code was rewritten and the library was expanded to include more features.

## Compatibility 
This library supports ESP32 boards using the Arduino framework.

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
This library depends on the following libraries for the ESP32 platform:
- ArduinoJson (>=7.1.0)
- WebSockets (>=2.4.1)

uBitcoin is the only elliptic-curve backend. Version 0.2.0 is pinned as the
`src/uBitcoin` git submodule.

When building from a git checkout, initialize the pinned source with:

```bash
git submodule update --init src/uBitcoin
```

Registry packages already contain the required submodule sources.

### ESP32 entropy modes

`ESP32Platform::initNostr(bool withLogger, bool initializeRngWithoutRadio = false)`

when initializing the library, leave the second argument at `false` when the
application uses or will start Wi-Fi or Bluetooth.

Firmware that does not use and will not start Wi-Fi or Bluetooth must enable the
explicit offline mode:

```cpp
nostr::esp32::ESP32Platform::initNostr(true, true);
```

With this option, Nostrduino calls `bootloader_random_enable()` before the
normal initialization so `esp_fill_random` has the offline entropy source
needed by cryptographic operations. 

**This is a radio-free/offline mode only, when using Wi-Fi or Bluetooth the entropy source is provided by the ESP32 radio and the second argument must be `false`.**

Do not start Wi-Fi, Bluetooth, ADC or I2S after enabling it without first
calling `bootloader_random_disable()` and handling the transition required by
the installed ESP-IDF; that lifecycle remains the application's responsibility.

### TLS configuration

The default ESP32 transport accepts TLS connections without certificate
validation.
For certificate validation, provide a PEM CA and require it:

```cpp
nostr::esp32::ESP32TransportConfig config;
config.tls.caCertificatePem = MY_ROOT_CA_PEM;
config.tls.requireCertificateValidation = true;
nostr::Transport *transport = nostr::esp32::ESP32Platform::getTransport(config);
```


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
pip install platformio==6.1.18
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
