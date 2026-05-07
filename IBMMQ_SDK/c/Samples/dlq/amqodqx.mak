#
# Name:          AMQODQX.MAK
#
# Description:   Example Windows NT makefile for the dlq handler sample. 
#                Note this is the makefile for use with the Microsoft Visual C++
#                compiler.
#
#   <copyright 
#   notice="lm-source-program" 
#   pids="5724-H72," 
#   years="2000,2021" 
#   crc="4193376015" > 
#   Licensed Materials - Property of IBM  
#    
#   5724-H72, 
#    
#   (C) Copyright IBM Corp. 2000, 2021 All Rights Reserved.  
#    
#   US Government Users Restricted Rights - Use, duplication or  
#   disclosure restricted by GSA ADP Schedule Contract with  
#   IBM Corp.  
#   </copyright> 
#
#
# To make the dlq handler sample, run the command:-
#                nmake -f amqodqx.mak all
#

#The name of the compiler to use
CC=cl.exe
LL=link.exe
CCARGS=/c /I. /DAMQ_NT /W3

# The libraries to include
LDLIBS=MQM.lib
CLDLIBS=MQIC.lib
OBJS=amqodqka.obj amqodqla.obj amqodqma.obj amqodqna.obj amqodqoa.obj amqodqpa.obj amqodqua.obj
HDEPS=amqodqha.h amqodkha.h amqodtha.h 

all: amqsdlq.exe amqsdlqc.exe

amqsdlq.exe: $(OBJS)
        $(LL) /OUT:amqsdlq.exe $(OBJS) $(LDLIBS)

amqsdlqc.exe: $(OBJS)
        $(LL) /OUT:amqsdlqc.exe $(OBJS) $(CLDLIBS)

# The mainline routine
amqodqka.obj: amqodqka.c $(HDEPS)
              $(CC) $(CCARGS) $*.c
 
# The queue browser
amqodqla.obj: amqodqla.c $(HDEPS)   
              $(CC) $(CCARGS) $*.c
 
# The rule table parser
amqodqma.obj: amqodqma.c $(HDEPS)   
              $(CC) $(CCARGS) $*.c
 
# The pattern matching code
amqodqna.obj: amqodqna.c $(HDEPS)   
              $(CC) $(CCARGS) $*.c

# The message history management
amqodqoa.obj: amqodqoa.c $(HDEPS)   
              $(CC) $(CCARGS) $*.c
 
# The rule table lexer
amqodqpa.obj: amqodqpa.c $(HDEPS)  
              $(CC) $(CCARGS) $*.c

# Common services (message, trace, etc )
amqodqua.obj: amqodqua.c $(HDEPS)   
              $(CC) $(CCARGS) $*.c

