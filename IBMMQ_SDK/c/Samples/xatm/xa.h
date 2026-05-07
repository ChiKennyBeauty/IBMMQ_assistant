/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=include/pc/winnt/mqxa.h */
/***********************************************************************/
/*                                                                     */
/*  Module Name: MQXA.H                                                */
/*                                                                     */
/*  Description: This files declares functions, structures, and        */
/*               constants relating to the XA interface. MQ users this */
/*               interface when it coordinates units of work which     */
/*               involve external resource managers.                   */
/*                                                                     */
/*   <copyright                                                        */
/*   notice="lm-source-program"                                        */
/*   pids="5724-H72"                                                   */
/*   years="1994,2019"                                                 */
/*   crc="3755242588" >                                                */
/*   Licensed Materials - Property of IBM                              */
/*                                                                     */
/*   5724-H72                                                          */
/*                                                                     */
/*   (C) Copyright IBM Corp. 1994, 2019 All Rights Reserved.           */
/*                                                                     */
/*   US Government Users Restricted Rights - Use, duplication or       */
/*   disclosure restricted by GSA ADP Schedule Contract with           */
/*   IBM Corp.                                                         */
/*   </copyright>                                                      */
/*                                                                     */
/***********************************************************************/

/*
 * Start of xa.h header
 *
 * Define a symbol to prevent multiple inclusions of this header file
 */

#ifndef XA_H
#define XA_H

#include <cmqc.h>                                          /* MQI */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <txdtc.h>

/*
 * Transaction branch identification: XID and NULLXID:
 */
#define XIDDATASIZE   128                                  /* size in bytes */
#define MAXGTRIDSIZE  64                                   /* maximum size in bytes of gtrid */
#define MAXBQUALSIZE  64                                   /* maximum size in bytes of bqual */

typedef struct xid_t XID;

struct xid_t_64 {
  MQINT64 formatID;                                        /* format identifier */
  MQINT64 gtrid_length;                                    /* value from 1 through 64 */
  MQINT64 bqual_length;                                    /* value from 1 through 64 */
  char data[XIDDATASIZE];
  };
typedef struct xid_t_64 XID_64;
/*
 * A value of -1 in formatID means that the XID is null.
 */

/*
 * Declarations of routines by which RMs call TMs:
 */
typedef int __cdecl ax_reg_t(int, XID *, MQINT32);
typedef int __cdecl ax_unreg_t(int, MQINT32);
typedef int __cdecl ax_reg_t_64(int, XID_64 *, MQINT64);
typedef int __cdecl ax_unreg_t_64(int, MQINT64);

extern ax_reg_t ax_reg;
extern ax_unreg_t ax_unreg;

/*
 * XA Switch Data Structure
 */
#define RMNAMESZ 32                                        /* length of resource manager name, */
                                                           /* inluding the null terminator */
#define MAXINFOSIZE 256                                    /* maximum size in bytes of xa_info
                                                              strings, including the null-terminator */

/******************************************************************************/
/* The 64-bit stuff here mirrors the UNIX platforms, but as 'long' on Windows */
/* is 32-bit for 64-bit applications, it's probably never going to be needed. */
/******************************************************************************/
struct xa_switch_t_64 {
  char name[RMNAMESZ];                                     /* name of resource manager */
  MQINT64 flags;                                           /* resource manager specific options */
  MQINT64 version;                                         /* must be 0 */
  int (__cdecl * xa_open_entry) (char *, int, MQINT64);    /* xa_open function pointer */
  int (__cdecl * xa_close_entry) (char *, int, MQINT64);   /* xa_close function pointer*/
  int (__cdecl * xa_start_entry) (XID_64 *, int, MQINT64); /* xa_start function pointer */
  int (__cdecl * xa_end_entry) (XID_64 *, int, MQINT64);   /* xa_end function pointer */
  int (__cdecl * xa_rollback_entry) (XID_64 *, int, MQINT64); /* xa_rollback function pointer */
  int (__cdecl * xa_prepare_entry) (XID_64 *, int, MQINT64); /* xa_prepare function pointer */
  int (__cdecl * xa_commit_entry) (XID_64 *, int, MQINT64); /* xa_commit function pointer */
  int (__cdecl * xa_recover_entry) (XID_64 *, MQINT64, int, MQINT64);
                                                           /* xa_recover function pointer */
  int (__cdecl * xa_forget_entry) (XID_64 *, int, MQINT64); /* xa_forget function pointer */
  int (__cdecl * xa_complete_entry) (int *, int *, int, MQINT64);
                                                           /* xa_complete function pointer */
  };
/*
 * Flag definitions for the RM switch
 */
#define TMNOFLAGS    0x00000000                            /* no resource manager features selected */
#define TMREGISTER   0x00000001                            /* resource manager dynamically registers */
#define TMNOMIGRATE  0x00000002                            /* resource manager does not support
                                                              association migration */
