from pymongo import MongoClient
from datetime import datetime
#look into this library later:
#from bson.objectid import OgjectId

#NOTE: you can call mongoDB commands from the terminal by typing:
#	mongo --eval " *command to execute* "

client = MongoClient('mongodb://cedric:SegaSmashSoma540@192.168.239.146:27017/test') #creates connection to database
db = client['test'] #creates database if database name does not exist, accesses database otherwise
coll = db['dataset'] #creates a collection within the database

# docum = db.coll.insert_one( #inserts specified documents into the collection
# 	{
# 		"message": "Hello World!",
# 		"Time Stamp": datetime.now(),
# 	}
# )

cursor = db.coll.find() #sets pointer to traverse collection

#"document" is just the name we give to the elements inside "cursor"
#theres nothing special about the name "document" in this case
for document in cursor:
	print(document)

#when updating a document, the value of the document being updated must match
#the value that you want to update the document to.