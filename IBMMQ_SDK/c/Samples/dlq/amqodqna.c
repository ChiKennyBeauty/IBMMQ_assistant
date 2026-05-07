const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=cmd/servers/amqodqna.c";
/*********************************************************************/
/*                                                                   */
/* Module Name: AMQODQNA.c                                           */
/*                                                                   */
/* Description: Common source for the IBM MQ Dead letter             */
/* handler.                                                          */
/*   <copyright                                                      */
/*   notice="lm-source-program"                                      */
/*   pids="5724-H72,"                                                */
/*   years="1994,2016"                                               */
/*   crc="2143900702" >                                              */
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
/*                                                                   */
/*--------------------------------------------------------------------*/
/* This file contains the functions used to decide what action to take*/
/* for a given message on the dead letter queue.                      */
/* Input to the decision process is through stdin, and consists of a  */
/* series of pattern definitions, and an action associated with each  */
/* pattern definition. When a message arrives on the dead letter queue*/
/* we scan the pattern definitions until we find the first match and  */
/* the attempt to take the appropriate action.                        */
/*                                                                    */
/* This file contains the pattern matching code for the DLQ handler.  */
/* (amqodqna.c is the pattern matching code, amqodqma and amqodqpa    */
/* contain the pattern building code.)                                */
/*                                                                    */
/* amqodqma.y and amqodqpa.l contain the yacc gramar and the lex      */
/* specification for the input stream. amqodqma.y converts this       */
/* input stream into a list of structures of type odqPattern_t        */
/* which are then used to match messages read from the DLQ.           */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <cmqc.h>
#include <amqodqha.h>




/*--------------------------------------------------------------------*/
/*                                                                    */
/*  I N T E R N A L    F U N C T I O N    P R O T O T Y P E S         */
/*                                                                    */
/*--------------------------------------------------------------------*/

static odqResp odqMatchLine(
                            /*IN*/
                              const odqPattern_t *pPattern,
                              const MQDLH        *dlh,
                              const MQMD         *mqmd
                            /*OUT*/
                           );
static odqResp odqMatchString( /*IN*/
                                 const char * parm,
                                 const char * value,
                                 const MQLONG length
                               /*OUT*/
                             );
static odqResp odqMatchInteger( /*IN*/
                                  const int parm,
                                  const int * value,
                                  const MQLONG length
                                /*OUT*/
                               );

/*--------------------------------------------------------------------*/
/*                                                                    */
/*       E X E C U T A B L E     C O D E                              */
/*                                                                    */
/*--------------------------------------------------------------------*/

