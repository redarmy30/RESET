import serialWrapper 
import packetBuilder
import packetParser
from serial.tools import list_ports
import sys
import time

# STM32 USB microcontroller ID
VID = '0483'
PID = '5740'
SNR = '338434693534'

def initPTC():
	"""Initialize PID, Trajectory, Kinematics"""
	# Build packet for sending to robot	
	packet = packetBuilder.BuildPacket(commands.switchOnPid)
		
	# send packet to port. sendRequest method will wait answer from robot. In case 
	# if you don't need answer possible to use 'sendData' method (just send data, 
	# without waiting for answer)
	startT = time.time()	
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	endT = time.time()
	print 'Recieved ', (endT - startT)
	if recievedPacket.reply == 'Ok':
		print 'PID controller On'		
	else:
		raise Exception('switchOnPid failed')
	
	packet = packetBuilder.BuildPacket(commands.switchOnTrajectoryRegulator)
	startT = time.time()	
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	endT = time.time()
	print 'Recieved ', (endT - startT)
	if recievedPacket.reply == 'Ok':
		print 'Trajectory regulator ON'
	else:
		raise Exception('switchOnTrajectoryRegulator failed')
	
	packet = packetBuilder.BuildPacket(commands.switchOnKinematicCalculation)
	startT = time.time()
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	endT = time.time()
	print 'Recieved ', (endT - startT)
	if recievedPacket.reply == 'Ok':
		print 'Kinematics ON'
	else:
		raise Exception('switchOnKinematicCalculation failed')

def port_number():
	"""Find all ports, and returns one with defined STM values"""	
	for port in list_ports.comports():
		if port[2] == 'USB VID:PID=%s:%s SNR=%s' %(VID,PID,SNR):	
			return port[0]

def movement():
	print '\nInput coordinates and speed type'	
	# Get user input for movement
	x = float(raw_input('X: '))
	y = float(raw_input('Y: '))
	fi = float(raw_input('angle: '))
	speed = int(raw_input('speed type (1 = normal, 2 = stop, 3 = stand): '))
	coordinates = [x, y, fi, speed]
	print 'Movement command: ', coordinates
	
	packet = packetBuilder.BuildPacket(commands.addPointToStack, coordinates)
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	
	if recievedPacket.reply != 'Ok':
		raise Exception('add PointToStack failed')
	print 'Wait 5 seconds timeout (robot is moving)....'
	time.sleep(5)
	print 'Get current coodrdinates:'
	packet = packetBuilder.BuildPacket(commands.getCurentCoordinates)
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	
def setCoord():
	print '\nSet current robot coordinates'	
	x = float(raw_input('X: '))
	y = float(raw_input('Y: '))
	fi = float(raw_input('angle: '))	
	coordinates = [x, y, fi]

	packet = packetBuilder.BuildPacket(commands.setCoordinates, coordinates)
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	if recievedPacket.reply != 'Ok':
		raise Exception('setCoordinates failed')

def getCoord():
	"""Return robot current coordinates"""		
	packet = packetBuilder.BuildPacket(commands.getCurentCoordinates)
	startT = time.time()	
	recievedPacket = computerPort.sendRequest(packet.bytearray)
	endT = time.time()
	print 'GetCoord time: ', (endT - startT)	
	print 'Current robot coordinates: ', recievedPacket.reply

port = port_number()
if port:
	print 'STM32 found on port %s' %port
else:
	print 'No STM32 found. Aborting'
	sys.exit()

# COM port initialization 
computerPort = serialWrapper.SerialWrapper(port)

# we will choose commands which we want to send from this list
commands = packetBuilder.CommandsList()

# Initialize PID, Trajectory and Kinematics
initPTC()
iteration = 0
while True:
	iteration += 1	
	print '\nList of available commands: \n1 Movement\n2 Get Coordinates',\
			'\n3 Set Coordinates' 	
	#command = raw_input('Command number: ')
	#if command == '1':
	#	movement()
	#elif command == '2':
	#	getCoord()
	#elif command == '3':
	#	setCoord()	
	getCoord()
	print 'Iteration: ', iteration



	
	
	

	


