import requests
import json
import time

url = "http://172.25.73.218:8081/commands"
with open("message.json", "r") as f:
   data = json.load(f)

def sndpost():
    response = requests.post(url, json=data)
    if response.status_code == 200:
        print("Request successful!")
        print("Response JSON:", response.json())
        with open("answer.json", "w") as f:
            json.dump(response.json(), f)
    else:
       print(f"Request failed with status code {response.status_code}")
       print("Response text:", response.text)
sndpost()
