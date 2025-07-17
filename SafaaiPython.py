import cv2
import numpy as np
from elements.yolo import OBJ_DETECTION
import RPi.GPIO as GPIO
from time import sleep

# GPIO setup
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)

PLASTIC_PIN = 36  # GPIO16 (Closer to GPIO20)
NPLASTIC_PIN = 38  # GPIO20 (Closer to GPIO16)

GPIO.setup(PLASTIC_PIN, GPIO.OUT, initial=GPIO.LOW)
GPIO.setup(NPLASTIC_PIN, GPIO.OUT, initial=GPIO.LOW)

# Object detection setup
Object_classes = ['plastic', 'nplastic']
Object_colors = list(np.random.rand(len(Object_classes), 3) * 255)
Object_detector = OBJ_DETECTION('plastic.pt', Object_classes)  # Update with actual model path

# Video capture
cap = cv2.VideoCapture(0)
if cap.isOpened():
    window_handle = cv2.namedWindow("Camera", cv2.WINDOW_AUTOSIZE)
    
    while cv2.getWindowProperty("Camera", 0) >= 0:
        ret, frame = cap.read()
        
        if ret:
            objs = Object_detector.detect(frame)
            
            # Reset both GPIO pins to LOW before processing
            GPIO.output(PLASTIC_PIN, GPIO.LOW)
            GPIO.output(NPLASTIC_PIN, GPIO.LOW)
            
            if len(objs) == 0:
                print('No object detected')
            
            for obj in objs:
                label = obj['label']
                score = obj['score']
                
                if label == 'plastic':
                    print(f"Plastic detected with confidence {score:.2f}")
                    GPIO.output(PLASTIC_PIN, GPIO.HIGH)
                    sleep(2)
                    GPIO.output(PLASTIC_PIN, GPIO.LOW)

                elif label == 'nplastic':
                    print(f"Non-plastic detected with confidence {score:.2f}")
                    GPIO.output(NPLASTIC_PIN, GPIO.HIGH)
                    sleep(1)
                    GPIO.output(NPLASTIC_PIN, GPIO.LOW)

                # Bounding box
                [(xmin, ymin), (xmax, ymax)] = obj['bbox']
                color = Object_colors[Object_classes.index(label)]
                frame = cv2.rectangle(frame, (xmin, ymin), (xmax, ymax), color, 2)

        cv2.imshow("Camera", frame)
        keyCode = cv2.waitKey(1)
        if keyCode == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()
else:
    print("Unable to open camera")
