from picamera2 import Picamera2, Preview
import cv2
import time

# Initialize Picamera2
picam2 = Picamera2()

# Configure camera for libcamera-hello resolution and format
camera_config = picam2.create_preview_configuration(
    main={"size": (1640, 1232), "format": "RGB888"}, 
    transform={"hflip": False, "vflip": False}
)
picam2.configure(camera_config)

# Start the camera
picam2.start()

# FPS Calculation
prev_time = 0

try:
    while True:
        # Capture frame from the camera
        frame = picam2.capture_array()  # This is in RGB format

        # Display raw camera input for debugging
        raw_bgr_frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)
        cv2.imshow("Raw Camera Input", raw_bgr_frame)

        # Annotate frame with FPS for debugging
        curr_time = time.time()
        fps = 1 / (curr_time - prev_time) if prev_time else 0
        prev_time = curr_time

        fps_str = f"FPS: {fps:.1f}"
        annotated_frame = frame.copy()
        cv2.putText(
            annotated_frame, fps_str, (10, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2
        )
        
        # Display the annotated frame
        annotated_bgr_frame = cv2.cvtColor(annotated_frame, cv2.COLOR_RGB2BGR)
        cv2.imshow("Annotated Frame", annotated_bgr_frame)

        # Exit on 'Esc'
        if cv2.waitKey(1) & 0xFF == 27:
            break

finally:
    # Clean up
    cv2.destroyAllWindows()
    picam2.stop()
