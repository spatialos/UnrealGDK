import socket 
import sys 

def main():
    is_connected = False
    while not is_connected:
        try:
            print("Attempting connection to 127.0.0.1:1981 [server worker]")
            worker_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            worker_connection.connect(("127.0.0.1", 1981))
            worker_connection.settimeout(1.0)
            is_connected = True
        except socket.error:
            pass
            
    print("Connecting to 127.0.0.1:1980 [Insights]")
    proxy_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    proxy_connection.connect(("127.0.0.1", 1980))
    
    print("Waiting for target..")

    while True:
        try:
            chunk = worker_connection.recv(16*1024*1024)
        except socket.timeout:
            continue
        print "Forwarding " + str(len(chunk))
        if chunk == b'':
            print("Connection dropped, exiting.")
            sys.exit(0)
        proxy_connection.send(chunk)
        
try:
    main()
except KeyboardInterrupt:
    print("Exiting.")
    sys.exit(0)
