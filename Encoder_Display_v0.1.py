# Jonathan Cheng
# Modified PyMongo file, copied from Cedric and changed around to get and display info onto the UI from the DB
# side note: this was used in the beginning for comms b/w DB and robot, but was abandoned b/c we needed C++ instead
	# Cedric was working on that^, and got it done...mostly. - 3/14/16
#from pymongo import MongoClient
#from datetime import datetime
#look into this library later:
#from bson.objectid import ObjectId

#----------------------------------------------------------------
#from flask import Flask, render_template, request, redirect
import os
from pymongo import MongoClient

teststring = "Fish"

def connect():
# Substitute the 5 pieces of information you got when creating
# the Mongo DB Database (underlined in red in the screenshots)
# Obviously, do not store your password as plaintext in practice
    connection = MongoClient("ds015878.mongolab.com",15878)
    handle = connection["rosedb"]
    handle.authenticate("Jon","12345678")
    return handle

#app = Flask(__name__)
#handle = connect()
db = connect() # creates if nonexistent; access otherwise
collect_encoders = ['Encoders'] # creates collection of this name if not found; otherwise, accesses it

#cursor = db.Encoders.find()
cursor = db.Encoders.find({},{"Encoder_val":1, "_id":0})
arr = []

for documents in cursor:

	arr.append(documents)

print arr 



#----------------------------------------------------------------







#NOTE: you can call mongoDB commands from the terminal by typing:
#	mongo --eval " *command to execute* "


#client = MongoClient('mongodb://cedric:SegaSmashSoma540@192.168.239.146:27017/test') #creates connection to database
#db = client['test'] #creates database if database name does not exist, accesses database otherwise

#collect_encoders = db['Encoders'] #creates a collection within the database, or accesses it if it was already created

# docum = db.coll.insert_one( #inserts specified documents into the collection
# 	{
# 		"message": "Hello World!",
# 		"Time Stamp": datetime.now(),
# 	}
# )

#cursor = db.collect_encoders.find() #sets pointer to traverse collection called Encoders
# also calls ALL documents within the collection of this name, unless parameters are specified within the function call

#Enc_capture = db.collect_encoders.find({"encoder value": [tl, bl, tr, br]}) # first value is key, second is/(are) value(s) paired with it
# The two-letter abbreviations in the matrix/array stand for top left, back left, top right,  back right, respectively. Refers to our wheels

#"document" is just the name we give to the elements inside "cursor"
#theres nothing special about the name "document" in this case

#for document in Enc_capture:
#	print(document) # Attempting to syntatically output the values of the encoders onto the UI as an "array"

#when updating a document, the value of the document being updated must match
#the value that you want to update the document to.