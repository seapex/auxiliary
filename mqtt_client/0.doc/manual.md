**Programer manual of MQTT Client**

## key classes
### main.cpp
#### InitValues()
```cpp
messageq_mqttc().InitQueue(CNNCT_MAX);
    `--> MessageQueue::InitQueue(CNNCT_MAX)
data_bufs().Initialize(CNNCT_MAX);
g_sock_client = new SocketClient(CNNCT_MAX);
mqtt_client_ = new MQTTClient(g_sock_client);
```
#### MQTTClient(SocketClient *sock_clnt)
```cpp
ass_idx_ = sock_clnt_->RegistIdx();
data_bufs().new_buf(ass_idx_);
sock_clnt_->Start(, , , , ass_idx_+1);
    |--> Connect(, , ass_idx_+1);
    |--> CreateCommuObj(, ass_idx_+1);
    `--> AppPrtclMqttC::SetCoIdx(ass_idx_);
              |--> MessageQMqttC::BindQueue(ass_idx_);
              `--> data_bufs().set_status(1, ass_idx_);
              co_idx_ = ass_idx_;
data_buf_ = data_bufs().buf(ass_idx_);
```
## mqtt_client.conf
```bash
HostName=192.168.1.3
PortName=1883
ClientID=PQSCADA50_001
DeviceID=PQNet300D000003
ConnectFlags=00  # hexadecimal number
Protocol level=5
KeepAlive(s)=120
SubTopics=seapex/test1/#,05;seapex/test2/#,05;,;,;,;    # topic,option(hex); 
Disable=0
```

## usage
```bash
MAIN_PROG [option]
    -h, --help      Print help information
    -V, --version   Print version information
    -dn --debug=n   Show debug information
    -r              RETAIN=1
    -q n            QoS. n=0,1(default)
    --pub           send PUBLISH
    --sub           send SUBSCRIBE
    -t topic        PUBLISH topic
    -m message      PUBLISH message
    --tstpqm n      test pqm function. n is func code\n"
                    1 clear new energy record.\n"
                    2 get new energy record status.\n"
                    3 get new energy record.\n"
```
