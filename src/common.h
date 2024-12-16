#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>  
#include <stdlib.h>
#include <stdint.h>
#include <math.h>               // 수학 라이브러리 ( -lm )
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <wiringPi.h>           // wiringPi 라이브러리 사용 ( -lwiringPi )
#include <mqueue.h>             // MessageQueue

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>


#define DHT11_PIN 4            // DHT11 GPIO PIN

#define SERVO_PIN 18

#define DC_IN1_PIN 12           // DC Motor IN1 PIN
#define DC_IN2_PIN 13           // DC Motor IN2 PIN

#define BUTTON_PIN 22           // 버튼 입력 핀
#define LED_PIN 27              // 전원 표시 LED 핀


#define SHM_KEY 0x1234         // 공유 메모리 키

#define MQ_NAME "/servo_angle_queue"   // 메시지 큐 이름

// 공유 메모리에 저장할 구조체
typedef struct {
    int power;

    double humidity;
    double temperature;

    pthread_mutex_t mutex;
} SensorData;

#endif