#define TMUSEASYNC   0x00000004                            /* resource manager supports
                                                              asynchronous operations */
/*
 * Flag definitions for xa_and ax_ routines
 */
/* use TMNOFLAGS, defined above, when not specifying other flags */
#define TMASYNC      0x80000000                            /* perform routine asynchronously */
#define TMONEPHASE   0x40000000                            /* caller is using on-phase commit
                                                              optimisation */
#define TMFAIL       0x20000000                            /* dissociates caller and marks
                                                              transaction branch rollback-only */
#define TMNOWAIT     0x10000000                            /* return if blocking condition exists */
#define TMRESUME     0x08000000                            /* caller is resuming association
                                                              with suspended transaction branch */
#define TMSUCCESS    0x04000000                            /* dissociate caller from transaction
                                                              branch*/
#define TMSUSPEND    0x02000000                            /* caller is suspending, not ending,
                                                              association */
#define TMSTARTRSCAN 0x01000000                            /* start a recovery scan */
#define TMENDRSCAN   0x00800000                            /* end a recovery scan */
#define TMMULTIPLE   0x00400000                            /* wait for any asynchronous operation */
#define TMJOIN       0x00200000                            /* caller is joining existing transaction
                                                              branch */
#define TMMIGRATE    0x00100000                            /* caller intends to perfrom migration */
/*
 * ax_() return codes (transaction manager reports to resource manager)
 */
#define TM_JOIN      2                                     /* caller is joining existing transaction
                                                              branch */
#define TM_RESUME    1                                     /* caller is resuming association with
                                                              suspended transaction branch */
#define TM_OK        0                                     /* normal execution */
#define TMER_TMERR   (-1)                                  /* an error occured in the
                                                              transaction manager */
#define TMER_INVAL   (-2)                                  /* invalid arguments were given */
#define TMER_PROTO   (-3)                                  /* routine invoked in an improper context */
/*
 * xa_() return codes (resource manager reports to transaction manager)
 */
#define XA_RBBASE      100                                 /* The inclusive lower bound of the
                                                              rollback codes */
#define XA_RBROLLBACK  XA_RBBASE                           /* The rollback was caused by an
                                                              unspecified reason */
#define XA_RBCOMMFAIL  XA_RBBASE+1                         /* The rollback was caused by a
                                                              communication failure */
#define XA_RBDEADLOCK  XA_RBBASE+2                         /* A deadlock was detected */
#define XA_RBINTEGRITY XA_RBBASE+3                         /* A condition that violates the integrity
                                                              of the resources was detected */
#define XA_RBOTHER     XA_RBBASE+4                         /* The resource manager rolled back the
                                                              transaction branch for a reason not on
                                                              this list */
#define XA_RBPROTO     XA_RBBASE+5                         /* A protocol error occurred in the
                                                              resource manager */
#define XA_RBTIMEOUT   XA_RBBASE+6                         /* A transaction branch took too long */
#define XA_RBTRANSIENT XA_RBBASE+7                         /* May retry the transaction branch */
#define XA_RBEND       XA_RBTRANSIENT                      /* The inclusive upper bound of the
                                                              rollback codes */

#define XA_NOMIGRATE  9                                    /* resumption must occur where
                                                              suspension occured */
#define XA_HEURHAZ    8                                    /* the transaction branch may have
                                                              been heuristically completed */
#define XA_HEURCOM    7                                    /* the transaction branch has been
                                                              heuristically committed */
#define XA_HEURRB     6                                    /* the transaction branch has been
                                                              heuristically rolled back */
#define XA_HEURMIX    5                                    /* the transaction branch has been
                                                              heuristically committed and rolled
                                                              back */
#define XA_RETRY      4                                    /* routine returned with no effect and
                                                              may be re-issued */
#define XA_RDONLY     3                                    /* the transaction branch was read-
                                                              only and has been committed */
#define XA_OK         0                                    /* normal execution */
#define XAER_ASYNC    (-2)                                 /* asynchronous operation already outstanding */
#define XAER_RMERR    (-3)                                 /* a resource manager error occured in
                                                              the transaction branch */
#define XAER_NOTA     (-4)                                 /* the XID is not valid */
#define XAER_INVAL    (-5)                                 /* invalid arguments were given */
#define XAER_PROTO    (-6)                                 /* routine invoked in an improper context */
#define XAER_RMFAIL   (-7)                                 /* resource manager unavailable */
#define XAER_DUPID    (-8)                                 /* the XID already exits */
#define XAER_OUTSIDE  (-9)                                 /* resource manager doing work outside */
                                                           /* global transaction */

#endif /* ifndef XA_H */
/*
 * End of xa.h header
 */
