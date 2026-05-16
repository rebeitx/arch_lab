import requests, json, time

url = "http://172.25.73.218:8081/commands"

while True:
    response = requests.get(url)
    if response.status_code == 200:
        with open("answer.json", "w") as f:
            json.dump(response.json(), f)
    time.sleep(0.1)
