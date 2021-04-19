import socket 
import sys 

TARGET_IP = "127.0.0.1"
TARGET_PORT = 1981
INSIGHTS_PORT = 1980 

def main():
    while True:
        try:
            print("Attempting connection to server worker %s:%s" % (TARGET_IP, TARGET_PORT))
            worker_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            worker_connection.connect((TARGET_IP, TARGET_PORT))
            worker_connection.settimeout(1.0)
            break
        except socket.error:
            pass
            
    print("Connecting to the running Insights instance 127.0.0.1:%s" % INSIGHTS_PORT)
    proxy_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    proxy_connection.connect(("127.0.0.1", INSIGHTS_PORT))
    
    print("Waiting for target..")

    while True:
        try:
            chunk = worker_connection.recv(16*1024*1024)
        except socket.timeout:
            continue
        print("Forwarding " + str(len(chunk)))
        if chunk == b'':
            print("Connection dropped, exiting.")
            sys.exit(0)
        proxy_connection.send(chunk)
        
try:
    main()
except KeyboardInterrupt:
    print("Received `KeyboardInterrupt`, exiting.")
    sys.exit(0)
