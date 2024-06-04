import time
import argparse
import queue
import cv2
import threading
import logging
from threading import Thread


class Containers:
	def __init__(self):
		self.q1 = queue.Queue()
		self.q2 = queue.Queue()
		self.q3 = queue.Queue()
		self.im = queue.Queue()
	def put(self, ind, obj):
		match ind:
			case 1:
				self.q1.put(obj)
			case 2:
				self.q2.put(obj)
			case 3:
				self.q3.put(obj)
			case 4:
				self.im.put(obj)
	def get(self, ind):
		match ind:
			case 1:
				if self.q1.empty():
					return None
				return self.q1.get()
			case 2:
				if self.q2.empty():
					return None
				return self.q2.get()
			case 3:
				if self.q3.empty():
					return None
				return self.q3.get()
			case 4:
				if self.im.empty():
					return None
				return self.im.get()

class Sensor:
	def get(self):
		raise NotImplementedError("Implement get()!")

class SensorX(Sensor):
	def __init__(self, delay : float):
		self._delay = delay
		self._data = 0

	def get(self) -> int:
		time.sleep(self._delay)
		self._data += 1
		return self._data

class SensorCam(Sensor):
	def __init__(self, cam_name, q):
		self.camera = cam_name
		self.h = q[1]
		self.w = q[0]
		try:
			self._cam = cv2.VideoCapture(0+cv2.CAP_DSHOW)
			if not self._cam.isOpened():
				raise Exception('Camera is not openned!')
		except Exception as e:
			logging.error(f"Error: {e}")
			exit(e)
	def get(self):
		try:
			_, image = self._cam.read()
			if image is None:
				raise Exception("Error getting image from camera (Check if camera is connected)")
		except Exception as e:
			logging.error(f"Error: {e}")
			exit(e)
		image = cv2.resize(image, (self.w, self.h))
		return image
	def __del__(self):
		self._cam.release()

class WindowImage(Sensor):
	def __init__(self, hz):
		self.hz = hz
		self.sensors = [0, 0, 0]

	def show(self, image):
		if image is None:
			logging.error("Error: invalid image!")
			exit("Invalid image!")
		cv2.putText(image, "Sensor0: " + str(self.sensors[0]), (20, 40), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
		cv2.putText(image, "Sensor1: " + str(self.sensors[1]), (20, 80), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
		cv2.putText(image, "Sensor2: " + str(self.sensors[2]), (20, 120), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
		cv2.imshow('Image', image)
		time.sleep(1/self.hz)
	def set_sensor(self, ind, val):
		self.sensors[ind-1] = val

	def __del__(self):
		cv2.destroyAllWindows()
		
lock = threading.Lock()
condition = threading.Condition()
flag = True

def threaded_function_2():
	global flag, data_queue, lock
	while flag:
		sens = sensor2.get()
		lock.acquire()
		try:
			data_queue.put(3, sens)
		finally:
			lock.release()

def threaded_function_1():
	global flag, data_queue, lock
	while flag:
		sens = sensor1.get()
		lock.acquire()
		try:
			data_queue.put(2, sens)
		finally:
			lock.release()
		

def cam_function():
	global flag, data_queue, lock
	while flag:
			im = cams0.get()
			lock.acquire()
			try:
				data_queue.put(4, im)
			finally:
				lock.release()


def threaded_function_0():
	global flag, data_queue, lock
	while flag:
		sens = sensor0.get()
		lock.acquire()
		try:
			data_queue.put(1, sens)
		finally:
			lock.release()
		

def main_function_0():
	global flag, data_queue, lock
	while flag:
		lock.acquire()
		try:
			for i in range(1, 5):
				data = data_queue.get(i)
				if data is not None:
					if i == 4:
						shows.show(data)
					else:
						shows.set_sensor(i, data)
		finally:
			lock.release()
		if cv2.waitKey(1) & 0xFF == ord('q'):
			flag = False
			break

logging.basicConfig(filename='errors.log', level=logging.ERROR, format='%(asctime)s - %(levelname)s - %(message)s')
		

data_queue = Containers()	

sensor0 = SensorX(0.01)
sensor1 = SensorX(0.1)
sensor2 = SensorX(1)

parser = argparse.ArgumentParser(description='Парсинг параметров видеокамеры')
parser.add_argument('--name', type=str, default='name', help='Имя видеокамеры')
parser.add_argument('--quality', type=str, default='400x300', help='Разрешение видеокамеры')
parser.add_argument('--hz', type=int, default=100, help='Частота обновления картинки')

# Парсим аргументы
try:
	args = parser.parse_args()
	if args.name is None or args.quality is None or args.hz is None:
		raise Exception("Invalid input arguments!")
except Exception as e:
	logging.error(f"Error: {e}")

# Выводим результаты
print('Имя видеокамеры:', args.name)
print('Разрешение видеокамеры:', args.quality)
print('Частота обновления картинки:', args.hz)

w, h = map(int, args.quality.split("x"))

# Вывод результатов
print("Ширина:", w)
print("Высота:", h)

shows = WindowImage(args.hz)

cams0 = SensorCam(args.name, (w, h))



if __name__ == "__main__":

	thread0 = Thread(target=threaded_function_0)
	thread1 = Thread(target=threaded_function_1)
	thread2 = Thread(target=threaded_function_2)
	camera = Thread(target=cam_function)
	main_thread = Thread(target=main_function_0)

	camera.start()
	thread0.start()
	thread1.start()
	thread2.start()
	main_thread.start()

	camera.join()
	thread1.join()
	thread2.join()
	thread0.join()
	main_thread.join()