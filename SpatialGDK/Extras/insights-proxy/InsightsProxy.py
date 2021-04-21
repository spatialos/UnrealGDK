# EXPERIMENTAL: We do not support this functionality currently: Do not use it unless you are Improbable staff.

import socket 
import sys 

TARGET_IP = "127.0.0.1"
LOCAL_IP = "127.0.0.1"
TARGET_PORT = 1981 # Our listen port, see Control.cpp for target usage (C:/work/dev/UnrealEngine4.26/Engine/Source/Runtime/TraceLog/Private/Trace/Control.cpp)
INSIGHTS_PORT = 1980 # See 

def main():
    while True:
        try:
            print("Attempting connection to server worker %s:%s" % (TARGET_IP, TARGET_PORT))
            worker_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            worker_connection.connect((TARGET_IP, TARGET_PORT))
            worker_connection.settimeout(1.0)
            break
        except socket.error:
            print("Connecting failed. Is the port-foward to the server worker active and listening?")
            
    print("Connecting to the running Insights instance %s:%s" % (LOCAL_IP, INSIGHTS_PORT))
    proxy_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    proxy_connection.connect((LOCAL_IP, INSIGHTS_PORT))
    
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
