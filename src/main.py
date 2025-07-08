import time
import requests

ip = "5.201.9.166"

for i in range(0,5):
    url = "http://ip-api.com/json/" + ip

    start = time.time()
    response = requests.get(url)
    end = time.time()
    dur = (end - start) * 1000

    if response.status_code == 200:
        data = response.json()
        print(f"IP: {ip}")
        print(f"Lat: {data.get('lat')}")
        print(f"Lon: {data.get('lon')}")
        print(f"Response Time: {dur:.2f} ms")
    else:
        print(response.status_code, response.text)



