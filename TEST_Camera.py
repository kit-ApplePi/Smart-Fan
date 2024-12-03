import cv2

cap = cv2.VideoCapture(0)  # Try with '0' for default camera or '/dev/video0'

if not cap.isOpened():
    print("Error: Camera not accessible.")
    exit()

# Test if the camera captures frames
while True:
    ret, frame = cap.read()
    if not ret:
        print("Failed to capture frame.")
        break
    cv2.imshow("Camera Test", frame)
    if cv2.waitKey(1) & 0xFF == 27:  # Press ESC to exit
        break

cap.release()
cv2.destroyAllWindows()
