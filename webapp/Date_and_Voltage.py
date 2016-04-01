from flask import Flask, render_template, request, redirect
import os
from pymongo import MongoClient
# import datetime
import time


#initialization
#global t0 
#t0 = time.clock() # initial time
#time.sleep(1.5) #1.5 seconds
#start_time = time.time()
start_clock = time.clock()

def connect():   
	#connection = MongoClient("ds015878.mongolab.com",15878)
	connection = MongoClient()
	handle = connection["rosedb"]
	#handle.authenticate("Jon","12345678")
	return handle
#^ USING original test site mlabs to try to display only VALUE in collection robo_info
# Jon Cheng 3/30/16, 11pm EDT -- testing done 11:45pm EDT

def display_voltage():
#------------------------------------------------------------------------
# Iterating through robo_info collection test collection on mlabs
# attempting to display only the VALUES

	cursor = handle.robo_info.find()
	
	for a in cursor:
		x = a['current_voltage'] #retrieves value that cursor iterates thtrough
		print (x) # prints purely the value out - type determined by program


def send_time_stamp():
	variable = {"clock": time.clock()}
	time_stamp = handle.time_stamp
	post_confirm = time_stamp.insert_one(variable).inserted_id
	print post_confirm
	print "date and time sent to the database! \n"



def update_time_stamp():
	some_update = handle.time_stamp.update_one(
	{	"clock": time.clock()},
			{"$currentDate": {"lastModified": True}

			}
		
		)
	# for le testing purrrpuhsee
	cursor = handle.time_stamp.find()
	t0 = time.clock()
	print t0 - start_clock
#	for b in cursor:
#		xx = b['clock']*1000


	



#cursor = db.robo_info.find({},{"made_up_voltage":1, "_id":0})
#arr = []
#for documents in cursor: 
#arbitrary variable of documents in 'cursor', which iterates through the collection
	#arr.append(documents)
#print arr 
#------------------------------------------------------------------------
# Send data to database along with a timestamp -- datetime by Python, get it by the ms
# document called timestamp or something, where we store value into that; check it func.


app = Flask(__name__)
#handle = connect()
handle = connect() # creates if nonexistent; access otherwise
collect_values = ['robo_info'] 
#^ creates collection of this name if not found; otherwise, accesses it

result = handle.time_stamp.delete_many({}) #delete everythinggg

display_voltage()
send_time_stamp()

for x in range(0,3):
	update_time_stamp() #updates and displays
	time.sleep(1) # every 1 ms 
	print "Time updated! Waiting 1 millisecond..."


# clock within datetime library
# push in ms and by the ms
	# update variable in even intervals...clock(t) and difference that and 
	# other variable, threshold check -- connection not lost 
	# if lost, oh well
# update time_stamp() - built-in function 

# TODO - fix up the conversion and ensure logic
# is consistent in terms of time stamp variables 