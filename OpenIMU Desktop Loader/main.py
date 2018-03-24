#!/usr/bin/python

import struct
import sys
import binascii

def processChunk(chunk):
    data = struct.unpack("<9f", chunk)
    for value in data:
        print(value)
        pass
    print("")

def readDataFile(file):
    while(1):
        chunk = file.read(1)
        if(len(chunk) < 1):
            print("Reached end of file")
            break
        headChar = struct.unpack("c", chunk)
        if(headChar[0] == 'h'):
            print("\nNew log stream\n")
        elif(headChar[0] == 'd'):
                chunk = file.read(36)
                processChunk(chunk)
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
