import socket

tuskaddr = '78:E3:6D:18:F5:DA'
port = 1 
size = 4096

s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
s.connect((tuskaddr, port))

try:
    while 1:
        data = s.recv(size)
        if data:
            print(data)
except:	
    print("Closing socket")	
    s.close()
