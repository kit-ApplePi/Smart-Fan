from picamera2 import Picamera2
import numpy as np
import mediapipe as mp
import cv2
import time

# Initializing mediapipe holistic
mp_holistic = mp.solutions.holistic
mp_drawing = mp.solutions.drawing_utils
mp_drawing_styles = mp.solutions.drawing_styles

# Initialize Picamera2
picam2 = Picamera2()

# Camera configuration with consistent resolution
camera_config = picam2.create_preview_configuration(main={"size": (640, 480), "format": "RGB888"})
picam2.configure(camera_config)

# Start the camera
picam2.start()

# Video output settings
output_resolution = (640, 480)  # Ensure consistency in resolution
fourcc = cv2.VideoWriter_fourcc(*'DIVX')
out = cv2.VideoWriter('./holistic_result.avi', fourcc, 30, output_resolution)

prev_time = 0

# Process frames with MediaPipe Holistic
with mp_holistic.Holistic(
    static_image_mode=False, min_detection_confidence=0.5, model_complexity=2) as holistic:
    try:
        while True:
            # Capture a frame from the camera
            frame = picam2.capture_array()

            # Convert the RGB frame to BGR for OpenCV compatibility
            image = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

            # Process the frame with MediaPipe Holistic
            results = holistic.process(frame)

            # Draw landmarks on the frame
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
            curr_time = time.time()
            sec = curr_time - prev_time
            prev_time = curr_time
            fps = 1 / sec
            fps_str = "FPS : %0.1f" % fps

            # Display FPS on the image
            cv2.putText(annotated_image, fps_str, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

            # Write the frame to the output video
            out.write(annotated_image)

            # Display the annotated frame
            cv2.imshow('MediaPipe Holistic Result', annotated_image)

            # Press 'Esc' to exit
            if cv2.waitKey(1) & 0xFF == 27:
                break

    finally:
        # Release resources
        out.release()
        cv2.destroyAllWindows()
        picam2.stop()
