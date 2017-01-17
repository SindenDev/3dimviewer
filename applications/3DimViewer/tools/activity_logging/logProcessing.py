#!/usr/bin/python3

import sys
import re
import string
import xml.etree.ElementTree as ET
import getopt
import math
import codecs
import time
import png

# global variables
inputFile = ""
outputFile = ""
parseAll = True
timeFrom = ""
timeTo = ""
objectNames = ""
eventTypes = ""
actionNames = ""
objectClasses = ""
pathSubstr = ""
textSubstr = ""
timeFromMs = 0
timeToMs = 0
tree = 0
root = 0

windowWidth = 0;
windowHeight = 0;

clickCnt = 0
dblClickCnt = 0
leftDblClickCnt = 0
rightDblClickCnt = 0
middleDblClickCnt = 0
leftClickCnt = 0
rightClickCnt = 0
middleClickCnt = 0
dragLength = 0
leftDragLength = 0
rightDragLength = 0
middleDragLength = 0

mat = 0

# Print help
def help():
	print("\nlogProcessing.py -i <inputFilePath/inputFileName.xml> [-o <outputFilePath/outputFileName.xml>] [-f <hh:mm:ss>] [-t <hh:mm:ss>] [-e <event1,event2,...>] [-a <action1,action2,...>] [-n <object1,object2,...>]\n\n\
  -i    Full path to input xml file.\n\
  -o    Full path to output xml file, that will be created.\n\
  -f    Beginning of the restrictive time interval.\n\
  -t    End of the restrictive time interval.\n\
  -e    Names of events, which should be processed, separated by ','. If not set, all events will be processed.\n\
        Possible values: mouseevent, wheelevent, keyboardevent, specialevent, customevent\n\
  -a    Actions, which should be processed, separated by ','. Works only for mouseevent and wheelevent. If not set, all actions will be processed.\n\
        Possible values: (click, doubleclick, drag, menuclick) - for mouseevent; (rollup, rolldown) - for wheelevent \n\
  -n    Names of the objects, which should be processed, separated by ','.\n\
  -c    Names of object classes, which should be processed, separated by ','.\n\
  -p    String, which will be searched in object path.\n\
  -s    String, which will be searched in object text.\n\
  ")

# Process console parametrs
def processParameters(argv):
	
	global inputFile
	global outputFile
	global parseAll
	global timeFrom
	global timeTo
	global timeFromMs
	global timeToMs
	global objectNames
	global eventTypes
	global actionNames
	global objectClasses
	global pathSubstr
	global textSubstr
	
	try:
		opts, args = getopt.getopt(argv,"hi:o:f:t:e:a:n:c:p:s:",["ifile=","ofile=","from=","to=","events=","actions=","objects=","classes=","path=","text="])
	except getopt.GetoptError:
		help()
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			help()
			sys.exit()
		elif opt in ("-i", "--ifile"):
			inputFile = arg
		elif opt in ("-o", "--ofile"):
			outputFile = arg
		elif opt in ("-f", "--from"):
			timeFrom = arg
		elif opt in ("-t", "--to"):
			timeTo = arg
		elif opt in ("-e", "--events"):
			eventTypes = arg
		elif opt in ("-a", "--actions"):
			actionNames = arg	
		elif opt in ("-n", "--objects"):
			objectNames = arg	
		elif opt in ("-c", "--classes"):
			objectClasses = arg	
		elif opt in ("-p", "--path"):
			pathSubstr = arg
		elif opt in ("-s", "--text"):
			textSubstr = arg	
		
	# some non-optional arguments missing	
	if (inputFile == ""):	
		help()
		sys.exit(2)
		
	if (outputFile == ""):
		outputFile = "out.xml"
		
	# process given time interval (convert times to miliseconds)
	if ((timeFrom != "") | (timeTo != "")):
		parseAll = False
		
	if (timeFrom != ""): 
		timeFrom += ":000"  
	
	if (timeTo != ""):
		timeTo += ":000"

