from flask import Flask, render_template, request, redirect
import os
from pymongo import MongoClient

def connect():
# Substitute the 5 pieces of information you got when creating
# the Mongo DB Database (underlined in red in the screenshots)
# Obviously, do not store your password as plaintext in practice
    # LOCALHOST
    #connection = MongoClient()
    #handle = connection["rosedb"]
    #return handle
    # MONGOLAB
     connection = MongoClient("ds015878.mongolab.com", 15878)
     handle = connection["rosedb"]
     handle.authenticate("Brice","12345678")
     return handle

app = Flask(__name__)
handle = connect()
tablenumber = 0

#This binds our index page to both "localhost:xxxx/" and "localhost:xxxx/index/".
#If navigating to that url, index.html will be shown.
@app.route("/index/<tblnum>" ,methods=['GET'])
@app.route("/<tblnum>", methods=['GET'])
def index(tblnum):
    table = tblnum
    # orders = [x for x in handle.orders.find()]
    return render_template('index.html', table=table)

#This is used whenever someone places an order.
@app.route("/order/table=<table>&items=<items>&prices=<prices>", methods=['POST'])
def order(table, items, prices):
    print "ORDER!"
    print items
    print prices
    print table
    oid = handle.orders.insert({"table":table,"items":items,"prices":prices});
    print oid
    
    return redirect ("/"+table)

@app.route("/deleteall", methods=['GET'])
def deleteall():
    handle.orders.remove()
    return redirect ("/")
    
def tokenize(string):
    return

# Remove the "debug=True" for production
if __name__ == '__main__':
    # Bind to PORT if defined, otherwise default to 5000.
    port = int(os.environ.get('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=True)
