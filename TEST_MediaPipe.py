import cv2
import numpy as np
import mediapipe as mp
import time

# Initializing mediapipe pose class.
mp_holistic = mp.solutions.holistic

# Import drawing_utils and drawing_styles.
mp_drawing = mp.solutions.drawing_utils 
mp_drawing_styles = mp.solutions.drawing_styles

# Open the Raspberry Pi camera using the appropriate device
cap = cv2.VideoCapture(0)  # Try 0, or 1, or '/dev/video0' if '0' does not work
if not cap.isOpened():
    print("Cannot open camera")
    exit()

# Set the resolution for the camera
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)  # Set width
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)  # Set height
cap.set(cv2.CAP_PROP_FPS, 30)  # Set FPS

# Get the width, height, and FPS
w = round(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
h = round(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
fps = cap.get(cv2.CAP_PROP_FPS)

# Video output settings
fourcc = cv2.VideoWriter_fourcc(*'DIVX')
out = cv2.VideoWriter('./holistic_result.avi', fourcc, fps, (w, h))

prev_time = 0

# Run MediaPipe Holistic and draw pose landmarks.
with mp_holistic.Holistic(
    static_image_mode=False, min_detection_confidence=0.5, model_complexity=2) as holistic:
    while cap.isOpened():
        success, image = cap.read()
        if not success:
            print("Failed to capture image.")
            break
        
        curr_time = time.time()

        # Convert the BGR image to RGB and process it with MediaPipe Pose.
        results = holistic.process(cv2.cvtColor(image, cv2.COLOR_BGR2RGB))

        # Draw pose landmarks.
        annotated_image = image.copy()
        if results.left_hand_landmarks:
            mp_drawing.draw_landmarks(annotated_image, results.left_hand_landmarks, mp_holistic.HAND_CONNECTIONS)
        if results.right_hand_landmarks:
            mp_drawing.draw_landmarks(annotated_image, results.right_hand_landmarks, mp_holistic.HAND_CONNECTIONS)
        if results.face_landmarks:
            mp_drawing.draw_landmarks(
                annotated_image,
                results.face_landmarks,
                mp_holistic.FACEMESH_TESSELATION,
                landmark_drawing_spec=None,
                connection_drawing_spec=mp_drawing_styles.get_default_face_mesh_tesselation_style())
        if results.pose_landmarks:
            mp_drawing.draw_landmarks(
                annotated_image,
                results.pose_landmarks,
                mp_holistic.POSE_CONNECTIONS,
                landmark_drawing_spec=mp_drawing_styles.get_default_pose_landmarks_style())

        # Calculate FPS
        sec = curr_time - prev_time
        prev_time = curr_time
        fps = 1 / sec
        fps_str = "FPS : %0.1f" % fps

        # Display FPS on the image
        cv2.putText(annotated_image, fps_str, (0, 100), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0))

        # Save the frame with annotations
        out.write(annotated_image)

        # Display the frame
        cv2.imshow('MediaPipe Pose Result', annotated_image)

        # Press 'Esc' to exit the loop
        if cv2.waitKey(1) & 0xFF == 27:
            break

# Release resources
cap.release()
out.release()
cv2.destroyAllWindows()
