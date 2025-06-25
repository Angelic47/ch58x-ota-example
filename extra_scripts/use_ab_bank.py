Import("env")

# redefine the upload.maximum_size to 216K => 221184 bytes
board = env.BoardConfig()
board.update("upload.maximum_size", 221184)

# redefine the supported frameworks
board.update("frameworks", ["noneos-sdk-autoota"])

# builtin startup files seems not to work with the OTA implementation,
# see src/startup.S line 162
board.update("build.use_builtin_startup_file", "no")

# get the partition bank from the env
bank = env.GetProjectOption("partition_bank", None)
if bank is None:
    raise ValueError("AutoOTA missing partition_bank option. Please set it in your platformio.ini file, e.g. `partition_bank = A` or `partition_bank = B`.")
bank = str(bank).lower()
if bank not in ("a", "b", "bootloader"):
    raise ValueError("AutoOTA partition_bank must be either 'A' or 'B' or 'Bootloader', got: %s" % bank)
elif bank == 'a':
    env.Append(CFLAGS=["-DLIBOTA_BUILD_CURRENT_BANK=0"])
elif bank == 'b':
    env.Append(CFLAGS=["-DLIBOTA_BUILD_CURRENT_BANK=1"])
elif bank == 'bootloader':
    env.Append(CFLAGS=["-DLIBOTA_BUILD_CURRENT_BANK=2"])

# LDSCRIPT_PATH is defined by forked version of ch32v platformio SCons script
# As there is no way to overwrite the LDSCRIPT_PATH in the original version script,
# see %platform-ch32v-dir%/builder/frameworks/noneos_sdk_autoota.py
#
# return join(PROJECT_DIR, "extra_scripts", "ldscripts", "Link_" + board.get("build.series", "")[0:-1].upper() + "x") + "." + bank + ".ld"
