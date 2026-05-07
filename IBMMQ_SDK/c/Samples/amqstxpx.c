/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/oltp/amqstxpx.c */
/*********************************************************************/
/*                                                                   */
/* Module Name: amqstxpx.c                                           */
/*                                                                   */
/* Description: IBM MQ sample Put transaction for Tuxedo             */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2016"                                               */
/*   crc="4000417301" >                                              */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72,                                                       */
/*                                                                   */
/*   (C) Copyright IBM Corp. 1994, 2016 All Rights Reserved.         */
/*                                                                   */
/*   US Government Users Restricted Rights - Use, duplication or     */
/*   disclosure restricted by GSA ADP Schedule Contract with         */
/*   IBM Corp.                                                       */
/*   </copyright>                                                    */
/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/* amqstxpx.c is a sample Tuxedo transaction working with IBM MQ.    */
/* It takes a number of arguments, allowing the user                 */
/* to place messages on a named queue, as follows                    */
/*      -m QMgrName     = Queue Manager to connect to                */
/*      -n QueueName    = Name of Queue to place messages on         */
/*      -b BatchSize    = Number of messages to put in each transn.  */
/*      -c TranCount    = Number of transactions to perform.         */
/*      -t MsgText      = Message Text                               */
/*                                                                   */
/* For build instructions please refer to the MQ documentation       */
/*                                                                   */
/* amqstxpx.c contains the following functions:                      */
/*  main                 main function entry point                   */
/*                                                                   */
/*********************************************************************/

/*********************************************************************/
/* Includes                                                          */
/*********************************************************************/

/*********************************************************************/
/* Select compiler specific header files                             */
/*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*********************************************************************/
/* include Tuxedo header files                                       */
/*********************************************************************/
#ifdef _TMFML32
#include <fml32.h>
#include <fml1632.h>
#else
#include <fml.h>
#endif
#include <atmi.h>
#include <Uunix.h>
#include <userlog.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
/*********************************************************************/
/* include MQ header files                                           */
/*********************************************************************/
#include <cmqc.h>

/*********************************************************************/
/* include local structure definitions                               */
/*********************************************************************/
#include "amqstxvx.h"

/*********************************************************************/
/* The main function                                                 */
/*********************************************************************/
#if defined(__STDC__) || defined(__cplusplus)
main(int argc, char *argv[])

#else

main(argc, argv)
int argc;
char *argv[];
#endif

