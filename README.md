# LoRaWAN_Driver软件包
	#** 简介**
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
	- 支持LoRaWAN协议版本
		- [x] V1.0.3
		- [ ] V1.1.0
	- 支持Region频段
		- [x] EU433
		- [x] CN470

	- 可通过menuconfig定义LoRaWAN的几项重要通信参数，降低入门门槛
	- 提供实例代码，可直接用于生产环境
	- 提供发送接受完成中断，进一步降低入门门槛
	- 控制台可直接进行数据发送测试以及数据接收测试
	- 具体使用文档请查看doc文件夹

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
