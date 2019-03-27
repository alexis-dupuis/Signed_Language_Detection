import numpy as np
import cv2
from primesense import openni2
from primesense import _openni2 as c_api

openni2.initialize()     # can also accept the path of the OpenNI redistribution

dev = openni2.Device.open_any()

depth_stream = dev.create_color_stream()
depth_stream.start()

size = (640,480)

out = cv2.VideoWriter("calib_kinect.avi",cv2.VideoWriter_fourcc(*'DIVX'), 24, size)

i=0
img_array = []

key = -1

while(key == -1):
    i += 1

    frame = depth_stream.read_frame()
    frame_data = frame.get_buffer_as_uint8()

    img = np.frombuffer(frame_data, dtype=np.uint8)
    img.shape = (480,640,3)
    
    RGB_img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
    #img = np.concatenate((img, img, img), axis=0)
    #img = np.swapaxes(img, 0, 2)
    #img = np.swapaxes(img, 0, 1)
    
    #img_name = str(i) + ".jpg"
    
    #cv2.imwrite("record_kinect/" + img_name, RGB_img)
    #img_array.append(RGB_img)
    out.write(RGB_img)
    
    cv2.imshow("image", RGB_img)

    key = cv2.waitKey(34)


depth_stream.stop()
openni2.unload()

print( "Exited successfuly" )
out.release()

"""
for k in range(len(img_array)):
    out.write(img_array[k])
out.release()

print("Recorded successfuly")
"""