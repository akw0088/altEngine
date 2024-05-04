import numpy as np
import tifffile as tif
#from PIL import Image
import cv2

image = tif.imread('e:\\test.tif')

image_up = (image & 0xFF00) >> 8
image_down = image & 0xFF

out_img = np.zeros(shape=(2048,2048,3))

#extract channels
red_channel = image_down.astype('uint8')
green_channel = image_up.astype('uint8')
#blue_channel = temp[:,:,:,2]

out_img[:,:,0] = 128
out_img[:,:,1] = green_channel
out_img[:,:,2] = red_channel

#write red channel to greyscale image
cv2.imwrite('merge.bmp', out_img) 
