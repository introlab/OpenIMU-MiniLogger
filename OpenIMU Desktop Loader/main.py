#!/usr/bin/python

import struct
import sys
import binascii
import datetime

def processChunk(chunk):
    data = struct.unpack("<9f", chunk)
    for value in data:
        print(value)
    print("")

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
        elif(headChar[0] == 'd'):
                n = n + 1
                chunk = file.read(36)
                processChunk(chunk)
        elif(headChar[0] == 't'):
                print(n)
                n = 0
                chunk = file.read(4)
                timestamps = struct.unpack("i", chunk)
                for timestamp in timestamps:
                    print(datetime.datetime.utcfromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S')
)


        else:
            print("\nUnrecognised chunk\n")


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
