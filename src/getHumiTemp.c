#include "common.h"

#define MAX_TIMINGS	85        			// 최대 신호 추출 개수

int data[5] = { 0, 0, 0, 0, 0 };       	// 온습도 및 checksum 데이터 저장용 변수 배열

void read_Humi_Temp_data(SensorData* shmData) {
	uint8_t laststate	= HIGH;         // DHT핀의 상태 저장용 변수(현재 신호가 HIGH인지 LOW인지 확인하기 위한 용도)
	uint8_t counter		= 0;            // 신호의 길이를 측정하기 위한 카운터 변수
	uint8_t j			= 0, i;         // 40개의 신호 인덱스 용 변수

	double humidity, temperature;     	//습도 및 온도 저장용 변수

	data[0] = data[1] = data[2] = data[3] = data[4] = 0;    //초기 데이터 값은 0으로 지정

	pinMode(DHT11_PIN, OUTPUT);
	digitalWrite(DHT11_PIN, LOW);
	delay( 18 );
	pinMode(DHT11_PIN, INPUT);

	for ( i = 0; i < MAX_TIMINGS; i++ ) {
		counter = 0;
		while ( digitalRead(DHT11_PIN) == laststate ) {
			counter++;
			delayMicroseconds( 1 );
			if ( counter == 255 ) {
				break;
			}
		}
		laststate = digitalRead(DHT11_PIN);

		if ( counter == 255 ) {
			break;
		}

		if ( (i >= 4) && (i % 2 == 0) ) {
			data[j / 8] <<= 1;
			if ( counter > 16 )
				data[j / 8] |= 1;
			j++;
		}
	}

	if ((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF))) {
		humidity = (float)data[0] + (float)data[1] / 100.0;
		temperature = (float)(data[2] & 0x7F) + (float)data[3] / 100.0;

		if (data[2] & 0x80) {
			temperature = -temperature;
		}

		pthread_mutex_lock(&shmData->mutex);
		shmData->humidity = humidity;
		shmData->temperature = temperature;
		pthread_mutex_unlock(&shmData->mutex);

        printf("[getHumiTemp] 습도: %.1f%%, 온도: %.1f°C \n", humidity, temperature);
	} else {
		printf("[getHumiTemp] 데이터 읽기 실패\n");
	}
}

int main( void ) {

	if ( wiringPiSetupGpio() == -1 )    //라즈베리파이의 BCM GPIO 핀번호를 사용하겠다고 선언
		exit( 1 );

	// 공유 메모리 연결
    int shmid = shmget(SHM_KEY, sizeof(SensorData), 0666);
    if (shmid < 0) {
        perror("[getHumiTemp] shmget failed");
        exit(1);
    }

    SensorData* shmData = (SensorData*)shmat(shmid, NULL, 0);
    if (shmData == (SensorData*)-1) {
        perror("[getHumiTemp] shmat failed");
        exit(1);
    }

	while ( 1 ) {
		// 뮤텍스 잠금 후 power 상태 확인
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

        read_Humi_Temp_data(shmData); // 온도 및 습도 데이터 획득
		delay( 1000 );                // 다음 읽기까지 1초 대기
	}

	// 공유 메모리 분리
    shmdt(shmData);

	return(0);
}