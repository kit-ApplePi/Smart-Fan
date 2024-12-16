#include "common.h"

void init_ManualControl() {
    pinMode(BUTTON_PIN, INPUT);     // 버튼 입력 핀
    pinMode(LED_PIN, OUTPUT);       // LED 출력 핀
    // pullUpDnControl(BUTTON_PIN, PUD_UP);  // 풀업 저항 활성화
}

int main(void) {
    // printf("수동 제어 시스템 시작\n");

    if (wiringPiSetupGpio() == -1) {
        perror("[manualControl] GPIO init failed");
        exit(1);
    }

    init_ManualControl();

    // 공유 메모리 초기화 대기
    delay(100);

    // 공유 메모리 연결
    int shmid = shmget(SHM_KEY, sizeof(SensorData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("[manualControl] shmget failed");
        exit(1);
    }

    SensorData* shmData = (SensorData*)shmat(shmid, NULL, 0);
    if (shmData == (SensorData*)-1) {
        perror("[manualControl] shmat failed");
        exit(1);
    }

    int prevButtonState = HIGH;

    while (1) {
        // 버튼 입력 상태 확인
        int buttonState = digitalRead(BUTTON_PIN);

        if (buttonState == LOW && prevButtonState == HIGH) {  // 버튼 눌림 감지
            pthread_mutex_lock(&shmData->mutex);

            if (shmData->power == 0) {
                shmData->power = 1;   // 시스템 전원 켜기
                digitalWrite(LED_PIN, HIGH);  // LED 켜기
                printf("[manualControl] Power On\n");
            } else {
                shmData->power = 0;   // 시스템 전원 끄기
                digitalWrite(LED_PIN, LOW);   // LED 끄기
                printf("[manualControl] Power Off\n");
            }

            pthread_mutex_unlock(&shmData->mutex);
            delay(100);  // 디바운싱 방지
        }

        prevButtonState = buttonState;
        delay(100);  // 상태 확인 주기
    }

    // 공유 메모리 분리
    shmdt(shmData);
    return 0;
}
