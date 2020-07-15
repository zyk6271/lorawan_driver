# LoRaWAN_Driver软件包

# 1.0  简介
	  LoRaWAN_Driver软件包是基于RTOS( RT-Thread ) 实现的接入LoRaWAN网络的驱动包，基于lora-radio驱动包可用于快速搭建基于LoRaWAN网络等通信的应用产品。

	- 主要特点:
	- 支持工作模式
		- [x] Class A
		- [ ] Class B
		- [x] Class C
	- 支持入网方式
		- [x] ABP
		- [x] OTAA
	- 支持发送方式
		- [x] Unconfirmed DataUp
		- [x] Confirmed DataUp
	- 支持接收方式
		- [x] Unconfirmed DataDown
		- [x] Confirmed DataDown
	- 支持Region频段
		- [x] EU433
		- [x] CN470

	- 可通过menuconfig定义LoRaWAN的几项重要通信参数，降低入门门槛
	- 提供实例代码，可直接用于生产环境
	- 提供发送接受完成中断，进一步降低入门门槛
	- 控制台可直接进行数据发送测试以及数据接收测试

	- 当前测试LoRa 模块\芯片:
	- LoRa Transceiver (SPI):
		- SX126X (SX1262\ASR6500S\LLCC68\SX1268..)
			- [ ] [LSD4RF-2R717N40 ( SX1268 )]
			- [x] [ASR6500S ( SX1262 )]
		- SX127X (SX1272\SX1276\SX1278..)
			- [ ] [LSD4RF-2F717N20 ( SX1278 )]
			- [ ] [Ra-01 ( SX1278 )]

	- 当前测试LoRaSever平台:
			- [x] [Chirpstack]
			- [ ] [Ali Cloud]
			- [ ] [Tencent Cloud]
			- [ ] [The Things Network]

	- 支持LoRaWAN协议版本
			- [x] V1.0.3
			- [ ] V1.1.0


## 1.1 LoRaWAN_Driver 软件包组织结构
	- boards
		- \NvmCtxMgmt.c
			- 非易失性存储器相关
	- mac
		- \LoRaMac.c
			- LoRaWAN协议栈
		- \LoRaMacAdr.c
			- Adr速率自适应相关
		- \LoRaMacClassB.c
			- Class B模式相关
		- \LoRaMacCommands.c
			- LoRaWAN协议栈自带网络命令相关
		- \LoRaMacConfirmQueue.c
			- LoRaWAN数据确认队列
		- \LoRaMacCrypto.c
			- LoRaWAN加密相关
		- \LoRaMacParser.c
			- LoRaWAN数据解析相关
		- \LoRaMacSerializer.c
			- LoRaWAN数据序列化相关
		- \Region.c
			- 总频段集合
		- \RegionCN470.c
			- CN470-510频段相关
		- \RegionCommon.c
			- 频段共用函数相关
		- \LoRaMacFunc.c
			- 各类回调函数入口
		- \LoRaMacConfig.h
			- 网络参数
	- samples
		- \sample.c
			- LoRaWAN初始化及主状态机例程
	- softse
		- \ase.c
			- ase加密
		- \cmac.c
			- cmac加密
		- \soft-se.c
			- 加密函数
		- \utilities.c
			- 相关工具

# 2 LoRaWAN_Driver软件包使用说明

## 2.1 依赖

	- 依赖
	  -  lora-radio-driver 软件包
		- 用于提供RF驱动
		- 用于提供timerserver

## 2.2 获取软件包
使用 LoRaWAN_Driver软件包，需要在 RT-Thread 的包管理中选中它，具体路径如下：<br />

```
RT-Thread online packages --->
    iot --->
        [*] LoRaWAN_Driver: Semtech LoRaMac driver. --->
                Version (latest)  --->
```

	使用须知：
	1. 确认配置好radio-driver的情况下，通过radio的主从机测试，方可使用本软件包
	2. 根据实际情况，进行网络参数的配置
	3. 在非必要的情况下请勿对频段参数(RegionCN470.h)进行修改
	4. radio包的example与当前例程，两者频段定义有冲突，只可存在一个频段定义
	5. 进行初始化后方可进行数据收发


## 2.3 参数修改介绍
	- \mac\LoRaMacConfig.h
		- ACTIVE_REGION
			- 频段选择
		- APP_TX_DUTYCYCLE
			- 循环发送间隔时间
		- APP_TX_DUTYCYCLE_RND
			- 循环发送间隔时间随机值大小
		- LORAWAN_DEFAULT_DATARATE
			- 默认发送速率
		- LORAWAN_CONFIRMED_MSG_ON
			- 消息发送是否需要服务器下发确认
		- LORAWAN_ADR_ON
			- 速率自适应开关
		- LORAWAN_APP_DATA_MAX_SIZE
			- 单条上发数据的最大长度，具体参照频段规范
		- LORAWAN_APP_PORT
			- 端口号
		- OVER_THE_AIR_ACTIVATION
			- 入网方式选择，0为ABP，1为OTAA
		- DEVICE_CLASS
			- 工作模式选择，0为Class A,2为Class C
		- ABP_ACTIVATION_LRWAN_VERSION_V10x
			- 版本
		- ABP_ACTIVATION_LRWAN_VERSION
			- 版本
		- LORAWAN_PUBLIC_NETWORK
			- 是否为公开网络
		- IEEE_OUI
			- 自定ID
		- LORAWAN_DEVICE_EUI
			- OTAA入网ID
		- LORAWAN_JOIN_EUI
			- 对应TTN平台的Application EUI
		- LORAWAN_APP_KEY
			- OTAA入网密钥
		- LORAWAN_GEN_APP_KEY
			- OTAA入网密钥(保持与LORAWAN_APP_KEY同步)
		- LORAWAN_NWK_KEY
			- OTAA网络密钥
		- LORAWAN_NETWORK_ID
			- 网络ID
		- LORAWAN_DEVICE_ADDRESS
			- ABP设备地址
		- LORAWAN_F_NWK_S_INT_KEY
			- ABP网络密钥
		- LORAWAN_S_NWK_S_INT_KEY
			- ABP网络密钥(保持与上方密钥同步)
		- LORAWAN_NWK_S_ENC_KEY
			- ABP网络密钥(保持与上方密钥同步)
		- LORAWAN_APP_S_KEY
			- ABP应用程序密钥

