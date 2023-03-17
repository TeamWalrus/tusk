import socket

tuskaddr = "78:E3:6D:18:F5:DA"
port = 1 
size = 1024
data = b''

s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
s.connect((tuskaddr, port))

try:
    while True:        
        packet = s.recv(size)
        if len(packet) <= 0:
            break
        data += packet
        if len(data) > 60:
            print(f"[+] {data.decode()}")
            data = b''
        
except:	
    print("Closing socket")	
    s.close()