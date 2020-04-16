#!/usr/bin/python

import struct
import sys
import binascii
import datetime
import string


def processImuChunk(chunk):
    data = struct.unpack("9f", chunk)
    for value in data:
        #print(value)
        pass
    print("imu")


def processTimestampChunk(chunk):
    timestamps = struct.unpack("i", chunk)
    for timestamp in timestamps:
        print(datetime.datetime.utcfromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S'))


def processGPSChunk(chunk):
    data = struct.unpack("?3f", chunk)
    fix = data[0]
    if fix:
        print(string.format("GPS FIX\t%2.6f\t%2.6f\t%3.1f"), data[1], data[2], data[3])
    else:
        print("No gps fix")


def readDataFile(file):
    n = 0
    while(1):
        chunk = file.read(1)
        if(len(chunk) < 1):
            print("Reached end of file")
            break
        headChar = struct.unpack("c", chunk)
        if(headChar[0] == 'h'):
            print("\nNew log stream\n")
        elif(headChar[0] == 'i'):
                n = n + 1
                chunk = file.read(struct.calcsize("9f"))
                processImuChunk(chunk)
        elif(headChar[0] == 't'):
                print(n)
                n = 0
                chunk = file.read(struct.calcsize("i"))
                processTimestampChunk(chunk)
        elif(headChar[0] == 'g'):
            chunk = file.read(struct.calcsize("?3f"))
            processGPSChunk(chunk)

        else:
            print("\nUnrecognised chunk\n")
            print(headChar[0])
            break


def main():
    if(len(sys.argv) != 2):
        print("\nPlease provide .oimu filename\n")

    else:
        print("\nLoading " + sys.argv[1] + "\n")
        with open(sys.argv[1], "rb") as file:
            print("\nLoading file\n")
            readDataFile(file)

        file.close()

if __name__ == "__main__":
    main()
