#!/usr/bin/python
import os
import sys

def main(argv):
   try:
     fname = argv[1]
   except IndexError:
     sys.stderr.write("ERR: Must provide an input file name.\n")
     return 0
   basename = os.path.splitext(fname)[0]
   # Generates a .s and a .ll just for show
   cmd = "llc " + fname
   os.system(cmd)
   print cmd
   cmd = "llvm-dis < " + fname + " > " + basename + ".ll"
   os.system(cmd)
   print cmd
   # Generates a .o from the input file
   cmd = "llc -filetype=obj " + fname
   os.system(cmd)
   print cmd
   # Generates an executable
   cmd = "gcc " + basename + ".o -o " + basename
   os.system(cmd)
   print cmd

if __name__ == "__main__":
  sys.exit(main(sys.argv))
