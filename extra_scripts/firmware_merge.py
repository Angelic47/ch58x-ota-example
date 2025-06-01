Import("env")
import os

buildDir = env["PROJECT_BUILD_DIR"] 

entryDir = os.path.join(buildDir, "buildEntry")
partitionADir = os.path.join(buildDir, "buildPartitionA")
partitionBDir = os.path.join(buildDir, "buildPartitionB")
bootloaderDir = os.path.join(buildDir, "buildBootloader")

entryFirmware = os.path.join(entryDir, "firmware.bin")
partitionAFirmware = os.path.join(partitionADir, "firmware.bin")
partitionBFirmware = os.path.join(partitionBDir, "firmware.bin")
bootloaderFirmware = os.path.join(bootloaderDir, "firmware.bin")

outputDir = os.path.join(buildDir, "mergedFirmware")

layout = [
    ("entry", entryFirmware, 0x00000000, 4 * 1024),
    ("partitionA", partitionAFirmware, 0x00001000, 216 * 1024),
    ("partitionB", partitionBFirmware, 0x00037000, 216 * 1024),
    ("bootloader", bootloaderFirmware, 0x0006D000, 12 * 1024)
]

fill_byte = 0xFF

# Create the output directory if it does not exist
if not os.path.exists(outputDir):
    os.makedirs(outputDir)
outputFirmwarePath = os.path.join(outputDir, "firmware.bin")

mergedFirmware = bytearray()

for name, firmware_path, offset, size in layout:
    if not os.path.exists(firmware_path):
        print(f"Error: Firmware file for {name} does not exist at {firmware_path}")
        exit(-1)
    actual_size = os.path.getsize(firmware_path)
    if actual_size > size:
        print(f"Error: Firmware file for {name} exceeds expected size {size} bytes at {firmware_path}")
        exit(-1)
    with open(firmware_path, "rb") as f:
        firmware_data = f.read()
        mergedFirmware.extend(firmware_data)
        # Fill the remaining space with fill_byte
        # No need for padding for bootloader because it's the last section
        padding_size = size - actual_size
        if padding_size > 0 and name != "bootloader":
            mergedFirmware.extend(bytearray([fill_byte] * padding_size))

with open(outputFirmwarePath, "wb") as output_file:
    output_file.write(mergedFirmware)
print(f"Merged firmware created at {outputFirmwarePath}")