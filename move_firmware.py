Import("env")

import shutil
import os

def after_build(source, target, env):
    # Copy firmware binary
    firmware_src = env.subst("$BUILD_DIR/${PROGNAME}.bin")
    platform="esp32"
    if "ESP32" in firmware_src:
        platform="esp32"
    elif "UNOR4" in firmware_src:
        platform="unor4"
    elif "UNO" in firmware_src:
        platform="uno"

    firmware_dest = ".pio/build/"+platform+"/firmware.bin"
    print("Copying firmware binary from: " + firmware_src + " to: " + firmware_dest)
    os.makedirs(os.path.dirname(firmware_dest), exist_ok=True)
    shutil.copy(firmware_src, firmware_dest)
    
    # Copy ELF file
    elf_src = env.subst("$BUILD_DIR/${PROGNAME}.elf")
    elf_dest = ".pio/build/"+platform+"/firmware.elf"
    print("Copying ELF file from: " + elf_src + " to: " + elf_dest)
    os.makedirs(os.path.dirname(elf_dest), exist_ok=True)
    shutil.copy(elf_src, elf_dest)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", after_build)