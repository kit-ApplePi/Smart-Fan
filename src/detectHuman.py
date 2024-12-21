import cv2
import mediapipe as mp
import math
import posix_ipc
import struct
import time

# 메시지 큐 초기화
MQ_NAME = "/servo_angle_queue"
mq = posix_ipc.MessageQueue(MQ_NAME, flags=posix_ipc.O_CREAT | posix_ipc.O_RDWR | posix_ipc.O_NONBLOCK)


# MediaPipe 초기화
mp_pose = mp.solutions.pose

# 카메라 초기화
cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

if not cap.isOpened():
    print("카메라를 열 수 없습니다.")
    exit()

print("사람과 화면 중심 각도 계산 및 전송 중... (종료: 'q' 키)")
last_time = time.time()

# MediaPipe Pose 모델 시작
with mp_pose.Pose(static_image_mode=False, min_detection_confidence=0.5, min_tracking_confidence=0.5) as pose:
    while cap.isOpened():
        ret, frame = cap.read()
        if not ret:
            print("프레임 읽기 실패")
            break

        frame_height, frame_width = frame.shape[:2]
        screen_center_x = frame_width // 2

        # BGR → RGB 변환
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # MediaPipe 실행
        results = pose.process(rgb_frame)

        if results.pose_landmarks:
            # X좌표 평균 계산
            x_coords = [lm.x * frame_width for lm in results.pose_landmarks.landmark]
            person_center_x = int(sum(x_coords) / len(x_coords))

            # 화면 중심과 사람 중심의 각도 계산
            dx = person_center_x - screen_center_x
            angle = math.degrees(math.atan2(dx, frame_height))

            # 메시지 큐로 각도 값 전송
            # angle_data = struct.pack("f", angle)]
            
            if (time.time() - last_time) >= 0.2:
                last_time = time.time()
                
                try:
                    mq.send(f"{angle:3.2f}")
                    print(f"[detectHuman] 전송된 각도: {angle:.2f}도")
                except posix_ipc.BusyError:
                    print("[detectHuman] 메시지 큐 가득 참: 전송 실패")

            # 시각적 표시
            cv2.line(frame, (screen_center_x, 0), (screen_center_x, frame_height), (255, 0, 0), 2)
            cv2.circle(frame, (person_center_x, frame_height // 2), 10, (0, 255, 0), -1)
            cv2.putText(frame, f"Angle: {angle:.2f} deg", (50, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

            # 터미널 출력
            print(f"중심 좌표 X={person_center_x}, 화면 중심 X={screen_center_x}, 각도={angle:.2f}도")

        # 결과 표시
        cv2.imshow("Angle Detection", frame)

        # 종료 조건: 'q' 키 입력
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

# 자원 해제
mq.close()
mq.unlink()
cap.release()
cv2.destroyAllWindows()
