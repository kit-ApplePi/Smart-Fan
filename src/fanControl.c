#include "common.h"

void setup_motor() {
    pinMode(DC_IN1_PIN, PWM_OUTPUT);
    pinMode(DC_IN2_PIN, PWM_OUTPUT);

    pwmWrite(DC_IN1_PIN, 0);
    pwmWrite(DC_IN2_PIN, 0);
}

void motor_Rotate(int speed, int dir) {
    if (dir == 0) {
        pwmWrite(DC_IN2_PIN, 1024 * speed / 100);
        pwmWrite(DC_IN1_PIN, 0);
    }
    else {
        pwmWrite(DC_IN1_PIN, 1024 * speed / 100);
        pwmWrite(DC_IN2_PIN, 0);
    }
}

float calculate_fan_speed(float humidity, float temperature) {
    float fan_speed = 10.0;

    // 습도 기반 바람 세기 계산
    if (humidity > 90.0) {
        fan_speed = 100.0;   // 매우 높은 습도
    } else if (humidity > 80.0) {
        fan_speed = 90.0;    // 높은 습도
    } else if (humidity > 70.0) {
        fan_speed = 80.0;    // 다소 높은 습도
    } else if (humidity > 60.0) {
        fan_speed = 70.0;    // 약간 높은 습도
    } else if (humidity > 50.0) {
        fan_speed = 60.0;    // 적당한 습도
    } else if (humidity > 40.0) {
        fan_speed = 50.0;    // 낮은 습도
    } else if (humidity > 30.0) {
        fan_speed = 40.0;    // 다소 낮은 습도
    } else if (humidity > 20.0) {
        fan_speed = 30.0;    // 매우 낮은 습도
    } else {
        fan_speed = 20.0;    // 극도로 낮은 습도
    }

    // 온도 보정
    if (temperature <= 25.0) {
        fan_speed *= 0.7;  // 온도가 낮으면 70%로 줄이기
    } else if (temperature >= 30.0) {
        fan_speed *= 1.3;  // 온도가 높으면 130%로 늘리기
    }

    // 바람 세기 범위 확인
    if (fan_speed < 10.0) fan_speed = 10.0;
    if (fan_speed > 100.0) fan_speed = 100.0;

    return fan_speed;
}

int main(void) {

    // GPIO 초기화
    if (wiringPiSetupGpio() == -1) {
        perror("[fanControl] GPIO init failed");
        exit(1);
    }

    // 공유 메모리 연결
    int shmid = shmget(SHM_KEY, sizeof(SensorData), 0666);
    if (shmid < 0) {
        perror("[fanControl] shmget failed");
        exit(1);
    }

    SensorData* shmData = (SensorData*)shmat(shmid, NULL, 0);
    if (shmData == (SensorData*)-1) {
        perror("[fanControl] shmat failed");
        exit(1);
    }

    // DC 모터 초기화
    setup_motor();

    while (1) {
        // 뮤텍스 잠금 후 데이터 읽기
        pthread_mutex_lock(&shmData->mutex);
        if (shmData->power == 0) {
            pthread_mutex_unlock(&shmData->mutex);
            motor_Rotate(0, 0);  // 시계방향 회전
            // printf("Power off. Waiting...\n");
            sleep(1);

            continue;
        } else if (shmData->power < 0) {
            pthread_mutex_unlock(&shmData->mutex);
            sleep(1);
            break;
        }
        pthread_mutex_unlock(&shmData->mutex);
        
        float humidity = shmData->humidity;
        float temperature = shmData->temperature;
        pthread_mutex_unlock(&shmData->mutex);

        int fan_speed = calculate_fan_speed(humidity, temperature);
        printf("[fanControl] Fan Speed : %.1f%%\n", humidity, temperature, fan_speed);

        motor_Rotate(fan_speed, 0);  // 시계방향 회전

        sleep(2);  // 2초 대기
    }

    // 공유 메모리 분리
    shmdt(shmData);
    return 0;
}
