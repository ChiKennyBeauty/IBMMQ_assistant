/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=include/amqodtha.h */
/**********************************************************************/
/*                                                                    */
/* Module Name: AMQODTHA.H                                            */
/*                                                                    */
/* Description: Trace constants for DLQ handler.                      */
/*   <copyright                                                       */
/*   notice="lm-source-program"                                       */
/*   pids="5724-H72,"                                                 */
/*   years="1994,2021"                                                */
/*   crc="824978263" >                                                */
/*   Licensed Materials - Property of IBM                             */
/*                                                                    */
/*   5724-H72,                                                        */
/*                                                                    */
/*   (C) Copyright IBM Corp. 1994, 2021 All Rights Reserved.          */
/*                                                                    */
/*   US Government Users Restricted Rights - Use, duplication or      */
/*   disclosure restricted by GSA ADP Schedule Contract with          */
/*   IBM Corp.                                                        */
/*   </copyright>                                                     */
/* Function:                                                          */
/* Trace constants used by the DLQ handler.                           */
/*                                                                    */
/**********************************************************************/
#if !defined AMQODTHA_H || defined ODQ_FUNCTION_NAMES
#define AMQODTHA_H

/* ------------------------------------------------------------------ */
/* The list of function names is pulled into amqoexta.h where other   */
/* function names are added. In case there is a need for additonal    */
/* functions in the DLQ handler, 'spare' slots have been placed at    */
/* the end of the list.                                               */
/*                                                                    */
/* Note: The enum relies on the behaviour of an enum with no initial  */
/* value starting at 0 - odqtmain is defined as 0. This makes it a    */
/* requirement that the DLQ handler function IDs are the first range  */
/* of values in their designated component (xcsCOMP_O).               */
/* ------------------------------------------------------------------ */

#ifdef  ODQ_FUNCTION_NAMES
#undef odq_function_name
#define odq_function_name(name) \
 {#name, 0}
#else
typedef enum
{
#undef odq_function_name
#define odq_function_name(name) \
 odqt##name
#endif
  odq_function_name(main),
  odq_function_name(odqConnectQmgr),
  odq_function_name(odqDisconnectQmgr),
  odq_function_name(odqInquireQMAttrs),

  odq_function_name(odqProcessQueue),
  odq_function_name(odqSyncpoint),
  odq_function_name(odqStartBrowse),
  odq_function_name(odqEndBrowse),
  odq_function_name(odqGetNext),
  odq_function_name(odqProcessMessage),

  odq_function_name(odqyyparse),
  odq_function_name(odqProcessStdin),
  odq_function_name(odqPrepareTemplate),
  odq_function_name(odqAddParm),
  odq_function_name(odqUseDefault),
  odq_function_name(odqAddLine),

  odq_function_name(odqMatchMessage),
  odq_function_name(odqMatchLine),
  odq_function_name(odqMatchString),
  odq_function_name(odqMatchInteger),

  odq_function_name(odqPreInitialize),
  odq_function_name(odqParseArgs),
  odq_function_name(odqLoadLibrary),
  odq_function_name(odqInitialize),
  odq_function_name(odqFFDC),
  odq_function_name(odqGetMem),
  odq_function_name(odqFreeMem),
  odq_function_name(odqMQCONN),

  odq_function_name(odqInquireRetryCount),
  odq_function_name(odqAddMsg),
  odq_function_name(odqDeleteMsg),

  odq_function_name(odqStrDup),
  odq_function_name(odqQStrDup),
  odq_function_name(odqAtoi),
  odq_function_name(odqSymbtoi),
  odq_function_name(odqInitialiseRetryCount),
  odq_function_name(odqConvertDLH),
  odq_function_name(odqGetGMOOpts),
  
  odq_function_name(odqMQCONNX),
  odq_function_name(odqMQBACK),
  odq_function_name(odqMQCLOSE),
  odq_function_name(odqMQCMIT),
  odq_function_name(odqMQCRTMH),
  odq_function_name(odqMQDISC),
  odq_function_name(odqMQDLTMH),
  odq_function_name(odqMQGET),
  odq_function_name(odqMQINQ),
  odq_function_name(odqMQOPEN),
  odq_function_name(odqMQPUT1),

/* ------------------------------------------------------------------ */
/* The following entries are to allow for potential expansion of the  */
/* DLQ handler. The name of the spare entry should be changed as      */
/* required.                                                          */
/* ------------------------------------------------------------------ */

  odq_function_name(spare01),
  odq_function_name(spare02),
  odq_function_name(spare03),
  odq_function_name(spare04),
  odq_function_name(spare05),
  odq_function_name(spare06),
  odq_function_name(spare07),
  odq_function_name(spare08),
  odq_function_name(spare09),
  odq_function_name(spare10),
  odq_function_name(spare11),
  odq_function_name(spare12),
  odq_function_name(spare13),
  odq_function_name(spare14),
  odq_function_name(spare15),
  odq_function_name(spare16),
  odq_function_name(spare17),
  odq_function_name(spare18),
  odq_function_name(spare19),
  odq_function_name(spare20),
  odq_function_name(spare21),
  odq_function_name(spare22),
  odq_function_name(spare23),
  odq_function_name(spare24),
  odq_function_name(spare25),
  odq_function_name(spare26),
  odq_function_name(spare27),
  odq_function_name(spare28),
  odq_function_name(spare29),
  odq_function_name(spare30),
  odq_function_name(spare31),
  odq_function_name(spare32),
  odq_function_name(spare33),
  odq_function_name(spare34),
  odq_function_name(spare35),
  odq_function_name(spare36),
  odq_function_name(spare37),
  odq_function_name(spare38),
  odq_function_name(spare39),
  odq_function_name(spare40),
  odq_function_name(spare41),
  odq_function_name(spare42),
  odq_function_name(spare43),
  odq_function_name(spare44),
  odq_function_name(spare45),
  odq_function_name(spare46),
  odq_function_name(spare47),
  odq_function_name(spare48),
  odq_function_name(spare49),
  odq_function_name(spare50),
  odq_function_name(spare51),
  odq_function_name(spare52),
  odq_function_name(spare53),
  odq_function_name(spare54),
  odq_function_name(spare55),
  odq_function_name(spare56),
  odq_function_name(spare57),
  odq_function_name(spare58),
  odq_function_name(spare59),
  odq_function_name(spare60),
  odq_function_name(spare61),
  odq_function_name(spare62),
  odq_function_name(spare63),
  odq_function_name(spare64),
  odq_function_name(spare65),
  odq_function_name(spare66),
  odq_function_name(spare67),
  odq_function_name(spare68),
  odq_function_name(spare69),
  odq_function_name(spare70),
  odq_function_name(spare71),
  odq_function_name(spare72),
  odq_function_name(spare73),
  odq_function_name(spare74),
  odq_function_name(spare75),
  odq_function_name(spare76),
  odq_function_name(spare77),
  odq_function_name(spare78),

/* ------------------------------------------------------------------ */
/* LastFunc must be defined as the last entry in order for the        */
/* definitions in amqoexta.h to start at the correct numeric value    */
/* (odqtLastFunc + 1). Note that effectively, 'LastFunc' is also a    */
/* spare function name, but it MUST be named 'LastFunc'!              */
/* ------------------------------------------------------------------ */

  odq_function_name(LastFunc)

#ifdef  ODQ_FUNCTION_NAMES
/* empty */
#else
} odqTraceId;
#endif

#endif

