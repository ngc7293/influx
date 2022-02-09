import json
from flask import Flask, request

app = Flask(__name__)


@app.route("/", methods=['GET', 'POST'])
def hello_world():
    print(request.headers)
    print(json.loads(request.data.decode()))
    return ""
