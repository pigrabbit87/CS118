#!/usr/bin/python

"""
Output lines selected randomly from a file

Copyright 2005, 2007 Paul Eggert.
Copyright 2010 Darrell Benjamin Carbajal.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Please see <http://www.gnu.org/licenses/> for a copy of the license.

$Id: randline.py,v 1.4 2010/04/05 20:04:43 eggert Exp $
"""

import random, sys, string
from optparse import OptionParser

class randline:
    def __init__(self, filename):
        f = open(filename, 'r')
        self.lines = f.readlines()
        f.close()

    def chooseline(self):
        return random.choice(self.lines)    


def main():
    version_msg = "%prog 2.0"
    usage_msg = """%prog [OPTION]... FILE
    
Output randomly selected lines from combining FILEs if more than one.
-n option outputs number of lines
-u option delete duplicates. 
-w option limit output number of duplicated items
"""
   

    parser = OptionParser(version=version_msg,
                          usage=usage_msg)
    parser.add_option("-n", "--numlines",
                      action="store", dest="numlines", default=1,
                      help="output NUMLINES lines (default 1)")
    parser.add_option("-u", "--unique", action="store_true", dest="unique")
    parser.add_option("-w", "--without-replacement", 
                      action="store_true", dest="placement")
    options, args = parser.parse_args(sys.argv[1:])

    try:
        numlines = int(options.numlines)
    except:
        parser.error("invalid NUMLINES: {0}".
                     format(options.numlines))
    if numlines < 0:
        parser.error("negative count: {0}".
                     format(numlines))
    if len(args) < 1:
        parser.error("wrong number of operands")
   
    f = open("complete.txt", "w")
    f.close()
 
    for i in args: #compile all files into one big file
        fh = open(i, "r")
        with open("complete.txt","a") as myfile:
            myfile.write(fh.read())
    input_file = "complete.txt"
    #check how many lines there are in the combined file
    f = open("complete.txt", "r")    
    lines = f.readlines()
    num = len(lines)
    f.close()

    if options.unique == True or options.placement == True: 
#if there is option -u or -w
    #check if there are more lines in the file than newline
        if numlines > num and options.placement == True:
            parser.error("too many numlines")

        my_set = set()
        result = open("unique.txt", "w")
        for line in open("complete.txt", "r"):
                if line not in my_set:
                        result.write(line)
                        my_set.add(line)
        result.close()
        #the unique.txt file has non-duplicated entry
        input_file = "unique.txt"



#make two list for the -w options
    f = open("complete.txt", 'r')
    u = open("unique.txt", 'r')
    my_original = f.readlines()
    my_unique = u.readlines()
    my_count = []
    f.close()
    u.close()

    for i in my_unique:
        if options.unique == True:
            my_count.append(1)
        else:
            my_count.append(my_original.count(i)) 
    
    try:
        generator = randline(input_file)
        for index in range(numlines):
            if options.placement == True:        
                count = 0
                while True:
                    result = generator.chooseline()
                    index = my_unique.index(result)
                    if my_count[index] > 0:
                        sys.stdout.write(result)
                        my_count[index] = my_count[index]-1
                        break
                    count = count + 1
                    if count == num and options.unique == True:
                        #print('error, numline exceeds number of unique items')
                        break
            else:
                sys.stdout.write(generator.chooseline())
    except IOError as xxx_todo_changeme:
        (errno, strerror) = xxx_todo_changeme.args
        parser.error("I/O error({0}): {1}".
                     format(errno, strerror))

if __name__ == "__main__":
    main()


