#include "common.h"

int main(void) {
    // 공유 메모리 생성
    int shmid = shmget(SHM_KEY, sizeof(SensorData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("[main] shmget failed");
        exit(1);
    }

    // 공유 메모리 연결
    SensorData* data = (SensorData*)shmat(shmid, NULL, 0);
    if (data == (SensorData*)-1) {
        perror("[main] shmat failed");
        exit(1);
    }

    // 뮤텍스 초기화
    if (pthread_mutex_init(&data->mutex, NULL) != 0) {
        perror("[main] pthread_mutex_init failed");
        exit(1);
    }

    // 데이터 초기화
    memset(data, 0, sizeof(SensorData));

    printf("[main] 공유 메모리 생성 및 초기화 완료. SHM ID: %d\n", shmid);
    
    // manualControl 실행
    printf("[main] Running manualControl...\n");
    if (fork() == 0) {
        execlp("./manualControl", "manualControl", NULL);
        perror("[main] exec manualControl failed");
        exit(1);
    }
    // getHumiTemp 실행
    printf("[main] Running getHumiTemp...\n");
    if (fork() == 0) {
        execlp("./getHumiTemp", "getHumiTemp", NULL);
        perror("[main] exec getHumiTemp failed");
        exit(1);
    }

    // fanControl 실행
    printf("[main] Running fanControl...\n");
    if (fork() == 0) { // 자식 프로세스 생성
        execlp("./fanControl", "fanControl", NULL);
        perror("[main] exec fanControl failed");
        exit(1);
    }

    // rotateControl 실행
    printf("[main] Running rotateControl...\n");
    if (fork() == 0) {
        execlp("./rotateControl", "rotateControl", NULL);
        perror("[main] exec rotateControl failed");
        exit(1);
    }


    int i = 0;
    while (1) {
        // 뮤텍스 잠금 후 데이터 읽기
        // pthread_mutex_lock(&data->mutex);
        // printf("[%3d] 현재 습도: %.1f%%, 현재 온도: %.1f°C\n", i++, data->humidity, data->temperature);
        // pthread_mutex_unlock(&data->mutex);

        sleep(2);  // 2초 대기
    }

    // 부모 프로세스는 자식 프로세스가 종료될 때까지 대기
    for (int i = 0; i < 4; i++) {
        wait(NULL);
    }

    printf("[main] All processes completed.\n");

    // 공유 메모리 분리 및 뮤텍스 삭제
    pthread_mutex_destroy(&data->mutex);
    shmdt(data);
    return 0;
}