# Parse one event (xml element)
def parseEvent(event):
	
	global objectNames
	global eventTypes
	global actionNames
	global tree
	global root
	global clickCnt
	global leftClickCnt
	global rightClickCnt
	global middleClickCnt
	global dblClickCnt
	global leftDblClickCnt
	global rightDblClickCnt
	global middleDblClickCnt
	global dragLength
	global leftDragLength
	global rightDragLength
	global middleDragLength
	global objectClasses
	global windowWidth
	global windowHeight
	global mat
	global pathSubstr
	global textSubstr
	
	# remove event with wrong type
	if ((eventTypes != "") & (eventTypes.find(event.get("type")) == -1)):
		root.remove(event)
		return

	# remove event with wrong action
	if ((actionNames != "") & ((event.get("type") == "mouseevent") | (event.get("type") == "wheelevent"))):
		try:
			actionNames.split(",").index(event.find("action").text)
		except ValueError:
			root.remove(event)
			return
			
	# remove event with wrong objectClass
	if (objectClasses != ""):
		if (event.find("objectClass").text == None):
			root.remove(event)
			return
		if (objectClasses.find(event.find("objectClass").text) == -1):
			root.remove(event)
			return

	# remove event with wrong objectName
	if (objectNames != ""):
		if (event.find("objectName").text == None):
			root.remove(event)
			return
		if (objectNames.find(event.find("objectName").text) == -1):
			root.remove(event)
			return
	
	# remove event with wrong objectPath		
	if (pathSubstr != ""):
		if (event.find("objectPath").text == None):
			root.remove(event)
			return
		if ((event.find("objectPath").text).find(pathSubstr) == -1):
			root.remove(event)
			return
	
	# remove event with wrong objectText		
	if (textSubstr != ""):
		if (event.find("objectText").text == None):
			root.remove(event)
			return
		if ((event.find("objectText").text).find(textSubstr) == -1):
			root.remove(event)
			return
	
	# count mouse clicks, double click and drag length		
	if (event.get("type") == "mouseevent"):
		if (event.find("action").text == "click"):
			clickCnt += 1
			x, y = event.find("position").text.split(":")				
			mat[int(y)][int(x)*2] += 1;
			if (event.find("mouseButton").text == "left"):
				leftClickCnt += 1
			elif (event.find("mouseButton").text == "right"):
				rightClickCnt += 1
			elif (event.find("mouseButton").text == "middle"):
				middleClickCnt += 1			
		elif (event.find("action").text == "doubleclick"):
			dblClickCnt += 1
			x, y = event.find("position").text.split(":")
			mat[int(y)][int(x)*2] += 1;
			if (event.find("mouseButton").text == "left"):
				leftDblClickCnt += 1
			elif (event.find("mouseButton").text == "right"):
				rightDblClickCnt += 1
			elif (event.find("mouseButton").text == "middle"):
				middleDblClickCnt += 1
		elif (event.find("action").text == "drag"):
			startX, startY = event.find("startPosition").text.split(":")
			endX, endY = event.find("endPosition").text.split(":")
			distance = math.sqrt(math.pow((int(endX) - int(startX)), 2) + math.pow((int(endY) - int(startY)), 2))
			dragLength += distance
			if (event.find("mouseButton").text == "left"):
				leftDragLength += distance
			elif (event.find("mouseButton").text == "right"):
				rightDragLength += distance
			elif (event.find("mouseButton").text == "middle"):
				middleDragLength += distance

