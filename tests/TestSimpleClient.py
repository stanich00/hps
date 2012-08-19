#!/usr/bin/python

import socket
import time

HOST = '127.0.0.1'    
PORT = 10000          
s = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
s.connect((HOST, PORT))

msg = "Hello"
s.send(msg)
print "Sent: ", msg

data = s.recv(1024)
s.close()
print "Recieved: ", repr(data)
