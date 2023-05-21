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