# Load input xml file and parse it
def parseFile():
	
	global inputFile
	global outputFile
	global parseAll
	global timeFrom
	global timeTo
	global timeFromMS
	global timeToMs
	global tree
	global root
	global windowWidth
	global windowHeight
	global mat
	
	inFile = codecs.open(inputFile, mode='r')
	
	# parse file
	tree = ET.parse(inFile)

	# get root element
	root = tree.getroot()
	
	# get window size
	width, height = root.get("windowSize").split(":")
	windowWidth = int(width)
	windowHeight = int(height)
	
	# create matrix which is 2*wider than window (alpha channel)
	mat = [[0 for x in range(windowWidth*2)] for y in range(windowHeight)] 
	
	# find all events
	events = root.findall("event")
	
	# get start and end time
	start = (timeFrom if (timeFrom != "") else events[0].find("time").text)
	end = (timeTo if (timeTo != "") else events[len(events)-1].find("time").text)
	
	# convert it to miliseconds
	hFrom, mFrom, sFrom, msFrom = start.split(":")
	timeFromMs = int(hFrom) * 3600000 + int(mFrom) * 60000 + int(sFrom) * 1000 + int(msFrom)
	
	hTo, mTo, sTo, msTo = end.split(":")	
	timeToMs = int(hTo) * 3600000 + int(mTo) * 60000 + int(sTo) * 1000 + int(msTo)
		
	# count duration and convert it to readeable string
	durationMs = timeToMs - timeFromMs
	
	duration = str(int(durationMs/3600000)) if ((durationMs/3600000) > 9) else ("0" + str(int(durationMs/3600000)))
	durationMs -= int(durationMs/3600000) * 3600000
	duration += str(":" + str(int(durationMs/60000))) if ((durationMs/60000) > 9) else (":0" + str(int(durationMs/60000)))
	durationMs -= int(durationMs/60000) * 60000
	duration += str(":" + str(int(durationMs/1000))) if ((durationMs/1000) > 9) else (":0" + str(int(durationMs/1000)))
	durationMs -= int(durationMs/1000) * 1000
	duration += str(":" + str(durationMs))
	
	# print all arguments - it is here, because i need to get start and end time from file (if it is not set)
	print("\nParameters:")
	print(" Input file: " + inputFile)
	print(" Output file: " + outputFile)
	print(" Start time: " + start)
	print(" End time: " + end)
	print(" Duration: " + str(duration))
	print(" Event types: " + eventTypes)
	print(" Action names: " + actionNames)
	print(" Object names: " + objectNames)
	print(" Object classes: " + objectClasses)
	print(" Text substring: " + textSubstr)
	print(" Path substring: " + pathSubstr)
	print("\n")

	# go thru all events
	for event in events:
		# no time interval given, process all events
		if (parseAll):
			parseEvent(event)
		else:
			# get event time and convert it to miliseconds
			h, m, s, ms = event[0].text.split(":")
			time = int(h) * 3600000 + int(m) * 60000 + int(s) * 1000 + int(ms)
			
			# remove events that are not in given time interval, process the others
			if (timeTo == ""):
				if ((timeFrom != "") & (time < timeFromMs)):
					root.remove(event)
				elif ((timeFrom != "") & (time >= timeFromMs)):
					parseEvent(event)
			elif (timeFrom == ""):
				if ((timeTo != "") & (time > timeToMs)):
					root.remove(event)
				elif ((timeTo != "") & (time <= timeToMs)):
					parseEvent(event)
			elif ((time < timeFromMs) | (time > timeToMs)):
				root.remove(event)
			elif ((time >= timeFromMs) & (time <= timeToMs)):
				parseEvent(event)

# Create png image with heat map
def createHeatMap():
	global mat
	
	maxVal = 0;
		
	newMat = [[0 for x in range(windowWidth*2)] for y in range(windowHeight)] 
	
	# copy value in matrix (which is bigger than 0) to 3x3 neighborhood (for better visibility in final image)
	for x in range(1, windowHeight-1):
		for y in range(2, windowWidth*2, 2):
			val = mat[x][y]
			if (val > 0):
				for k in range(-1, 2):
					for j in range(-2, 3, 2):
						if (mat[x+k][y+j] <= val):
							newMat[x+k][y+j] = val
					
			
	# get max click count
	for x in range(windowHeight):
		for y in range(0, windowWidth*2, 2):
			if (newMat[x][y] > maxVal):
				maxVal = newMat[x][y]
	
	# convert click counts to intensity value
	if (maxVal != 0):	
		for x in range(windowHeight):
			for y in range(0, windowWidth*2, 2):
				newMat[x][y] *= int(255/maxVal)
				if (newMat[x][y] > 0):
					newMat[x][y+1] = 255;
	
	# save image
	png.from_array(newMat, 'LA').save("heat_map.png")

# Main	
def main(argv):
	
	processParameters(argv)
	parseFile()
	createHeatMap()
	
	# write modified xml to output file
	tree.write(outputFile)
	
	# write statistics
	print("Clicks count: " + str(clickCnt))
	print(" Left mouse button clicks count: " + str(leftClickCnt))
	print(" Right mouse button clicks count: " + str(rightClickCnt))
	print(" Middle mouse button clicks count: " + str(middleClickCnt))
	print("\n")
	print("Doubleclicks count: " + str(dblClickCnt))
	print(" Left mouse button doubleclicks count: " + str(leftDblClickCnt))
	print(" Right mouse button doubleclicks count: " + str(rightDblClickCnt))
	print(" Middle mouse button doubleclicks count: " + str(middleDblClickCnt))
	print("\n")
	print("Drag length: " + str(dragLength))
	print(" Left mouse button drag length: " + str(leftDragLength))
	print(" Right mouse button drag length: " + str(rightDragLength))
	print(" Middle mouse button drag length: " + str(middleDragLength))

if __name__ == "__main__":
   main(sys.argv[1:])
