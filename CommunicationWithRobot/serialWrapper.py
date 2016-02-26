import serial
import time
import packetParser


class SerialWrapper(object) :

	def __init__ (self, portName):		
		self.initializePort(portName)
		self.name = self.SerialPort.name
		
	def initializePort(self, portName):		
		self.SerialPort = serial.Serial(portName, baudrate = 9600, timeout = 1)

	# send data without checking answer
	def sendData(self, dataToSend):
		self.SerialPort.write(dataToSend)

	def recieveData(self):
		return self.SerialPort.readline()

	# send data and recieve answer
	def sendRequest(self, dataToSend, timeout = 5):	
		# read all previous answers (clean buffer)
		self.SerialPort.readline()
		self.SerialPort.write(dataToSend)
		sendRequestTime = time.time()
		requestRecieved = False
		recievedPacket = ""
		
		while (requestRecieved == False and (time.time() - sendRequestTime) < timeout):
			recievedData = self.SerialPort.readline()
			if len(recievedData) is not 0:
				recievedPacket = packetParser.ParsePacket(recievedData)
				requestRecieved = True

		if recievedPacket == "" or requestRecieved == False:
				raise Exception("Reply was not recieved")			

		return recievedPacket


	def __del__(self):		
		self.SerialPort.close()