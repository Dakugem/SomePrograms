# echo-client.py

import socket
from struct import *

HOST = "127.0.0.1" 
PORT = 65000  
BUFF_SIZE = 64
ctr = 0
data = bytearray()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
    
Delta = 0
a = [0,0,0]
g = [0,0,0]
isEnd = False
while not isEnd:
    data = s.recv(BUFF_SIZE)
    ##print(f"{data.hex()}")
    Delta 
    Delta, g[0], g[1], g[2], a[0], a[1], a[2], isEnd = unpack('>qhhhhhh?', data)

    to_send = pack('>I', ctr)
    s.send(to_send)
    ctr += 1
    ctr %= 256**4
       
    Delta = Delta / 1000000
    g[0] = g[0] / 16384
    g[1] = g[1] / 16384
    g[2] = g[2] / 16384
    a[0] = a[0] / 16384
    a[1] = a[1] / 16384
    a[2] = a[2] / 16384
    print(f"Received {Delta:>6.4f}, {g[0]:>7.3f}, {g[1]:>7.03}, {g[2]:>7.3f}, {a[0]:>7.3f}, {a[1]:>7.3f}, {a[2]:>7.3f}")
    
s.close()