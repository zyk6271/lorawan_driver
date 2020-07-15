Import('RTT_ROOT')
from building import *

src   = []
cwd   = GetCurrentDir()

src += Split("""
softse/aes.c
softse/cmac.c
softse/soft-se.c
softse/utilities.c
""")
src += Split("""
samples/sample.c
""")
src += Split("""
mac/RegionCommon.c
mac/RegionCN470.c
mac/Region.c
mac/LoRaMacSerializer.c
mac/LoRaMacParser.c
mac/LoRaMacCrypto.c
mac/LoRaMacConfirmQueue.c
mac/LoRaMacCommands.c
mac/LoRaMacClassB.c
mac/LoRaMacAdr.c
mac/LoRaMac.c
mac/LoRaMacFunc.c
""")
src += Split("""
boards/NvmCtxMgmt.c
""")


# The set of source files associated with this SConscript file.
path = [cwd + '/softse',
    cwd + '/samples',
    cwd + '/mac',
    cwd + '/boards']


#src = src + softse_src + mac_src + boards_src

group = DefineGroup('RTT_LoRaWAN_Driver', src, depend = ['PKG_USING_RTT_LORAWAN_DRIVER'], CPPPATH = path)

Return('group')