/*********************************************************************/
/*                                                                   */
/* Function: odqMatchMessage                                         */
/*                                                                   */
/* Description: Find a rule in the rule table to match a message     */
/*                                                                   */
/* Intended Function: This function scans the rule table tooking     */
/*                    ffor a rule that matches a message read from   */
/*                    the dead letter queue.                         */
/*                                                                   */
/* Input Parameters: dead letter header                              */
/*                   message descriptor                              */
/*                   number of times the message has been tried      */
/*                                                                   */
/*                                                                   */
/* Output Parameters: address of the rule                            */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
#define FUNCTION_ID odqtodqMatchMessage
/*--------------------------------------------------------------------*/
/* Attempt to match a message from the DLQ with the patterns provided */
/* as input.                                                          */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
odqResp odqMatchMessage( /*IN*/
                           const MQDLH   *dlh,
                           const MQMD    *MsgDesc,
                           const MQLONG  RetryCount,
                         /*OUT*/
                           odqPattern_t **pattern
                       )
{
 odqResp rc=odq_RetryLater ;
 odqPattern_t *pPattern ;
 MQLONG  RetryTotal=0 ;

 odq_fnc_entry( FUNCTION_ID ) ;

 /*-------------------------------------------------------------------*/
 /* Scan the list of patterns looking for matching patterns.          */
 /*-------------------------------------------------------------------*/
 for ( pPattern=odqGlobal.odqPatternHead;
       pPattern!=NULL && rc != odq_PatternMatches;
       pPattern=pPattern->next)
 {
   rc = odqMatchLine(
                      /*IN*/
                        pPattern,
                        dlh,
                        MsgDesc
                      /*OUT*/
                    ) ;
   if ( odq_PatternMatches == rc )
   {
     /*---------------------------------------------------------------*/
     /* if we've found a matching pattern then check to see if the    */
     /* number of retries indicates that we've already retried this   */
     /* message as often as we should have.                           */
     /* Otherwise continue to search for the next matching pattern    */
     /*---------------------------------------------------------------*/

     /*---------------------------------------------------------------*/
     /* It's not possible for us to fail to ignore a message, so the  */
     /* retry count does not apply in this case.                      */
     /*---------------------------------------------------------------*/
     if( pPattern->Parm[Action].i == odqIGNORE )
     {
       *pattern = pPattern ;
       break;
     }

     RetryTotal += pPattern->Parm[Retry].i ;
     if( RetryTotal > RetryCount )
     {
       *pattern = pPattern ;
       break;
     }
     else
     {
       rc=odq_RetryLater ;
     }
   }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}
#undef FUNCTION_ID
#define FUNCTION_ID odqtodqMatchLine

/*********************************************************************/
/*                                                                   */
/* Function: odqMatchLine                                            */
/*                                                                   */
/* Description: See if a particular rule line matches a message      */
/*                                                                   */
/* Intended Function: This function determines if a rule from the    */
/*                    rule table matches a message from the DLQ.     */
/*                                                                   */
/* Input Parameters: address of rule line                            */
/*                   dead letter header                              */
/*                   message descriptor                              */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqMatchLine(
                            /*IN*/
                              const odqPattern_t *pPattern,
                              const MQDLH   *dlh,
                              const MQMD    *mqmd
                            /*OUT*/
                           )
{
 odqResp rc=odq_PatternMatches ;
 odqKeyword key ;
 char *  odqStruct[odqSTRUCTLAST];

/*--------------------------------------------------------------------*/
/* For a given line check each attribute in turn looking for a        */
/* mismatch. If all attributes match then we've found the line that   */
/* describes the action for the current message.                      */
/*                                                                    */
/*                                                                    */
/*--------------------------------------------------------------------*/
 odq_fnc_entry( FUNCTION_ID ) ;
 odqStruct[odqSTRUCTMQMD] = (char *)mqmd ;
 odqStruct[odqSTRUCTMQDLH] = (char *)dlh ;
 for( key=FirstKeyword ;
      key<LastKeyword && rc==odq_PatternMatches;
      key ++ )
 {
   if( *(pPattern->ParmSpecified+key)==Specified &&
        odqParmAttrs[key].Struct )
   {
     switch(odqParmAttrs[key].format)
     {
      case 's':
      case 'w':
         rc=odqMatchString(/*IN*/
                             pPattern->Parm[key].s,
                             odqStruct[odqParmAttrs[key].Struct]+
                               odqParmAttrs[key].Offset,
                             odqParmAttrs[key].maxlen
                           /*OUT*/
                          );
         break ;
      case 'd':
         rc=odqMatchInteger(/*IN*/
                            pPattern->Parm[key].i,
                            (int *)(odqStruct[odqParmAttrs[key].Struct]+
                               odqParmAttrs[key].Offset),
                            odqParmAttrs[key].maxlen
                            /*OUT*/
                           );
         break ;
      default:
         break ;
     }
   }
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}
#undef FUNCTION_ID
#define FUNCTION_ID odqtodqMatchInteger

/*********************************************************************/
/*                                                                   */
/* Function: odqMatchInteger                                         */
/*                                                                   */
/* Description: Check if an integer pattern matches an integer       */
/*                                                                   */
/* Intended Function: This function checks if an integer as          */
/*                    as specified in a pattern field mtaches        */
/*                    an integer field from the message.             */
/*                                                                   */
/* Input Parameters: pattern field                                   */
/*                   integer value                                   */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqMatchInteger( /*IN*/
                                  const int parm,
                                  const int * value,
                                  const MQLONG length
                                /*OUT*/
                              )
{
/*--------------------------------------------------------------------*/
/* odqMatchInteger: Compare a integer with a pattern and decide if    */
/*                  the integer matches the pattern.                  */
/*                                                                    */
/*--------------------------------------------------------------------*/
 odqResp rc=odq_PatternMismatch;

 odq_fnc_entry( FUNCTION_ID ) ;
 if( *value == parm )
 {
   rc=odq_PatternMatches;
 }
 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}


#undef FUNCTION_ID
#define FUNCTION_ID odqtodqMatchString
/*********************************************************************/
/*                                                                   */
/* Function: odqMatchString                                          */
/*                                                                   */
/* Description: Check if an string pattern matches a string          */
/*                                                                   */
/* Intended Function: This function checks if a string as            */
/*                    as specified in a pattern field mtaches        */
/*                    an string field from the message.              */
/*                                                                   */
/* Input Parameters: pattern field                                   */
/*                   string value                                    */
/*                                                                   */
/* Output Parameters: none                                           */
/*                                                                   */
/* Returns:                                                          */
/*********************************************************************/
static odqResp odqMatchString( /*IN*/
                                 const char * parm,
                                 const char * value,
                                 const MQLONG length
                               /*OUT*/
                             )
{
 odqResp rc=odq_PatternMatches;
 int idxp;
 int idxs ;

 odq_fnc_entry( FUNCTION_ID ) ;

 for( idxp = 0, idxs = 0 ;
      (idxs < length) && (rc == odq_PatternMatches) ;
      idxp ++, idxs ++ )
 {
   switch( parm[idxp] )
   {
     case 0:
             /*-------------------------------------------------------*/
             /* if the pattern ends before the string then we check to*/
             /* see if the string is comprised only of blanks.        */
             /* All of the data we are comparing is character data and*/
             /* so has been padded with blanks by IBM MQ.             */
             /*-------------------------------------------------------*/
             for( ; idxs < length; idxs ++ )
             {
               if( ' ' != value[idxs])
               {
                 rc = odq_PatternMismatch ;
                 break ;
               }
             }
             idxp -- ;
             break ;
     case odqANY_CHARACTER:
             /*-------------------------------------------------------*/
             /* Matches any single character other than a trailing    */
             /* blank.                                                */
             /*-------------------------------------------------------*/
             {
               int i ;
               /*-----------------------------------------------------*/
               /* look for the first non blank character              */
               /*-----------------------------------------------------*/
               for( i = idxs ;
                    i < length && ' ' == value[i] ;
                    i ++ )  ;
               /*-----------------------------------------------------*/
               /*  no non blank characters => signal a mismatch       */
               /*-----------------------------------------------------*/
               if ( i >= length )
               {
                 rc = odq_PatternMismatch ;
               }
             }
             break ;
     case odqANY_SUBSTRING:
             /*-------------------------------------------------------*/
             /* Matches any string.                                   */
             /*-------------------------------------------------------*/
             {
               /*-----------------------------------------------------*/
               /* if this is a trailing * then the string matches.    */
               /*-----------------------------------------------------*/
               if( 0 == parm[idxp+1] )
               {
                 idxs = length ;
               }
               else
               {
                 /*---------------------------------------------------*/
                 /* See if any trailing substring of the string       */
                 /* matches the remainder of the pattern.             */
                 /*---------------------------------------------------*/
                 int idx ;
                 odqResp rc1=odq_PatternMismatch ;
                 for( idx=idxs ;
                      (idx<length) && (rc1 != odq_PatternMatches) ;
                      idx++ )
                 {
                   rc1 = odqMatchString( &(parm[idxp+1]),
                                         &(value[idx]),
                                         length-idx );
                 }
                 idxs = length ;
                 idxp = (int)strlen(parm) -1 ;
                 rc = rc1 ;
               }
             }
             break ;
     default:
             /*-------------------------------------------------------*/
             /* If it's not a wildcard then it must be an exact match.*/
             /*-------------------------------------------------------*/
             if( parm[idxp] == value[idxs] )
             {
               break ;
             }
             else
             {
               rc = odq_PatternMismatch ;
               break ;
             }
   }
 }
 /*-------------------------------------------------------------------*/
 /* If the string ran out before the pattern then check to see if     */
 /* the pattern is also exhausted.                                    */
 /* Any number of trailing '*' can be ignored.                        */
 /*-------------------------------------------------------------------*/
 if (rc==odq_PatternMatches)
 {
   while( 0 != parm[idxp] )
   {
     if (odqANY_SUBSTRING != parm[idxp])
     {
       rc = odq_PatternMismatch ;
     }
     idxp ++ ;
   }
 }

 odq_fnc_retcode( FUNCTION_ID, rc ) ;
 return rc ;
}
#undef FUNCTION_ID
