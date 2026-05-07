/*  @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=include/amqodkha.h */

/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQHT.H                                           */
/*                                                                   */
/* Description: RUNMQDLQ parameter types                             */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2020"                                               */
/*   crc="2060528840" >                                              */
/*   Licensed Materials - Property of IBM                            */
/*                                                                   */
/*   5724-H72,                                                       */
/*                                                                   */
/*   (C) Copyright IBM Corp. 1994, 2020 All Rights Reserved.         */
/*                                                                   */
/*   US Government Users Restricted Rights - Use, duplication or     */
/*   disclosure restricted by GSA ADP Schedule Contract with         */
/*   IBM Corp.                                                       */
/*   </copyright>                                                    */
/*********************************************************************/
/*                                                                   */
/* Function:                                                         */
/*                                                                   */
/* This file contains a table of the keywords which are supported    */
/* by the DLQ handler.                                               */
/*                                                                   */
/*                                                                   */
/*********************************************************************/
#ifdef ODQENUM
#undef odqParmDefPattern
#undef odqParmDefLimit
#undef odqParmDefStatic
#define odqParmDefPattern(a,b,c,d,e) a
#define odqParmDefLimit(a) a
#define odqParmDefStatic(a,b,c,d) a
typedef enum
{
#else
#undef odqParmDefPattern
#undef odqParmDefLimit
#undef odqParmDefStatic
#ifndef FIELDOFFSET
 #ifndef offsetof
   #include <stddef.h>
 #endif
 #define FIELDOFFSET(type, field)  offsetof(type,field)
#endif
#define odqParmDefPattern(a,b,c,d,e) {\
                               b,\
                               c,\
                               odqSTRUCT##d,\
                               FIELDOFFSET(d,a),\
                               #e}
#define odqParmDefLimit(a)       {\
                             0,\
                             0,\
                             0,\
                             0,\
                             NULL}
#define odqParmDefStatic(a,b,c,d) {\
                              b,\
                              c,\
                              0,\
                              0,\
                              #d}
#define odqSTRUCTMQMD  1
#define odqSTRUCTMQDLH 2
#define odqSTRUCTLAST  3

typedef struct
{
 int    maxlen;
 char   format;
 int    Struct;
 int    Offset;
 char  *keyword;
} odqParmAttr_t;

static const odqParmAttr_t odqParmAttrs[]={
#endif
 odqParmDefLimit(FirstKeyword),
 odqParmDefPattern(ApplIdentityData,MQ_APPL_IDENTITY_DATA_LENGTH,'w',MQMD, APPLIDAT),
 odqParmDefPattern(DestQName,MQ_Q_NAME_LENGTH,'w', MQDLH, DESTQ),
 odqParmDefPattern(DestQMgrName,MQ_Q_MGR_NAME_LENGTH,'w',MQDLH, DESTQM),
 odqParmDefPattern(Format,MQ_FORMAT_LENGTH,'w', MQDLH, FORMAT),
 odqParmDefPattern(PutApplName,MQ_PUT_APPL_NAME_LENGTH,'w',MQMD, APPLNAME),
 odqParmDefPattern(ReplyToQ,MQ_Q_NAME_LENGTH,'w',MQMD, REPLYQ),
 odqParmDefPattern(ReplyToQMgr,MQ_Q_MGR_NAME_LENGTH,'w',MQMD, REPLYQM),
 odqParmDefPattern(UserIdentifier,MQ_USER_ID_LENGTH,'w',MQMD, USERID),
 odqParmDefPattern(PutApplType,sizeof(MQLONG),'d',MQMD, APPLTYPE),
 odqParmDefPattern(Feedback,sizeof(MQLONG),'d',MQMD, FEEDBACK),
 odqParmDefPattern(MsgType,sizeof(MQLONG),'d',MQMD, MSGTYPE),
 odqParmDefPattern(Reason,sizeof(MQLONG),'d',MQDLH, REASON),
 odqParmDefPattern(Persistence,sizeof(MQLONG),'d',MQMD, PERSIST),
 odqParmDefStatic(ForwardQ, MQ_Q_NAME_LENGTH, 's', FWDQ),
 odqParmDefStatic(ForwardQMgr, MQ_Q_MGR_NAME_LENGTH, 's', FWDQM),
 odqParmDefStatic(Action, sizeof(int), 'd', ACTION),
 odqParmDefStatic(Retry, sizeof(int), 'd', RETRY),
 odqParmDefStatic(Header, sizeof(int),'d',HEADER),
 odqParmDefStatic(PutAut, sizeof(int) ,'d',PUTAUT),
 odqParmDefStatic(InputQ, MQ_Q_NAME_LENGTH ,'s',INPUTQ),
 odqParmDefStatic(InputQM, MQ_Q_MGR_NAME_LENGTH ,'s',INPUTQM),
 odqParmDefStatic(RetryInt, sizeof(int) ,'d',RETRYINT),
 odqParmDefStatic(Wait, sizeof(int) ,'d',WAIT),
 odqParmDefLimit(LastKeyword)
#ifdef ODQENUM
} odqKeyword;
#else
};
#endif
