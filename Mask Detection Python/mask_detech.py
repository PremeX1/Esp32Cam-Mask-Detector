import tracemalloc
from keras.models import load_model
from PIL import Image, ImageOps
import numpy as np
import cv2
from imutils.video import VideoStream
import websocket
from datetime import datetime

now = datetime.now()

# arduino = serial.Serial(port='COM5', baudrate=115200)

model = load_model('./train_model/keras_model.h5')
face_cascade=cv2.CascadeClassifier("detech\haarcascade_frontalface_alt.xml")

ws = websocket.WebSocket()
ws.connect("ws://192.168.137.241:81")

while True:
	cap_time = now.strftime("%d-%m-%Y-%H-%M-%S")
	#test image
	# frame = cv2.imread("./Test.png")
	# img = frame

	resp = ws.recv()
	decoded = cv2.imdecode(np.frombuffer(resp, np.uint8), -1)
	frame = decoded
	img = frame
	frame = vs.read() 
	imgResponse=urllib.request.urlopen(url)
	imgnp=np.array(bytearray(imgResponse.read()),dtype=np.uint8)
	img=cv2.imdecode(imgnp,-1)
	r, frame = stream.read()
	
	image_grey = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

	image_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
	(thresh, BW_IMG) = cv2.threshold(image_grey, 127, 255, cv2.THRESH_BINARY)

	dark_img  = cv2.cvtColor(frame, cv2.COLOR_BGR2HLS)
	light_level = dark_img[:,:,1]
	light_level_value =cv2.mean(light_level)[0]
	cv2.putText(dark_img, str(light_level_value), (50, 100), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2, cv2.LINE_AA)
	cv2.imshow("WebCam [0] Bit Frame", dark_img) 

	faces = face_cascade.detectMultiScale(image_grey)

	for x,y,w,h in faces:
		crop_frame = Image.fromarray(image_rgb[y:y+h,x:x+w])

		data = np.ndarray(shape=(1, 224, 224, 3), dtype=np.float32)

		image = crop_frame 
		size = (224, 224)
		image = ImageOps.fit(image, size, Image.ANTIALIAS)

		image_array = np.asarray(image)

		normalized_image_array = (image_array.astype(np.float32) / 127.0) - 1

		data[0] = normalized_image_array
		
		prediction = model.predict(data)
		print(int(prediction[0][0]))
		print(int(prediction[0][1]))
		if(prediction[0][0] > prediction[0][1]):
			color = (0, 255, 0)
			ws.send('DEBEEP')
			no_mask_people = False
		else:
			color = (0, 0, 255)
			cv2.imwrite(f'./no_mask/{cap_time}.png',faces)
			ws.send('BEEP')
			no_mask_people = True
		img=cv2.rectangle(img,(x,y),(x+w,y+h),color,3)
		# arduino.write(bytes(str(arduino_signal), 'utf-8'))	
	Frame_Size = img
	cv2.imshow("WebCam [1][Release] Windows Frame", Frame_Size)
	cv2.imshow("WebCam [0] Bit Grey Frame", image_grey)
	if(no_mask_people): 
		cv2.imwrite(f'{cap_time}.png',frame)
		pass
	else:
		pass
	if cv2.waitKey(1) == ord('q'):
		break
	if(cv2.waitKey(1) == ord('f')):
		# ws.send('F-ON')
		pass
	if(cv2.waitKey(1) == 27):
		# ws.send('F-OFF')
		pass
