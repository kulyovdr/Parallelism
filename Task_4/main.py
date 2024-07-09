import argparse
import threading
import time
import cv2
import logging
import sys
import queue
import numpy



class Sensor:
    def get(self):
        raise NotImplementedError("Subclasses must implement method get()")


class SensorX(Sensor):
    '''Sensor X'''
    def __init__(self, delay: float):
        self._delay = delay
        self._data = 0

    def get(self) -> int:
        time.sleep(self._delay)
        self._data += 1
        return self._data


class SensorCam(Sensor):
    def __init__(self, cam_name: str, resolution: tuple[int, int]):
        if cam_name == 'default':
            self.cam = cv2.VideoCapture(0)
        else:
            self.cam = cv2.VideoCapture(int(cam_name))
        self.cam.set(3, resolution[0])
        self.cam.set(4, resolution[1])

    def get(self):
        return self.cam.read()

    def __del__(self):
        self.cam.release()


class WindowImage:
    def __init__(self, freq: int):
        self.freq = freq
        cv2.namedWindow("window")

    def show(self, img: numpy.ndarray, s1: SensorX, s2: SensorX, s3: SensorX):
        x = 50
        y = 50
        text1 = f"Sensor 1: {s1}"
        text2 = f"Sensor 2: {s2}"
        text3 = f"Sensor 3: {s3}"
        cv2.putText(img, text1, (x, y), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        cv2.putText(img, text2, (x, y + 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        cv2.putText(img, text3, (x, y + 60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 0, 255), 2)
        cv2.imshow("window", img)

    def __del__(self):
        cv2.destroyWindow("window")


def process(queue: queue.Queue, sensor: SensorX, stop_event: threading.Event):
    while True:
        new_sens = sensor.get()
        if queue.empty():
            queue.put(new_sens)
        if stop_event.is_set():
            break


if __name__ == '__main__':
    logging.basicConfig(filename='log/errors.log', level=logging.ERROR, format='%(asctime)s - %(levelname)s - %(message)s')

    parser = argparse.ArgumentParser()
    split_char = '*'
    parser.add_argument('--cam', type=str, default='default', help='Camera name')
    parser.add_argument('--res', type=str, default='1280' + split_char + '720', help='Camera resolution')
    parser.add_argument('--freq', type=int, default=60, help='Output frequency')
    args = parser.parse_args()
    shapes = (int(args.res.split(split_char)[0]), int(args.res.split(split_char)[1]))

    sensor1 = SensorX(1)
    sensor2 = SensorX(0.1)
    sensor3 = SensorX(0.01)
    window = WindowImage(args.freq)
    camera = SensorCam(args.cam, shapes)

    if not camera.cam.isOpened():
        logging.error('There is no camera with that name in the system')
        camera.release()
        window.close()
        sys.exit()

    queue1 = queue.Queue()
    queue2 = queue.Queue()
    queue3 = queue.Queue()

    stop_event = threading.Event()
    thread1 = threading.Thread(target=process, args=(queue1, sensor1, stop_event))
    thread2 = threading.Thread(target=process, args=(queue2, sensor2, stop_event))
    thread3 = threading.Thread(target=process, args=(queue3, sensor3, stop_event))

    thread1.start()
    thread2.start()
    thread3.start()

    sensor_count1 = sensor_count2 = sensor_count3 = 0
    while True:
        if not queue1.empty():
            sensor_count1 = queue1.get()
        if not queue2.empty():
            sensor_count2 = queue2.get()
        if not queue3.empty():
            sensor_count3 = queue3.get()
        ret, frame = camera.get()
        if not ret or not camera.cam.isOpened() or not camera.cam.grab():
            logging.error('The camera is out of order')
            stop_event.set()
            thread1.join()
            thread2.join()
            thread3.join()
            sys.exit()

        window.show(frame, sensor_count1, sensor_count2, sensor_count3)
        time.sleep(1 / window.freq)

        if cv2.waitKey(1) == ord('q'):
            stop_event.set()
            thread1.join()
            thread2.join()
            thread3.join()
            sys.exit()
