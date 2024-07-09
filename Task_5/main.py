from ultralytics import YOLO
import threading
import queue
import time
import cv2
import argparse


result_frames_list = []


def fun_thread_read(video_path: str, frame_queue: queue.Queue, stop_event: threading.Event):
    video = cv2.VideoCapture(video_path)
    id_frame = 0
    while video.isOpened():
        ret, frame = video.read()
        if not ret:
            print("End of video!")
            break
        frame_queue.put((frame, id_frame))
        id_frame += 1
    stop_event.set()

def fun_thread_safe_predict(frame_queue: queue.Queue, stop_event: threading.Event):
    global result_frames_list
    model = YOLO(model="yolov8n-pose.pt")
    while True:
        try:
            frame, idd = frame_queue.get(timeout=1)
            result_frames_list.append((model.predict(source=frame, device='cpu')[0].plot(), idd))
        except queue.Empty:
            if stop_event.is_set():
                print(f'Thread {threading.get_ident()} final!')
                break
    
if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--video_name', type=str, default="test.mp4",  help='input video')
    parser.add_argument('--threads', type=int, default=1, help='number threads')
    parser.add_argument('--output_video', type=str, default="result.mp4",  help='output video')
    args = parser.parse_args()

    threads = []
    frame_queue = queue.Queue()
    stop_event = threading.Event()
    video_path = args.video_name
    thread_read = threading.Thread(target=fun_thread_read, args=(video_path, frame_queue, stop_event))
    thread_read.start()
    start_t = time.monotonic()
    for _ in range(args.threads):
        threads.append(threading.Thread(target=fun_thread_safe_predict, args=(frame_queue, stop_event)))

    for thread in threads:
        thread.start()

    for thread in threads:
        thread.join()

    thread_read.join()
    end_t = time.monotonic()
    print(f'Time: {end_t - start_t}')
    
    w = 640
    h = 480
    
    output_video = cv2.VideoWriter(filename=args.output_video, fourcc=cv2.VideoWriter_fourcc(*'mp4v'), fps=60, frameSize=(w, h))
    
    srt = sorted(result_frames_list, key=lambda x: x[1])
    
    for s in srt:
        output_video.write(s[0])
        
    output_video.release()