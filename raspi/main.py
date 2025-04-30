from picamera2 import Picamera2
import cv2
import numpy as np
import serial
import time
import libcamera

# Configurações da Serial
arduino = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
time.sleep(2)

# Carregar modelo YOLO
net = cv2.dnn.readNet("seu_modelo.weights", "seu_modelo.cfg")
classes = []
with open("coco.names", "r") as f:
    classes = [line.strip() for line in f.readlines()]

layer_names = net.getLayerNames()
output_layers = [layer_names[i[0] - 1] for i in net.getUnconnectedOutLayers()]

# Configuração da câmera
picam2 = Picamera2()
config = picam2.create_preview_configuration(
    main={"size": (640, 480), "format": "RGB888"},
    transform=libcamera.Transform(hflip=1, vflip=1))  # Ajustar flip conforme necessário
picam2.configure(config)
picam2.start()

# Parâmetros YOLO
conf_threshold = 0.5
nms_threshold = 0.4

def send_direction(direction):
    """Envia comando para o Arduino"""
    arduino.write(direction.encode())
    print(f"Enviado: {direction}")

def process_frame(frame):
    """Processa o frame e detecta cones"""
    height, width = frame.shape[:2]

    # Detecção de objetos
    blob = cv2.dnn.blobFromImage(frame, 0.00392, (416, 416), (0, 0, 0), True, crop=False)
    net.setInput(blob)
    outs = net.forward(output_layers)

    # Processar detecções
    class_ids = []
    confidences = []
    boxes = []
    
    for out in outs:
        for detection in out:
            scores = detection[5:]
            class_id = np.argmax(scores)
            confidence = scores[class_id]
            
            if confidence > conf_threshold and classes[class_id] == "cone":
                center_x = int(detection[0] * width)
                center_y = int(detection[1] * height)
                w = int(detection[2] * width)
                h = int(detection[3] * height)
                
                x = int(center_x - w / 2)
                y = int(center_y - h / 2)
                
                boxes.append([x, y, w, h])
                confidences.append(float(confidence))
                class_ids.append(class_id)

    # Aplicar Non-Maximum Suppression
    indexes = cv2.dnn.NMSBoxes(boxes, confidences, conf_threshold, nms_threshold)
    
    if len(indexes) > 0:
        for i in indexes.flatten():
            x, y, w, h = boxes[i]
            
            center_x = x + w//2
            center_y = y + h//2
            
            if center_x < width//3:
                send_direction("LEFT")
            elif center_x > 2*width//3:
                send_direction("RIGHT")
            else:
                send_direction("FORWARD")
            
            cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
            cv2.putText(frame, "Cone", (x, y - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 2)
            return frame
    
    send_direction("STOP")
    return frame

try:
    while True:
        # Capturar frame da câmera
        frame = picam2.capture_array()
        
        # Processar frame
        processed_frame = process_frame(frame)
        
        # Mostrar resultado
        cv2.imshow("Cone Detection", processed_frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

except KeyboardInterrupt:
    pass

finally:
    picam2.stop()
    cv2.destroyAllWindows()
    arduino.close()
    print("Recursos liberados")
