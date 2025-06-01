Import("env")
import os

# set the PROJECT_SRC_DIR to the entry point of the project
entry_path = os.path.join(env["PROJECT_DIR"], "extra_scripts/extra_components/entryJumpToBL/src")
env.Replace(PROJECT_SRC_DIR=entry_path)

# set the PROJECT_INCLUDE_DIR to the entry point of the project
include_path = os.path.join(env["PROJECT_DIR"], "extra_scripts/extra_components/entryJumpToBL/include")
env.Replace(PROJECT_INCLUDE_DIR=include_path)

# set the ldscript to the entry point of the project
ldscript_path = os.path.join(env["PROJECT_DIR"], "extra_scripts/extra_components/entryJumpToBL/ldscript/link.ld")
env.Replace(LDSCRIPT_PATH=ldscript_path)

# redefine the upload.maximum_size to 4K => 4096 bytes
board = env.BoardConfig()
board.update("upload.maximum_size", 4096)
