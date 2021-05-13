## PQNet1xx/2xxD 参数设置作业指导书
**修订历史记录**
A - 增加  M - 修订  D - 删除

| 版本号 | 日期       | 变更类型(A/M/D) | 修改人 | 摘   要  |
| ------ | ---------- | --------------- | ------ | -------- |
| 1.0    | 2021-04-23 | A               | seapex | 初始版本 |
|        |            |                 |        |          |
|        |            |                 |        |          |

#### 硬件准备

1. 给 PQNet300D 开发板上电，给**网口1**插上网线，并确认升级主机与开发板网络畅通
2. 给 PQNet1xx/2xxD 上电，并用一根网线连接 PQNetxx1/2xxD 与 300D开发板的**网口2**

<img src="Figure/PQNet300D_evaluation_board.jpg" style="zoom: 50%;" />

#### 软件准备

1. 启动 tftpd32
2. 点击 “Browse” 按钮，切换工作目录到 “E:\软件\设备软件\scnettool”
3. 运行 putty4pqnet_1_71.bat (E:\软件\设备软件\scnettool)

#### 参数设置

以下命令都在 上节第3步运行后弹出的命令行窗口执行.

##### 设置MAC

> root@ok335x:~# scnettool 0-0-1

参数为MAC地址低3组数值

##### 获取参数

> root@ok335x:~# scnettool get 0-0-1

命令运行成功后，参数会保存到文件 E:\软件\设备软件\scnettool\0-0-1.cfg

##### 设置参数

打开上一步保存的参数文件，修改相应的参数，保存后，执行以下命令

> root@ok335x:~# scnettool set 0-0-1