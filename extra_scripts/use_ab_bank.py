Import("env")

# redefine the upload.maximum_size to 216K => 221184 bytes
board = env.BoardConfig()
board.update("upload.maximum_size", 221184)

# redefine the supported frameworks
board.update("frameworks", ["noneos-sdk-autoota"])

# builtin startup files seems not to work with the OTA implementation,
# see src/startup.S line 162
board.update("build.use_builtin_startup_file", "no")

# LDSCRIPT_PATH is defined by forked version of ch32v platformio SCons script
# As there is no way to overwrite the LDSCRIPT_PATH in the original version script,
# see builder/frameworks/noneos_sdk_autoota.py
#
# return join(PROJECT_DIR, "extra_scripts", "ldscripts", "Link_" + board.get("build.series", "")[0:-1].upper() + "x") + "." + bank + ".ld"
