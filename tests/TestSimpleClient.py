#!/usr/bin/python

import socket
import time
import threading
from threading import Thread

def TestConnect():
	print hex(threading.current_thread().ident), " started"
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
	print hex(threading.current_thread().ident), " finished"


# start here
for i in range(100):
	t = Thread(target=TestConnect)
	t.start()
