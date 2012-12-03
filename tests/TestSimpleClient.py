#!/usr/bin/python

import socket
import time
import threading
from threading import Thread

def TestConnect():
	thread_id = threading.current_thread().ident
	print hex(thread_id), " started"
	
	HOST = '127.0.0.1'    
	PORT = 10000          
	s = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
	s.connect((HOST, PORT))

	msg = "Hello"
	s.send(msg)
	print  hex(thread_id), " Sent: ", msg

	data = s.recv(1024)
	s.close()
	print  hex(thread_id), " Recieved: ", repr(data)

	print hex(thread_id), " finished"


# start here
for i in range(100):
	t = Thread(target=TestConnect)
	t.start()