------------

	\mac\RegionCN470.h
		- CN470_MAX_NB_CHANNELS
			- 频段最大频道数
		- CN470_TX_MIN_DATARATE
			- 频段最小发射速率
		- CN470_TX_MAX_DATARATE
			- 频段最大发射速率
		- CN470_RX_MIN_DATARATE
			- 频段最小接收速率
		- CN470_RX_MAX_DATARATE
			- 频段最大接收速率
		- CN470_DEFAULT_DATARATE
			- 频段默认接收速率
		- CN470_MIN_RX1_DR_OFFSET
			- 频段最小接收速率偏移
		- CN470_MAX_RX1_DR_OFFSET
			- 频段最大接收速率偏移
		- CN470_DEFAULT_RX1_DR_OFFSET
			- 频段默认接收速率偏移
		- CN470_MIN_TX_POWER
			- 频段最小发射功率
		- CN470_MAX_TX_POWER
			- 频段最大发射功率
		- CN470_DEFAULT_TX_POWER
			- 频段默认发射功率
		- CN470_DEFAULT_MAX_EIRP
			- 频段默认辐射功率
		- CN470_DEFAULT_ANTENNA_GAIN
			- 频段默认天线增益
		- CN470_ADR_ACK_LIMIT
			- 速率自适应最小ack次数
		- CN470_ADR_ACK_DELAY
			- 速率自适应ack延迟
		- CN470_DUTY_CYCLE_ENABLED
			- 发射占空比开关
		- CN470_MAX_RX_WINDOW
			- 频段最大窗口时间
		- CN470_RECEIVE_DELAY1
			- 频段窗口1延迟时间
		- CN470_RECEIVE_DELAY2
			- 频段窗口2延迟时间
		- CN470_JOIN_ACCEPT_DELAY1
			- 频段入网窗口1延迟时间
		- CN470_JOIN_ACCEPT_DELAY2
			- 频段入网窗口2延迟时间
		- CN470_MAX_FCNT_GAP
			- 最大允许下行丢包数
		- CN470_ACKTIMEOUT
			- ack超时时间
		- CN470_ACK_TIMEOUT_RND
			- ack超时时间随机数大小
		- CN470_RX_WND_2_FREQ
			- 窗口2频率
		- CN470_RX_WND_2_DR
			- 窗口2速率

------------


	\mac\RegionCN470.c
		- Line 377
			NvmCtx.ChannelsDefaultMask[0] = 0x00FF;
			NvmCtx.ChannelsDefaultMask[1] = 0x0;
			NvmCtx.ChannelsDefaultMask[2] = 0x0;
			NvmCtx.ChannelsDefaultMask[3] = 0x0;
			NvmCtx.ChannelsDefaultMask[4] = 0x0;
			NvmCtx.ChannelsDefaultMask[5] = 0x0;

		CN470频段分为96个频点，每8个频点对应一个BAND,如需修改设备所属的BAND，应对此进行修改，一共六组掩码，对应BAND1-BAND12.当前配置所显示的为BAND1,若NvmCtx.ChannelsDefaultMask[0]为0xFFFF则为BAND1,2同时选中，若NvmCtx.ChannelsDefaultMask[0]与NvmCtx.ChannelsDefaultMask[1]分别为0xFFFF，0xFF00则是选中BAND1,2,4，以此类推。
# 3 使用示例
## 3.1 测试平台
	当前MCU：STM32L431CBT6
	当前RF：ASR6500S(SX1262)
	当前服务器：LoRaServer(老版本的ChirpStack)
## 3.2 Finish测试命令
| finish命令 | 说明 |
|  --- | --- |
| lorawan init | lorawan初始化 |
| lorawan send <para1>| 数据发送 <para1>:要发送的数据|
| lorawan cycle <para1> | 循环发送测试 <para1>:时间（ms)（至少5秒）|
| lorawan restart   | 重新开始配置LoRaWAN,状态机置初始位 |
## 3.3 实际测试示例
图1：软件包命令help界面
![](https://s1.ax1x.com/2020/07/14/UUnwgx.png)
图2：ABP模式下初始化
![](https://s1.ax1x.com/2020/07/14/UUnaCR.png)
图3：OTAA模式下初始化
![](https://s1.ax1x.com/2020/07/14/UUn0v6.png)
图4：Class A模式下发送数据
![](https://s1.ax1x.com/2020/07/14/UUnN59.png)
图5：Class A模式下发送并接收数据
![](https://s1.ax1x.com/2020/07/14/UUnd81.png)
图6：Class C模式下发送数据
![](https://s1.ax1x.com/2020/07/14/UUntUJ.png)
图7：Class C模式下接收数据
![](https://s1.ax1x.com/2020/07/14/UUnYE4.png)

# 4 问题和建议

如果有什么问题或者建议欢迎提交 [Issue](https://github.com/zyk6271/LoRaWAN_Driver/issues) 进行讨论。
