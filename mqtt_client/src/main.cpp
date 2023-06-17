/*! \file main.cpp
    \brief DPQNet300 mainboard progress.
    Copyright (c) 2016  Xi'an Boyuu Electric, Inc.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>

#include "config.h"
#include "parse_optn_mqttc.h"
#include "time_cst.h"
#include "thread_mng.h"
#include "socket_client.h"
#include "messageq_mqttc.h"
#include "mqttc_bufs.h"
#include "mqttc_para.h"
#include "mqtt_client.h"
#include "debug_info.h"
#include "test_pqm.h"

static int numthreads_; //线程数(不包括主线程)

void *ThrdTimer(void *myarg);
void *ThrdSocketMqttC(void *myarg);

SocketClient *g_sock_mqttc = NULL;
static const int CNNCT_MAX = 3; //最多建立3个socket连接
static MQTTClient *mqtt_client_=NULL;

int CreateThread()
{
    int x;
    CleanupNode *curnode;
    int retv = 0;

    pthread_attr_t attr;
    pthread_attr_init(&attr); //初始化属性值，均设为默认值
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM); //设为绑定的
    for (x = 0; x < 2; x++) {
        curnode = (CleanupNode*)malloc(sizeof(CleanupNode));
        if (!curnode)
            return 1;
        curnode->threadnum = x;
        switch (x) {
            case 0:
                retv = pthread_create(&curnode->tid,
                                      NULL, ThrdSocketMqttC, (void *)curnode);
                //&attr, ThrdSocketGui, (void *) curnode); // 设为与轻进程绑定
                break;
            case 1:
                retv = pthread_create(&curnode->tid,
                                      NULL, ThrdTimer, (void *)curnode);
                //&attr, ThrdTimer, (void *) curnode); // 设为与轻进程绑定
                break;
            default:
                numthreads_--;
                g_thread_cnt[x] = 0xfdddfddd;
                break;
        }
        if (retv) return 1;
        numthreads_++;
        msSleep(50);
    }
    return retv;
}

/*!
Initailize value
*/
void InitValues()
{
    printf("Initialize values...\n");
    numthreads_ = 0;
    memset(g_thread_cnt, 0, sizeof(g_thread_cnt));

    TimeCstInit();

    messageq_mqttc().InitQueue(CNNCT_MAX);
    mqttc_bufs().Initialize(CNNCT_MAX);
    g_sock_mqttc = new SocketClient(CNNCT_MAX);
    g_sock_mqttc->set_idle_tmout(300*60);  //300 minute
    mqtt_client_ = new MQTTClient(g_sock_mqttc);
}

void CleanupValues()
{
    printf("Cleanup values..\n");
    if (mqtt_client_) delete mqtt_client_;
    if (g_sock_mqttc) delete g_sock_mqttc;
    
    TimeCstEnd();
}

/*!
Initialize thread queues
*/
void InitThrdQs()
{
    printf("Initialize and active thread queues...\n");
    if (ControlInit(&g_clrq.control)) {
        dabort();
    }
    QueueInit(&g_clrq.task);

    if (ControlInit(&g_mainq.control)) {
        ControlDestroy(&g_clrq.control);
        dabort();
    }
    QueueInit(&g_mainq.task);
}

/*!
Cleanup thread queues
*/
void CleanupThrdQs()
{
    printf("Cleanup thread queues..\n");
    ControlDestroy(&g_clrq.control);
    ControlDestroy(&g_mainq.control);
}

void OnCreate()
{
    InitValues();
    InitThrdQs();
    if (CreateThread()) {
        printf("Error starting threads... cleaning up.\n");
        JoinThread(numthreads_);
        dabort();
    }
    printf("%s\n", NowTime(0));
}

void OnTerminal()
{
    printf("%s\n", NowTime(0));

    JoinThread(numthreads_);

    CleanupThrdQs();
    CleanupValues();

    printf("OVER ^Q^\n");
}

void HandlePtmr(int subtype)
{
    if (subtype&kOneSecond) {  //kOneSecond kFiveSecond
        mqtt_client_->KeepConnect();
    }
    if (subtype&kOneMinute) {
        mqtt_client_->Ping();
    }
}

static uint8_t retval_ = 0;
/*!
Handles the ^c
*/
void CtrlCFun (int i)
{
    //ControlDeactivate(&g_mainq.control);
    retval_ = 107;
    printf("\nprogram to be terminal... by ^c\n");
}

/*!
Handles the PIPE signal
*/
void PipeFun(int sig)
{
    //printf("sig=%d\n", sig);
}

/*!
Initialize signal handle function
*/
void IniSignal ()
{
    signal(SIGINT, CtrlCFun);  // Set the ^c catcher
    
    struct sigaction action;
    action.sa_handler = PipeFun;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGPIPE, &action, NULL);
}

int main(int argc, char *argv[])
{
    ParseOptnMqttC parse_opt;
    if (argc>1) {
        int ret = parse_opt.Parse(argc, argv);
        if (ret < 0) return ret;
        g_debugx = parse_opt.debug();
    }

    OnCreate();
    IniSignal();
    printf ("program is running, hit ^c to exit ... \n");
    
    for (int i=0; i<6 && mqtt_client_->AssociateMqttS(); i++) {
        msSleep(1000);
    }

    mqtt_client_->Connect();

    if (parse_opt.tstpqm()) {
        TestPQM(parse_opt.tstpqm(), mqtt_client_);
    } else {
        if (parse_opt.send(kMsgSubscribe)) {
            mqtt_client_->Subscribe();
        }

        if (parse_opt.send(kMsgPublish)) {
            const char *msg = parse_opt.msg();
            uint8_t flag = parse_opt.retain();
            flag |= parse_opt.qos()<<1;
            mqtt_client_->Publish(flag, parse_opt.topic(), (const uint8_t *)msg, strlen(msg));
        }
    }

    //mqtt_client_->Disconnect();
    WorkNode wnode;
    const int kTmOut = 100;   //unit:ms
    for (;;) {  //Clear the thread queue -- g_mainq
        PthreadCondWait(&wnode, &g_mainq, kTmOut);
        if (wnode.major_type == 42767) break;
    }
    while (retval_ != 107) {
        //break;
        g_thread_cnt[kMainProcessNum]++; //Increase this thread count
        if (PthreadCondWait(&wnode, &g_mainq, kTmOut)) break;

        switch (wnode.major_type) {
            case kPTimerInfo:
                HandlePtmr(wnode.minor_type);
                break;
            default:
                break;
        }
    }
    mqtt_client_->Disconnect();
    //msSleep(2000);
    ControlDeactivate(&g_mainq.control);
    OnTerminal();
    return retval_;
}

