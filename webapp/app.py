from flask import Flask, render_template, request, redirect
import os
from pymongo import MongoClient

def connect():
# Substitute the 5 pieces of information you got when creating
# the Mongo DB Database (underlined in red in the screenshots)
# Obviously, do not store your password as plaintext in practice
    # LOCALHOST
    connection = MongoClient()
    handle = connection["rosedb"]
    return handle
    # MONGOLAB
    # connection = MongoClient("ds015878.mongolab.com", 15878)
    # handle = connection["rosedb"]
    # handle.authenticate("Brice","12345678")
    # return handle

app = Flask(__name__)
handle = connect()

# Bind our index page to both www.domain.com/
# and www.domain.com/index
@app.route("/index" ,methods=['GET'])
@app.route("/", methods=['GET'])
def index():
    userinputs = [x for x in handle.mycollection.find()]
    # speed = handle.mycollection.find({"state":{"$exists":True}})[0]["speed"]
    return render_template('index.html', userinputs=userinputs)

@app.route("/test", methods=['GET', 'POST'])
def test():
	print "Test!"
	print request.method
	#userinputs = [x for x in handle.mycollection.find()]
	userinput = request.args.get("userinput")
        oid = handle.mycollection.update({"state":{"$exists":True}},{"state":userinput},True)
        print userinput
        #userinputs = [x for x in handle.mycollection.find()]
	#return render_template('index.html', userinputs=userinputs)
	return 'Where does this return?'

@app.route("/write", methods=['POST'])
def write():
    userinput = request.form.get("userinput")
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":userinput}, True)
    handle.mycollection.update({"speed":{"$exists":True}},{"speed":0.0},True)
    handle.mycollection.update({"rotation":{"$exists":True}},{"rotation":0},True);
    return redirect ("/")

#

@app.route("/north", methods=['POST'])
def up():
    oid = handle.mycollection.update({"state":{"$exists":True}}, {"state":"NORTH"}, True)
    return redirect ("/")

@app.route("/south", methods=['POST'])
def down():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"SOUTH"}, True)
    return redirect ("/")

@app.route("/west", methods=['POST'])
def left():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"WEST"}, True)
    return redirect ("/")

@app.route("/east", methods=['POST'])
def right():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"EAST"}, True)
    return redirect ("/")

@app.route("/northwest", methods=['POST'])
def northwest():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"NORTHWEST"}, True)
    return redirect ("/")

@app.route("/northeast", methods=['POST'])
def northeast():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"NORTHEAST"}, True)
    return redirect ("/")

@app.route("/southwest", methods=['POST'])
def southwest():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"SOUTHWEST"}, True)
    return redirect ("/")

@app.route("/southeast", methods=['POST'])
def southeast():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"SOUTHEAST"}, True)
    return redirect ("/")


@app.route("/reset", methods=['POST'])
def reset():
    oid = handle.mycollection.update({"state":{"$exists":True}},{"state":"STOP"}, True)
    return redirect ("/")

@app.route("/speedup", methods=['POST'])
def speedup():
	print (handle.mycollection.find({"speed":{"$exists":True}}) is not None)
	if handle.mycollection.find({"speed":{"$exists":True}}) is not None:
		print ("speed:%f"%handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"])
		spe = handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"] + 0.05 if handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"] <= 0.95 else handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"]
	else:
		spe = 0.05
	oid = handle.mycollection.update({"speed":{"$exists":True}},{"speed":spe}, True)
	return redirect ("/")

@app.route("/speeddown", methods=['POST'])
def speeddown():
	if handle.mycollection.find({"speed":{"$exists":True}}) is not None:
		print ("speed:%f"%handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"])
		spe = handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"] - 0.05 if handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"] >= 0.05 else handle.mycollection.find({"speed":{"$exists":True}})[0]["speed"]
	else:
		spe = 0.0
	oid = handle.mycollection.update({"speed":{"$exists":True}},{"speed":spe}, True)
	return redirect ("/")

@app.route("/rotateCW", methods = ['POST'])
def rotateCW():
    oid = handle.mycollection.update({"rotation":{"$exists":True}},{"rotation": 1}, True)
    return redirect ("/")

@app.route("/rotateCCW", methods = ['POST'])
def rotateCCW():
    oid = handle.mycollection.update({"rotation":{"$exists":True}},{"rotation": -1}, True)
    return redirect ("/")

@app.route("/rotatereset", methods = ['POST'])
def rotatereset():
    oid = handle.mycollection.update({"rotation":{"$exists":True}},{"rotation": 0}, True)
    return redirect ("/")

def threadtest():
	return

@app.route("/deleteall", methods=['GET'])
def deleteall():
    handle.mycollection.remove()
    return redirect ("/")

# Remove the "debug=True" for production
if __name__ == '__main__':
    # Bind to PORT if defined, otherwise default to 5000.
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=True)
