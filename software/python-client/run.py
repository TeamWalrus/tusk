import socket

tuskaddr = "78:E3:6D:18:F5:DA"
port = 1 
size = 1024
data = b''

try:
    s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    s.connect((tuskaddr, port))

except socket.error as serr:
    print(f"[!] Socket Error: {serr}")
    exit(1)

try:
    while True:        
        packet = s.recv(size)
        if len(packet) <= 0:
            break
        data += packet
        # if there is at least some card data, let's print it.
        # //todo could be a better way to do this
        if len(data) > 60:
            print(f"[+] {data.decode()}")
            data = b''
        
except:	
    print("[*] Closing socket")	
    s.close()