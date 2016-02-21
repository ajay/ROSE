#the following code connects to the remote server with the ip address "192.168.1.2"
#and prints out the documents of a collection in the database
from pymongo import MongoClient

#the ip address will need to change accordingly. 27017 is the main mongodb port
client = MongoClient('192.168.1.2',27017)
db = client['test']
#for my database, i had a collection called "coll"
#if the collection name you are looking for is different, change "coll" to the desired name
#coll = db['dataset']

cursor = db.coll.find()

for document in cursor:
	print(document)
