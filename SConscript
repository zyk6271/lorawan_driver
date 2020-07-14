Import('RTT_ROOT')
from building import *

softse_src = Split("""
aes.c
cmac.c
soft-se.c
utilities.c
""")
src = Split("""
LoRaWAN-Func.c
LoRaWAN-Run.c
""")
mac_src = Split("""
RegionCommon.c
RegionCN470.c
Region.c
LoRaMacSerializer.c
LoRaMacParser.c
LoRaMacCrypto.c
LoRaMacConfirmQueue.c
LoRaMacCommands.c
LoRaMacClassB.c
LoRaMacAdr.c
LoRaMac.c
""")
boadrds_src = Split("""
NvmCtxMgmt.c
""")


# The set of source files associated with this SConscript file.
path = [GetCurrentDir() + '/softse',
    GetCurrentDir() + '/samples',
    GetCurrentDir() + '/mac',
    GetCurrentDir() + '/boards']


src = src + softse_src + mac_src + boadrds_src

group = DefineGroup('RTT_LoRaWAN_Driver', src, depend = ['PKG_USING_RTT_LoRaWAN_Driver'], CPPPATH = path)

Return('group')
