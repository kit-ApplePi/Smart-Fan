# 컴파일러 및 플래그 설정
CC = gcc
CFLAGS = -Wall -lwiringPi -lpthread -lrt -lm

# 실행 파일 목록
TARGETS = main fanControl getHumiTemp manualControl rotateControl

# 소스 및 헤더 파일
SRC_DIR = .
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = common.h

# 개별 실행 파일 정의
all: $(TARGETS)

main: main.c $(HEADERS)
	$(CC) -o main main.c $(CFLAGS)

fanControl: fanControl.c $(HEADERS)
	$(CC) -o fanControl fanControl.c $(CFLAGS)

getHumiTemp: getHumiTemp.c $(HEADERS)
	$(CC) -o getHumiTemp getHumiTemp.c $(CFLAGS)

manualControl: manualControl.c $(HEADERS)
	$(CC) -o manualControl manualControl.c $(CFLAGS)

rotateControl: rotateControl.c $(HEADERS)
	$(CC) -o rotateControl rotateControl.c $(CFLAGS)

# 깨끗하게 빌드 파일 삭제
clean:
	rm -f $(TARGETS) *.o

.PHONY: all cle
