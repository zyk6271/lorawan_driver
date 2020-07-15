Import('RTT_ROOT')
from building import *

boards_src   = []
mac_src   = []
samples_src   = []
softse_src   = []
cwd   = GetCurrentDir()

softse_src = Split("""
softse/aes.c
softse/cmac.c
softse/soft-se.c
softse/utilities.c
""")
samples_src = Split("""
samples/sample.c
""")
mac_src = Split("""
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
boards_src = Split("""
boards/NvmCtxMgmt.c
""")


# The set of source files associated with this SConscript file.
path = [cwd + '/softse',
    cwd + '/samples',
    cwd + '/mac',
    cwd + '/boards']


#src = src + softse_src + mac_src + boards_src

group = DefineGroup('lorawan_driver/boards', boards_src, depend = ['PKG_USING_LORAWAN_DRIVER'], CPPPATH = path)
group = DefineGroup('lorawan_driver/mac', mac_src, depend = ['PKG_USING_LORAWAN_DRIVER'], CPPPATH = path)
group = DefineGroup('lorawan_driver/softse', softse_src, depend = ['PKG_USING_LORAWAN_DRIVER'], CPPPATH = path)
group = DefineGroup('lorawan_driver/samples', samples_src, depend = ['PKG_USING_LORAWAN_DRIVER'], CPPPATH = path)

Return('group')
