import socket
import sys

HOST, PORT = "os-class.engr.oregonstate.edu", 50001

# Create a socket (SOCK_STREAM means a TCP socket)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

while(1):
    # Connect to server and send data
    data = raw_input("What info you got?")
    sock.send(data + "\n")

    # Receive data from the server and shut down
    received = sock.recv(1024)
    print "Sent:     %s" % data
    print "Received: %s" % received

sock.close()
