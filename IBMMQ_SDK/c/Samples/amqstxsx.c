/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/oltp/amqstxsx.c */
/*********************************************************************/
/*                                                                   */
/* Module Name: amqstxsx.c                                           */
/*                                                                   */
/* Description: IBM MQ sample Server for Tuxedo                      */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2016"                                               */
/*   crc="633488697" >                                               */
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
/* amqstxsx.c is a sample Tuxedo server working with IBM MQ.         */
/* It provides a PUT service and a GET service.                      */
/* Arguments are as follows                                          */
/*      -m QMgrName     = Queue Manager to connect to                */
/*                                                                   */
/* amqstxsx.c contains the following functions:                      */
/*  main                 main function entry point                   */
/*                                                                   */
/* For build instructions see the MQ documentation                   */
/*********************************************************************/

/*********************************************************************/
/* Includes                                                          */
/*********************************************************************/

/*********************************************************************/
/* Select compiler specific header files                             */
/*********************************************************************/
#include <stdio.h>
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

/*********************************************************************/
/* include MQ header files                                           */
/*********************************************************************/
#include <cmqc.h>

/*********************************************************************/
/* include local structure definitions                               */
/*********************************************************************/
#include "amqstxvx.h"

/*********************************************************************/
/* Data declarations                                                 */
/*********************************************************************/
static char *pgmname;           /* program name = argv[0] */
static char *pReply;            /* pointer to audit buf struct */
static char *QMName ="";
static MQHCONN HConn;

/*********************************************************************/
/* The initialisation function                                       */
/*********************************************************************/
int
#if defined(__STDC__) || defined(__cplusplus)
tpsvrinit(int argc, char **argv)
#else
tpsvrinit(argc,argv)
int     argc;
char    **argv;
#endif
{
int c;
MQLONG CompCode;
MQLONG Reason;

	/*************************************************************/
	/* Work out the name of the program                          */
	/*************************************************************/
	pgmname = argv[0];
	userlog("%s started", pgmname);

	/*************************************************************/
	/* Pick up QMName from arguments                             */
	/*************************************************************/
	while((c = getopt(argc,argv,"m:")) != EOF)
	        switch((char)c) {
	        case 'm':
	                QMName = optarg;
	                break;
	        default:
	                (void)userlog( "%s:  usage %s [-m QMName]",
	                      proc_name, proc_name);
	                return(-1);
	        }

	/*************************************************************/
	/* Open the resource manager                                 */
	/*************************************************************/
	if (tpopen() == -1) {
	        (void)userlog("%s: tpopen failed tperrno %d",
	              pgmname,tperrno);
	        return(-1);
	}

	/*************************************************************/
	/* Connect to the Queue Manager                              */
	/*************************************************************/
	MQCONN(QMName, &HConn, &CompCode, &Reason);
	if (CompCode != MQCC_OK)
	{
	  (void)userlog("Unable to connect to QM, %s (%d)\n",
	        tpstrerror(tperrno), Reason);
	  return(-1);
	}

	/*************************************************************/
	/* Sucessful return to caller                                */
	/*************************************************************/
	return(0);
}

/*********************************************************************/
/* The Put service                                                   */
/*********************************************************************/
void
#if defined(__STDC__) || defined(__cplusplus)
MPUT(TPSVCINFO *svcinfo)
#else
MPUT(svcinfo)
TPSVCINFO *svcinfo;
#endif
{
	struct amqstxvx *pMsg = (struct amqstxvx *) svcinfo->data;
	char *pChar;
	MQLONG CompCode;
	MQLONG Reason;
	MQPMO  Pmo = { MQPMO_DEFAULT };
	MQMD   Md  = { MQMD_DEFAULT };
	MQOD   Od  = { MQOD_DEFAULT };

	/*************************************************************/
	/* Log the request                                           */
	/*************************************************************/
	userlog("MPUT Request %s:%s ", pMsg->QName, pMsg->Msg);

	/*************************************************************/
	/* Set up the MQ parameters                                  */
	/*************************************************************/
	memcpy(Od.ObjectName, pMsg->QName, 48);
	Pmo.Options = MQPMO_SYNCPOINT;
  memcpy(Md.Format,MQFMT_STRING, (size_t)MQ_FORMAT_LENGTH);

	/*************************************************************/
	/* Issue the requested call                                  */
	/*************************************************************/
	MQPUT1(HConn, &Od,
	       &Md, &Pmo,
	       strlen(pMsg->Msg)+1, pMsg->Msg,
	       &CompCode, &Reason);

	/*************************************************************/
	/* Set up the reason code in the reply structure             */
	/*************************************************************/
	pMsg->Reason = Reason;

	/*************************************************************/
	/* Return to caller                                          */
	/*************************************************************/
	tpreturn(TPSUCCESS, 0, (void *)pMsg, sizeof(struct amqstxvx), 0);
}

/*********************************************************************/
/* The Get service                                                   */
/*********************************************************************/
void
#if defined(__STDC__) || defined(__cplusplus)
MGET(TPSVCINFO *svcinfo)
#else
MGET(svcinfo)
TPSVCINFO *svcinfo;
#endif
{
	struct amqstxvx *pMsg = (struct amqstxvx *) svcinfo->data;
	MQLONG CompCode;
	MQLONG Reason;
	MQLONG CCompCode;
	MQLONG CReason;
	MQLONG DataLen;
	MQGMO  Gmo = { MQGMO_DEFAULT };
	MQMD   Md  = { MQMD_DEFAULT };
	MQOD   Od  = { MQOD_DEFAULT };
	MQHOBJ HObj;

	/*************************************************************/
	/* Log the request                                           */
	/*************************************************************/
	userlog("MGET Request %s", pMsg->QName);

	/*************************************************************/
	/* Set up the MQ parameters                                  */
	/*************************************************************/
	memcpy(Od.ObjectName, pMsg->QName, 48);
	Gmo.Options = MQGMO_SYNCPOINT;

	/*************************************************************/
	/* Open the requested queue                                  */
	/*************************************************************/
	MQOPEN(HConn, &Od, MQOO_INPUT_SHARED, &HObj,
	       &CompCode, &Reason);
	if (Reason == MQRC_NONE)
	{
	  /***********************************************************/
	  /* Get a message from the queue                            */
	  /***********************************************************/
	  MQGET(HConn, HObj, &Md,
	        &Gmo, sizeof(pMsg->Msg)-1,
	        pMsg->Msg, &DataLen,
	        &CompCode, &Reason);

	  /***********************************************************/
	  /* Close the queue                                         */
	  /***********************************************************/
	  MQCLOSE(HConn, &HObj, MQCO_NONE,
	          &CCompCode, &CReason);

	  /***********************************************************/
	  /* Only cascade CLOSE error if GET was OK                  */
	  /***********************************************************/
	  if (Reason == MQRC_NONE)
	    Reason = CReason;
	};

	/*************************************************************/
	/* Set up the reply message                                  */
	/*************************************************************/
	pMsg->Reason = Reason;
	if (Reason == MQRC_NONE)
	{
	  if (DataLen<sizeof(pMsg->Msg)) pMsg->Msg[DataLen] = 0;
	};

	/*************************************************************/
	/* Return to caller                                          */
	/*************************************************************/
	tpreturn(TPSUCCESS, 0, (void *)pMsg, sizeof(struct amqstxvx), 0);
}

