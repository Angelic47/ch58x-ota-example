Import("env")
import os

# set the PROJECT_SRC_DIR to the entry point of the project
entry_path = os.path.join(env["PROJECT_DIR"], "extra_scripts/extra_components/bootloader/src")
env.Replace(PROJECT_SRC_DIR=entry_path)

# set the PROJECT_INCLUDE_DIR to the entry point of the project
include_path = os.path.join(env["PROJECT_DIR"], "extra_scripts/extra_components/bootloader/include")
env.Replace(PROJECT_INCLUDE_DIR=include_path)

# set the ldscript to the entry point of the project
# Custom LDSCRIPT_PATH is defined by forked version of ch32v platformio SCons script
# As there is no way to overwrite the LDSCRIPT_PATH in the original version script,
# see %platform-ch32v-dir%/builder/frameworks/noneos_sdk_autoota.py
#
# ldscript_custom = env.get("CUSTOM_LDSCRIPT_PATH", None)
#    if ldscript_custom:
#        return ldscript_custom
ldscript_path = os.path.join(env["PROJECT_DIR"], "extra_scripts/extra_components/bootloader/ldscript/link.ld")
env.Replace(CUSTOM_LDSCRIPT_PATH=ldscript_path)

# redefine the upload.maximum_size to 12K => 12288 bytes
board = env.BoardConfig()
board.update("upload.maximum_size", 12288)

# builtin startup files seems not to work with the OTA implementation,
# see extra_scripts/extra_components/bootloader/src/startup.S line 162
board.update("build.use_builtin_startup_file", "no")

# redefine the supported frameworks
board.update("frameworks", ["noneos-sdk-autoota"])
