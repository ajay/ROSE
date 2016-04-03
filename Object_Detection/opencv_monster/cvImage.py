import cv2
import numpy as np
import os
import urllib.request

#Download neg images function
def store_neg_images():
	#Get images from imagenet
	#Sports Images: http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=n00523513
	#People Images: http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=n07942152
	#Restaurant Images: http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=n04081281
	neg_images_link = 'http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=n04081281'
	neg_image_urls = urllib.request.urlopen(neg_images_link).read().decode()

	#create a folder called neg to place the negative images in, if the folder doesn't already exist
	if not os.path.exists('neg'):
		os.makedirs('neg')

	#Counter to name the pictures you get from the url link (change this number, so you don't override old images when adding new ones)
	pic_num = 2000

	for i in neg_image_urls.split('\n'):
		 try:
		 	print(i)
		 	urllib.request.urlretrieve(i, "neg/"+str(pic_num)+".jpg")
		 	img = cv2.imread("neg/"+str(pic_num)+".jpg",cv2.IMREAD_GRAYSCALE)
		 	resized_image = cv2.resize(img, (200, 200))
		 	cv2.imwrite("neg/"+str(pic_num)+".jpg",resized_image)
		 	pic_num += 1

		 except Exception as e:
		 	print(str(e))
##Call this function only if you want to download new images, otherwise comment it out
#store_neg_images()

#Download pos images function
def store_pos_images():
	#Get images from imagenet
	#Coke Images: http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=n07928696
	pos_images_link = 'http://www.image-net.org/api/text/imagenet.synset.geturls?wnid=n07928696'
	pos_image_urls = urllib.request.urlopen(pos_images_link).read().decode()

	#create a folder called neg to place the negative images in, if the folder doesn't already exist
	if not os.path.exists('pos'):
		os.makedirs('pos')

	#Counter to name the pictures you get from the url link (change this number, so you don't override old images when adding new ones)
	pic_num = 0

	for i in pos_image_urls.split('\n'):
		 try:
		 	print(i)
		 	urllib.request.urlretrieve(i, "pos/"+str(pic_num)+".jpg")
		 	img = cv2.imread("pos/"+str(pic_num)+".jpg",cv2.IMREAD_GRAYSCALE)
		 	resized_image = cv2.resize(img, (200, 200))
		 	cv2.imwrite("pos/"+str(pic_num)+".jpg",resized_image)
		 	pic_num += 1

		 except Exception as e:
		 	print(str(e))
##Call this function only if you want to download new images, otherwise comment it out
store_pos_images()

#Remove ugly(unnecessary/bad) image
def find_uglies():
	match = False
	for file_type in ['neg']:
		for img in os.listdir(file_type):
			for ugly in os.listdir('uglies'):
				try:
					current_image_path = str(file_type) + '/' + str(img)
					ugly = cv2.imread('uglies/'+str(ugly))
					question = cv2.imread(current_image_path)
					if ugly.shape == question.shape and not(np.bitwise_xor(ugly,question).any()):
						print('Ugly pic deleting')
						print(current_image_path)
						os.remove(current_image_path)

				except Exception as e:
					print(str(e))
#Call this funtion to remove uglies, otherwise comment it out
# find_uglies()

#Create positive images by superimposing on negative images
def create_pos_n_neg():

	for file_type in ['neg']:

		 for img in os.listdir(file_type):
		 	if file_type == 'pos' :
		 		line = file_type + '/' + img + ' 1 0 0 50 50\n'
		 		with open('info.dat' , 'a') as f:
		 			f.write(line)
		 	elif file_type == 'neg':
		 		line = file_type + '/' + img + '\n'
		 		with open('bg.txt', 'a') as f:
		 			f.write(line)
#Call this function to create bg.txt file with list of all neg images (and pos if u had any)
#create_pos_n_neg()

