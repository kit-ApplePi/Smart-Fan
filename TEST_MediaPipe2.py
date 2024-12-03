from picamera2 import Picamera2
import cv2
import numpy as np
import time

# Initialize Picamera2
picam2 = Picamera2()

# Match libcamera-hello resolution and format
camera_config = picam2.create_preview_configuration(main={"size": (1640, 1232), "format": "RGB888"})
picam2.configure(camera_config)

# Start the camera
picam2.start()

prev_time = 0

try:
    while True:
        # Capture a frame from the camera
        frame = picam2.capture_array()  # RGB format

        # Display raw camera input for debugging
        cv2.imshow("Raw Camera Input", cv2.cvtColor(frame, cv2.COLOR_RGB2BGR))

        # FPS calculation for debugging
        curr_time = time.time()
        sec = curr_time - prev_time
        prev_time = curr_time
        fps = 1 / sec
        fps_str = "FPS : %0.1f" % fps

        # Display FPS on the image
        annotated_frame = frame.copy()
        cv2.putText(annotated_frame, fps_str, (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        # Show the annotated frame
        cv2.imshow("Annotated Frame", cv2.cvtColor(annotated_frame, cv2.COLOR_RGB2BGR))

        # Press 'Esc' to exit
        if cv2.waitKey(1) & 0xFF == 27:
            break

finally:
    cv2.destroyAllWindows()
    picam2.stop()
