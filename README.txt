Smart Waste Bin with ESP32 and YOLOv5
Overview
This project is a smart waste bin system built using ESP32, sensors, and machine learning. The system detects plastic waste using a YOLOv5 model running on a Raspberry Pi. When plastic is detected, the servo-controlled door opens, and the data is sent to Firebase for real-time tracking.

Components
ESP32: Handles communication with sensors and controls the servo motor.

YOLOv5: Detects plastic waste using a trained machine learning model on Raspberry Pi.

Firebase: Stores and updates the waste detection data in real-time.

Servo Motors: Control the door mechanism for sorting waste.



Software Setup
Install Libraries:

FirebaseESP32 library for Firebase communication

Servo library for motor control

Any other libraries for sensor integration

Upload Code to ESP32:

Use Arduino IDE to upload the code to your ESP32

Ensure the correct board and port are selected in Arduino IDE

Firebase Configuration:

Set up a Firebase project and obtain your credentials (API key, database URL, etc.)

Add Firebase configuration details to the code

YOLOv5 Model:

Train a YOLOv5 model to detect plastic waste (you can use pre-trained models or custom data)

Deploy the model on the Raspberry Pi

How It Works
Object Detection: The YOLOv5 model detects plastic waste and sends a signal to the ESP32.

Servo Control: The ESP32 processes the signal and moves the servo motor to open or close the door based on the detection result.

Firebase Update: The system updates Firebase with the type of waste detected (plastic or non-plastic) for tracking purposes.

Running the Project
Power up the ESP32 and Raspberry Pi.

Open the smart bin and place a plastic item near the sensor.

The servo motor should open if plastic is detected and close otherwise.

The Firebase database will update the detection status in real-time.