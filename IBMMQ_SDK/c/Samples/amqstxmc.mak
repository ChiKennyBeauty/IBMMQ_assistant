#/********************************************************************/
#/*                                                                  */
#/* Makefile Name: amqstxmc.mak                                      */
#/*                                                                  */
#/* Description: Makefile - IBM MQ Client sample for Tuxedo          */
#/*                                                                  */
#/* Environment: Windows with Microsoft Visual C++                   */
#/*              Makefile to be run as an external project makefile. */
#/*                                                                  */
#/*   <copyright                                                     */
#/*   notice="lm-source-program"                                     */
#/*   pids="5724-H72,5724-B41,"                                      */
#/*   years="2002,2016"                                              */
#/*   crc="753331452" >                                              */
#/*   Licensed Materials - Property of IBM                           */
#/*                                                                  */
#/*   5724-H72,5724-B41,                                             */
#/*                                                                  */
#/*   (C) Copyright IBM Corp. 2002, 2016 All Rights Reserved.        */
#/*                                                                  */
#/*   US Government Users Restricted Rights - Use, duplication or    */
#/*   disclosure restricted by GSA ADP Schedule Contract with        */
#/*   IBM Corp.                                                      */
#/*   </copyright>                                                   */
#/*                                                                  */
#/********************************************************************/
#
#/********************************************************************/
#/* Required Environment variables:                                  */
#/*    TUXDIR=<TUXDIR (see below)>                                   */
#/*    TUXCONFIG=<APPDIR (see below)>\tuxconfig                      */
#/*    FIELDTBLS=<MQMDIR (see below)>\tools\c\samples\amqstxvx.fld   */
#/*    LANG=C                                                        */
#/********************************************************************/
#
#/********************************************************************/
#/* Modify following paths to match your installation:               */
#/*  TUXDIR is the directory specified when installing TUXEDO.       */
#/*  MQMDIR is the directory specified when installing IBM MQ.       */
#/*  APPDIR is the directory into which the sample program is built. */
#/********************************************************************/
#/* NOTES:                                                           */
#/* 1) APPDIR should be the current directory when the makefile is   */
#/*    invoked, since viewc creates amqstxvx.h in current directory. */
#/* 2) APPDIR should contain a customised UBB file, ubbstxcn.cfg     */
#/* 3) The RM file in <TUXDIR>\udataobj requires the following       */
#/*    statement (as one line and replace <MQMDIR>):                 */
#/*  MQSeries_XA_RMI;MQRMIXASwitchDynamic;                           */
#/*     <MQMDIR>\tools\lib\mqctux.lib <MQMDIR>\tools\lib\mqic.lib    */
#/********************************************************************/
TUXDIR  = f:\tuxedo
MQMDIR  = g:\mqm
APPDIR  = f:\tuxedo\apps\mqapp
#/********************************************************************/
#/*                                                                  */
#/********************************************************************/
MQMLIB  = $(MQMDIR)\tools\lib
MQMINC  = $(MQMDIR)\tools\c\include
MQMSAMP = $(MQMDIR)\tools\c\samples
INC = -f "-I$(MQMINC) -I$(APPDIR)"
DBG = -f "/Zi"
amqstxc.exe:
 $(TUXDIR)\bin\mkfldhdr    -d$(APPDIR) $(MQMSAMP)\amqstxvx.fld
 $(TUXDIR)\bin\viewc       -d$(APPDIR) $(MQMSAMP)\amqstxvx.v
 $(TUXDIR)\bin\buildtms    -o MQXA -r MQSeries_XA_RMI
 $(TUXDIR)\bin\buildserver -o MQSERV1 -f $(MQMSAMP)\amqstxsx.c        \
                           -f $(MQMLIB)\mqic.lib -v $(INC) $(DBG)     \
                           -r MQSeries_XA_RMI                         \
                           -s MPUT1:MPUT -s MGET1:MGET
 $(TUXDIR)\bin\buildserver -o MQSERV2 -f $(MQMSAMP)\amqstxsx.c        \
                           -f $(MQMLIB)\mqic.lib -v $(INC) $(DBG)     \
                           -r MQSeries_XA_RMI                         \
                           -s MPUT2:MPUT -s MGET2:MGET
 $(TUXDIR)\bin\buildclient -o doputs -f $(MQMSAMP)\amqstxpx.c         \
                           -f $(MQMLIB)\mqic.lib -v $(INC) $(DBG)
 $(TUXDIR)\bin\buildclient -o dogets -f $(MQMSAMP)\amqstxgx.c         \
                           -f $(MQMLIB)\mqic.lib $(INC) -v $(DBG)
 $(TUXDIR)\bin\tmloadcf    -y $(APPDIR)\ubbstxcn.cfg