{
	int c;                  /* Option character */
	int cflgs=0;            /* Commit flags, currently unused */
	int aflgs=0;            /* Abort flags, currently unused */
	long audrl;             /* return length on getrply */
	int anycd;              /* receives call descriptor */
	char svc_name[20];      /* service name */
	char *proc_name;        /* Process name */
	int rc;                 /* return value of sum_bal() */
	long LoopCt=1;          /* Looping Count */
	long BatchSz=1;         /* Batch Size */
	long NumSvrs=1;         /* Server Nums */
	struct amqstxvx *pMsg;  /* Message Buffer */
	char *MsgPtr = "Msg";   /* TestMessage */
	char *QName = "SYSTEM.DEFAULT.LOCAL.QUEUE";   /* QName */
	MQLONG CompCode;
	MQLONG Reason;
	long i;
	long j;
	int TranStarted = FALSE;

	/*************************************************************/
	/* Pick up the name of the program                           */
	/*************************************************************/
	if (strrchr(argv[0],'/') != NULL)
	        proc_name = strrchr(argv[0],'/')+1;
	else
	        proc_name = argv[0];

	/*************************************************************/
	/* Set up the service name that we wish to call              */
	/*************************************************************/
	memset(svc_name, 0, 20);
	(void)strcpy(svc_name,"MPUT");

	/*************************************************************/
	/* Pick up and parse the arguments                           */
	/*************************************************************/
	while((c = getopt(argc,argv,"b:t:n:c:s:.")) != EOF)
	        switch((char)c) {
	        case 'b':
	                BatchSz = atol(optarg);
	                break;
	        case 'c':
	                LoopCt = atol(optarg);
	                break;
	        case 't':
	                MsgPtr = optarg;
	                break;
	        case 'n':
	                QName = optarg;
	                break;
	        case 's':
	                NumSvrs = atol(optarg);
	                break;
	        default:
	                (void)fprintf(stderr,
	                      "%s:  usage %s [-n QName] "
	                      "[-b BatchSize] [-c Count] [-t Text]\n",
	                      proc_name, proc_name);
	                exit(2);
	        }

	/*************************************************************/
	/* Join the application                                      */
	/*************************************************************/
	if (tpinit((TPINIT *) NULL) == -1)
	{
	  (void)fprintf(stderr,
	        "Failed to join application, %s\n",
	        tpstrerror(tperrno));
	  (void)userlog("Failed to join application, %s\n",
	        tpstrerror(tperrno));
	  exit(1);
	}

	/*************************************************************/
	/* Get a buffer to talk to Tuxedo with                       */
	/*************************************************************/
	if ((pMsg = (struct amqstxvx *)
	               tpalloc(VIEWTYPE,
	                       "amqstxvx",
	                       sizeof(struct amqstxvx))) == NULL)
	{
	  (void)fprintf(stderr,
	        "Unable to allocate space for MSG, %s\n",
	        tpstrerror(tperrno));
	  (void)userlog("Unable to allocate space for MSG, %s\n",
	        tpstrerror(tperrno));
	  (void)tpterm();
	  exit(1);
	}

	/*************************************************************/
	/* Set up the message from the arguments                     */
	/*************************************************************/
	strncpy(pMsg->QName, QName, 48);
	strncpy(pMsg->Msg, MsgPtr, 2047);

	/*************************************************************/
	/* Connected OK so do top-level loop                         */
	/*************************************************************/
	for(i=0, rc=0; i<LoopCt && (rc == 0); i++)
	{
	  /***********************************************************/
	  /* Start the transaction                                   */
	  /***********************************************************/
	  if (tpbegin(30, 0) == -1)
	  {
	    (void)fprintf(stderr,
	          "Failed to begin transaction %d, %s\n",
	          i, tpstrerror(tperrno));
	    (void)userlog("Failed to begin transaction %d, %s\n",
	          i, tpstrerror(tperrno));
	    rc = 1;
	  }
	  else
	  {
	    TranStarted = TRUE;
	  }

	  /***********************************************************/
	  /* Issue all the calls required in this transaction        */
	  /***********************************************************/
	  for(j=0; j<BatchSz && (rc == 0); j++)
	  {
	    /*********************************************************/
	    /* Request the PUT1 call                                 */
	    /*********************************************************/
	    svc_name[4] = '1' + (j % NumSvrs);
	    if (tpacall(svc_name,
	                (char *)pMsg,
	                sizeof(struct amqstxvx),
	                0) == -1)
	    {
	      (void)fprintf(stderr,
	            "%s: %s service request failed on loop %d:%d %s\n",
	            proc_name, svc_name,
	            i, j, tpstrerror(tperrno));
	      rc = 1;
	      break;
	    }
	  };

	  /***********************************************************/
	  /* Field all the replies for the transaction               */
	  /***********************************************************/
	  for(j=0; j<BatchSz && (rc == 0); j++)
	  {
	    /*********************************************************/
	    /* Request the next reply                                */
	    /*********************************************************/
	    if (tpgetrply(&anycd,
	                  (char **)&pMsg,
	                  &audrl,
	                  TPGETANY) == -1)
	    {
	      /*******************************************************/
	      /* Handle Tuxedo return codes                          */
	      /*******************************************************/
	      if (tperrno != TPESVCFAIL)
	      {
	        (void)fprintf (stderr,
	              "service tpgetrply failed, %d:%d %s\n",
	              i, j, tpstrerror(tperrno));
	      }
	      else
	      {
	        (void)fprintf(stderr,
	              "%s: %s service routine failed on loop %d:%d, %s\n",
	              proc_name, svc_name, i, j,
	              tpstrerror(tperrno));
	      };
	      rc = 1;
	      break;
	    }
	    else
	    {
	      /*******************************************************/
	      /* Check the MQ retcode                                */
	      /*******************************************************/
	      Reason = pMsg->Reason;
	      if (Reason != MQRC_NONE)
	      {
	        (void)fprintf(stderr,
	              "%s: %s MQPUT failed on loop %d:%d, %d\n",
	              proc_name, svc_name, i, j, Reason);
	        rc = 1;
	      };
	    };
	  };

	  /***********************************************************/
	  /* If we started a transaction                             */
	  /***********************************************************/
	  if (TranStarted == TRUE)
	  {
	    /*********************************************************/
	    /* Reset the flag                                        */
	    /*********************************************************/
	    TranStarted = FALSE;

	    /*********************************************************/
	    /* Errors mean we should abort the transaction           */
	    /*********************************************************/
	    if (rc != 0)
	    {
	      (void) tpabort(aflgs);
	    }
	    else
	    {
	      /*******************************************************/
	      /* Otherwise commit the transaction                    */
	      /*******************************************************/
	      if (tpcommit(cflgs) == -1)
	      {
	         (void)fprintf(stderr,
	               "Failed to commit transaction %d, %s\n",
	               i, tpstrerror(tperrno));
	         (void)userlog("Failed to commit transaction %d, %s\n",
	               i, tpstrerror(tperrno));
	         rc = 1;
	      };
	    };
	  };
	};

	/*************************************************************/
	/* Free up the message data                                  */
	/*************************************************************/
	tpfree((char *) pMsg);

	/*************************************************************/
	/* Leave the application                                     */
	/*************************************************************/
	if (tpterm() == -1)
	{
	   (void)fprintf(stderr,
	         "Failed to leave application, %s\n",
	         tpstrerror(tperrno));
	   (void)userlog("Failed to leave application, %s\n",
	         tpstrerror(tperrno));
	   exit(1);
	}

	/*************************************************************/
	/* Exit program                                              */
	/*************************************************************/
#ifdef lint
	return 1;
#else
	exit(1);
#endif
}
