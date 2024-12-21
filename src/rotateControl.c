#include "common.h"

float prevAngle = 0.0;

// SERVO 초기화 및 회전 함수
void init_Servo();
void rotate_Servo(float angle);

// SERVO 초기화
void init_Servo() {
    pinMode(SERVO_PIN, PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);  // MS 모드 설정
    pwmSetRange(2000);        // PWM 범위 설정 (0.5ms~2.5ms)
    pwmSetClock(192);         // 50Hz 생성 (19.2MHz / 192 / 2000 = 50Hz)
}

// SERVO 회전
void rotate_Servo(float angle) {
    if (angle < -90 || angle > 90) return;  // 범위 제한

    int minPulse = 50;    // 0.5ms에 해당하는 PWM 값
    int maxPulse = 250;   // 2.5ms에 해당하는 PWM 값
    int pwmValue = (int)((angle + 90) * (maxPulse - minPulse) / 180 + minPulse);

    // printf("[ANGLE] : %6.2f ,[PULSE] : %3d\n", angle, pwmValue);
    pwmWrite(SERVO_PIN, pwmValue);
}

int main(void) {

    if (wiringPiSetupGpio() == -1) {
        perror("[rotateControl] GPIO 초기화 실패");
        exit(1);
    }

    init_Servo();

    struct mq_attr attr;
    
    // 메시지 큐 속성 설정
    attr.mq_flags = 0;
    attr.mq_maxmsg = 100;
    attr.mq_msgsize = 16;
    attr.mq_curmsgs = 0;
    

    // 메시지 큐 열기
    mqd_t mq = mq_open(MQ_NAME, O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("[rotateControl] mq_open 실패");
        exit(1);
    }

    // 공유 메모리 연결
    int shmid = shmget(SHM_KEY, sizeof(SensorData), 0666);
    if (shmid < 0) {
        perror("[rotateControl] shmget 실패");
        mq_close(mq);
        exit(1);
    }

    SensorData* shmData = (SensorData*)shmat(shmid, NULL, 0);
    if (shmData == (SensorData*)-1) {
        perror("[rotateControl] shmat 실패");
        mq_close(mq);
        exit(1);
    }

    float angle;
    float deltaAngle;
    char angle_str[16];  // 메시지 수신 버퍼

    while (1) {
         // 뮤텍스 잠금 후 데이터 읽기
        pthread_mutex_lock(&shmData->mutex);
        if (shmData->power == 0) {
            pthread_mutex_unlock(&shmData->mutex);
            // printf("Power off. Waiting...\n");
            sleep(1);

            continue;
        } else if (shmData->power < 0) {
            pthread_mutex_unlock(&shmData->mutex);
            sleep(1);
            break;
        }
        pthread_mutex_unlock(&shmData->mutex);

        // 메시지 수신 (문자열 형식)
        if (mq_receive(mq, angle_str, sizeof(angle_str), NULL) == -1) {
            perror("[rotateControl] mq_receive 실패");
            sleep(1);
            continue;
        }

        // 수신된 각도 변화값을 float로 변환
        deltaAngle = atof(angle_str);

        // 목표 각도 업데이트
        float targetAngle = prevAngle - deltaAngle;

        // 각도 제한 (서보 모터 안전 범위)
        if (targetAngle < -90.0) targetAngle = -90.0;
        if (targetAngle > 90.0) targetAngle = 90.0;

        // 각도 이동 루프 (1도씩 변화)
        while (fabs(prevAngle - targetAngle) >= 1.0) {
            if (prevAngle < targetAngle) {
                prevAngle += 1.0;  // 시계 방향 회전
            } else if (prevAngle > targetAngle) {
                prevAngle -= 1.0;  // 반시계 방향 회전
            }

            // 누적 각도 출력
            printf("[rotateControl] 현재 회전 각도: %.2f도\n", prevAngle);

            // 서보 회전 수행
            rotate_Servo(prevAngle);

            // 약간의 대기 (속도 조절)
            delay(10);  // 20ms 대기
        }

        // 목표 각도 출력
        printf("[rotateControl] 최종 각도: %.2f도\n", targetAngle);

        // delay(100);  // 0.1초 대기
    }


    // 메시지 큐 닫기
    mq_close(mq);
    mq_unlink(MQ_NAME);

    // 공유 메모리 분리
    shmdt(shmData);
    return 0;
}
