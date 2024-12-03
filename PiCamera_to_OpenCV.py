from picamera2 import Picamera2
import cv2

# Initialize Picamera2
picam2 = Picamera2()

# # Configure the camera with a default preview resolution
# camera_config = picam2.create_preview_configuration(main={"size": (2592, 1944), "format": "XRGB8888"})
# picam2.configure(camera_config)

# Start the camera
picam2.start()

try:
    while True:
        # Capture frame from the camera
        frame = picam2.capture_array()  # The frame is in RGB format

        # Convert the RGB frame to BGR for OpenCV
        bgr_frame = cv2.cvtColor(frame, cv2.COLOR_RGB2BGR)

        # Display the camera feed
        cv2.imshow("Camera Feed", bgr_frame)

        # Exit the loop when 'Esc' is pressed
        if cv2.waitKey(1) & 0xFF == 27:
            break

finally:
    # Clean up
    cv2.destroyAllWindows()
    picam2.stop()
