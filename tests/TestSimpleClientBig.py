#!/usr/bin/python

import socket
import time
import threading
from threading import Thread

def TestConnect():
	try:
		thread_id = threading.current_thread().ident
		print hex(thread_id), "started"
		
		HOST = '127.0.0.1'    
		PORT = 10000          
		s = socket.socket(socket.AF_INET,  socket.SOCK_STREAM)
		s.connect((HOST, PORT))
	
		hello = "Hello"
		msg = "".join(['*'] * (2048  - len(hello)))
		msg += hello
		s.send(msg)
		print  hex(thread_id), "Sent message Hello"
	
		data = s.recv(2048)
		s.close()
		if data.find("OK") != -1:
			print  hex(thread_id), "Recieved OK" #, repr(data)
		else:
			print hex(thread_id), "Recieved ERROR"
	
		print hex(thread_id), " finished"
	except Exception, e:
		print e


# start here
for i in range(100):
	t = Thread(target=TestConnect)
	t.start()
