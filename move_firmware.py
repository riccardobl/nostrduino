Import("env")

import shutil
import os

def after_build(source, target, env):
    sol=env.subst("$BUILD_DIR")
    platform="esp32"
    if "ESP32" in sol:
        platform="esp32"
    elif "ESP8266" in sol:
        platform="esp8266"
    elif "UNOR4" in sol:
        platform="unor4"
    elif "UNO" in sol:
        platform="uno"
    elif "PICOW" in sol:
        platform="picow"

    # Copy firmware binary
    firmware_src = env.subst("$BUILD_DIR/${PROGNAME}.bin")
    if os.path.exists(firmware_src):
        firmware_dest = ".pio/build/"+platform+"/firmware.bin"
        print("Copying firmware binary from: " + firmware_src + " to: " + firmware_dest)
        os.makedirs(os.path.dirname(firmware_dest), exist_ok=True)
        shutil.copy(firmware_src, firmware_dest)
    
    # Copy ELF file
    elf_src = env.subst("$BUILD_DIR/${PROGNAME}.elf")
    if os.path.exists(elf_src):
        elf_dest = ".pio/build/"+platform+"/firmware.elf"
        print("Copying ELF file from: " + elf_src + " to: " + elf_dest)
        os.makedirs(os.path.dirname(elf_dest), exist_ok=True)
        shutil.copy(elf_src, elf_dest)
    
    # Copy UF2
    uf2_src = env.subst("$BUILD_DIR/${PROGNAME}.uf2")
    if os.path.exists(uf2_src):
        uf2_dest = ".pio/build/"+platform+"/firmware.uf2"
        print("Copying UF2 file from: " + uf2_src + " to: " + uf2_dest)
        os.makedirs(os.path.dirname(uf2_dest), exist_ok=True)
        shutil.copy(uf2_src, uf2_dest)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)