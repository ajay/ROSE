from flask import Flask, render_template, request, redirect
import os
from pymongo import MongoClient

def connect():
# Substitute the 5 pieces of information you got when creating
# the Mongo DB Database (underlined in red in the screenshots)
# Obviously, do not store your password as plaintext in practice
    connection = MongoClient()
    handle = connection["rosedb"]
    return handle

app = Flask(__name__)
handle = connect()

# Bind our index page to both www.domain.com/ 
#and www.domain.com/index
@app.route("/index" ,methods=['GET'])
@app.route("/", methods=['GET'])
def index():
    userinputs = [x for x in handle.mycollection.find()]
    #speed = handle.mycollection.find({"_id":1})[0]["speed"]
    return render_template('index.html', userinputs=userinputs)

@app.route("/test", methods=['GET', 'POST'])
def test():
	print "Test!"
	print request.method
	#userinputs = [x for x in handle.mycollection.find()]
	userinput = request.args.get("userinput")
        oid = handle.mycollection.update({"_id":1},{"state":userinput},True)
        print userinput
        #userinputs = [x for x in handle.mycollection.find()]
	#return render_template('index.html', userinputs=userinputs)
	return 'Where does this return?'

@app.route("/write", methods=['POST'])
def write():
    userinput = request.form.get("userinput")
    oid = handle.mycollection.update({"_id":1},{"state":userinput}, True)
    handle.mycollection.update({"_id":2},{"speed":0.0},True)
    handle.mycollection.update({"_id":3},{"rotation":0},True);
    return redirect ("/")
    
    
    
@app.route("/up", methods=['POST'])
def up():
    oid = handle.mycollection.update({"_id":1},{"state":"FORWARD"}, True)
    return redirect ("/")

@app.route("/down", methods=['POST'])
def down():
    oid = handle.mycollection.update({"_id":1},{"state":"BACKWARD"}, True)
    return redirect ("/")    

@app.route("/left", methods=['POST'])
def left():
    oid = handle.mycollection.update({"_id":1},{"state":"LEFT"}, True)
    return redirect ("/")

@app.route("/right", methods=['POST'])
def right():
    oid = handle.mycollection.update({"_id":1},{"state":"RIGHT"}, True)
    return redirect ("/")

@app.route("/northwest", methods=['POST'])
def northwest():
    oid = handle.mycollection.update({"_id":1},{"state":"NORTHWEST"}, True)
    return redirect ("/")

@app.route("/northeast", methods=['POST'])
def northeast():
    oid = handle.mycollection.update({"_id":1},{"state":"NORTHEAST"}, True)
    return redirect ("/")

@app.route("/southwest", methods=['POST'])
def southwest():
    oid = handle.mycollection.update({"_id":1},{"state":"SOUTHWEST"}, True)
    return redirect ("/")

@app.route("/southeast", methods=['POST'])
def southeast():
    oid = handle.mycollection.update({"_id":1},{"state":"SOUTHEAST"}, True)
    return redirect ("/")


@app.route("/reset", methods=['POST'])
def reset():
    oid = handle.mycollection.update({"_id":1},{"state":"STOP"}, True)
    return redirect ("/")

@app.route("/speedup", methods=['POST'])
def speedup():
	print "speed:%d"%(handle.mycollection.find({"_id":2})[0]["speed"])
	spe = handle.mycollection.find({"_id":2})[0]["speed"] + 0.05 if handle.mycollection.find({"_id":2})[0]["speed"] < 1 else handle.mycollection.find({"_id":2})[0]["speed"]
	oid = handle.mycollection.update({"_id":2},{"speed":spe}, True)
	return redirect ("/")

@app.route("/speeddown", methods=['POST'])
def speeddown():
	print "speed:%d"%(handle.mycollection.find({"_id":2})[0]["speed"])
	spe = handle.mycollection.find({"_id":2})[0]["speed"] - 0.05 if handle.mycollection.find({"_id":2})[0]["speed"] > 0 else handle.mycollection.find({"_id":2})[0]["speed"]
	oid = handle.mycollection.update({"_id":2},{"speed":spe}, True)
	return redirect ("/")

@app.route("/rotateCW", methods = ['POST'])
def rotateCW():
    oid = handle.mycollection.update({"_id":3},{"rotation": 1}, True)
    return redirect ("/")

@app.route("/rotateCCW", methods = ['POST'])
def rotateCCW():
    oid = handle.mycollection.update({"_id":3},{"rotation": -1}, True)
    return redirect ("/")

@app.route("/rotatereset", methods = ['POST'])
def rotatereset():
    oid = handle.mycollection.update({"_id":3},{"rotation": 0}, True)
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
