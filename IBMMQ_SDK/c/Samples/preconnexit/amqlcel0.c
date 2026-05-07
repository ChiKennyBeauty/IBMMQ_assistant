const static char sccsid[] = "@(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=lib/exits/eplookup/amqlcel0.txt";
/**********************************************************************/
/*                                                                    */
/* Module Name: AMQLCEL0.C                                            */
/*                                                                    */
/* Description: Connection endpoint lookup source file.               */
/* <copyright                                                         */
/* notice="lm-source-program"                                         */
/* pids="5724-H72,"                                                   */
/* years="1994,2024"                                                  */
/* crc="652464726" >                                                  */
/* Licensed Materials - Property of IBM                               */
/*                                                                    */
/* 5724-H72,                                                          */
/*                                                                    */
/* (C) Copyright IBM Corp. 1994, 2024 All Rights Reserved.            */
/*                                                                    */
/* US Government Users Restricted Rights - Use, duplication or        */
/* disclosure restricted by GSA ADP Schedule Contract with            */
/* IBM Corp.                                                          */
/* </copyright>                                                       */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*  This file contains the connection endpoint lookup exit routines   */
/*  for MQ. The exit does lookup in a LDAP Server.                    */
/*  This module contains one main entry point :                       */
/*                                                                    */
/*    MQENTRY LdapPreconnectExit ( pExitParms,                        */
/*                               , QmgrName                           */
/*                               , ppConnectOpts                      */
/*                               , pCompCode                          */
/*                               , pReasonCode)                       */
/*                                                                    */
/*  These functions call the various internal routines depending on   */
/*  the reason they were invoked.                                     */
/*                                                                    */
/*  Add PreConnect stanza in mqclient.ini                             */
/*  For Example                                                       */
/*                                                                    */
/*  PreConnect:                                                       */
/*  Module=<Module>                                                   */
/*  Function=LdapPreconnectExit                                       */
/*  Data=ldap://myLDAPServer.com:389/cn=wmq,ou=ibm,ou=com             */
/*  Sequence=1                                                        */
/*                                                                    */
/*  where Module is the path of the amqlcelp.dll on windows and       */
/*        amqlcelp module on unix platform                            */
/*                                                                    */
/*        Function is the entry point of the PreConnect exit code     */
/*                                                                    */
/*        Data is the parameter string containing a lookup address    */
/*        and a base DN for LDAP directory                            */
/*                                                                    */
/*        PreConnect exit Sequence is the sequence in which this exit */
/*        is called relative to other exits.                          */
/**********************************************************************/
/*                                                                    */
/* Notes:                                                             */
/*   Language:     C                                                  */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
#include <cmqc.h>
#include <cmqxc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <lber.h>
#include <ldap.h>


/* Instance structure definitions  */
typedef struct tagMQNLDAPCTX MQNLDAPCTX;
typedef MQNLDAPCTX MQPOINTER PMQNLDAPCTX;

struct tagMQNLDAPCTX
{
  MQCHAR4	StrucId;		/* Structure identifier */
  MQLONG	Version;		/* Structure version number */
  LDAP *	objectDirectory;/* LDAP Instance */
  MQLONG    ldapVersion;	/* Which LDAP version to use? */
  MQLONG    port;			/* Port number for LDAP server*/
  MQLONG    sizeLimit;		/* Size limit */
  MQBOOL	ssl;			/* SSL enabled? */
  MQCHAR *	host;			/* Hostname of LDAP server */
  MQCHAR *	password;		/* Password of LDAP server */
  MQCHAR *	searchFilter;   /* LDAP search filter */
  MQCHAR *	baseDN;			/* Base Distinguished Name */
  MQCHAR *	charSet;		/* Character set */
};

/* Internal constants*/
#define MQNLDAPCTX_VERSION_1       1
#define MQNLDAPCTX_CURRENT_VERSION 1

/* Schema member names */
#define PPT_IBM_AMQ_CHANNEL_NAME                  "ibm-amqChannelName"
#define PPT_IBM_AMQ_CONNECTION_NAME               "ibm-amqConnectionName"
#define PPT_IBM_AMQ_DESCRIPTION                   "ibm-amqDescription"
#define PPT_IBM_AMQ_LOCAL_ADDRESS                 "ibm-amqLocalAddress"
#define PPT_IBM_AMQ_MODE_NAME                     "ibm-amqModeName"
#define PPT_IBM_AMQ_PASSWORD                      "ibm-amqPassword"
#define PPT_IBM_AMQ_QUEUE_MANAGER_NAME            "ibm-amqQueueManagerName"
#define PPT_IBM_AMQ_RECEIVE_EXIT_NAME             "ibm-amqReceiveExitName"
#define PPT_IBM_AMQ_RECEIVE_EXIT_USER_DATA        "ibm-amqReceiveExitUserData"
#define PPT_IBM_AMQ_SECURITY_EXIT_NAME            "ibm-amqSecurityExitName"
#define PPT_IBM_AMQ_SECURITY_EXIT_USER_DATA       "ibm-amqSecurityExitUserData"
#define PPT_IBM_AMQ_SEND_EXIT_NAME                "ibm-amqSendExitName"
#define PPT_IBM_AMQ_SEND_EXIT_USER_DATA           "ibm-amqSendExitUserData"
#define PPT_IBM_AMQ_SSL_CIPHER_SPEC               "ibm-amqSslCipherSpec"
#define PPT_IBM_AMQ_SSL_PEER_NAME                 "ibm-amqSslPeerName"
#define PPT_IBM_AMQ_TRANSACTION_PROGRAM_NAME      "ibm-amqTransactionProgramName"
#define PPT_IBM_AMQ_CONNECTION_AFFINITY           "ibm-amqConnectionAffinity"
#define PPT_IBM_AMQ_CLIENT_CHANNEL_WEIGHT         "ibm-amqClientChannelWeight"
#define PPT_IBM_AMQ_HEADER_COMPRESSION            "ibm-amqHeaderCompression"
#define PPT_IBM_AMQ_MESSAGE_COMPRESSION           "ibm-amqMessageCompression"
#define PPT_IBM_AMQ_HEART_BEAT_INTERVAL           "ibm-amqHeartBeatInterval"
#define PPT_IBM_AMQ_KEEP_ALIVE_INTERVAL           "ibm-amqKeepAliveInterval"
#define PPT_IBM_AMQ_MAXIMUM_MESSAGE_LENGTH        "ibm-amqMaximumMessageLength"
#define PPT_IBM_AMQ_SHARING_CONVERSATIONS         "ibm-amqSharingConversations"
#define PPT_IBM_AMQ_TRANSPORT_TYPE                "ibm-amqTransportType"


/* Internal routines */
MQLONG amqLdapContextInit(PMQNXP  pExitParms);
MQLONG amqLdapContextDispose(PMQNXP pExitParms);
MQLONG amqLdapConnect ( PMQNXP pExitParms );
MQLONG amqLdapParseURI ( PMQNXP pExitParms );
MQLONG amqLdapLookup ( PMQNXP pExitParms
                     , PMQCHAR pQMgrName
                     , PMQCNO  pConnectOpts);
MQLONG amqLdapMakeChannelDefinition ( PMQNXP pExitParms
                                    , LDAPMessage * entry
                                    , LDAP * ld
                                    , MQLONG MaxMQCDVersion
                                    , MQCD * pMQCD);
MQLONG amqLdapProcessExitNames ( PMQNXP pExitParms
                               , PMQCHAR *pExitPtr
                               , PMQCHAR pValue
                               , MQLONG exitLength
                               , PMQLONG exitnum);


/*********************************************************************/
/*                                                                   */
/* Function: LdapPreconnectExit                                      */
/*                                                                   */
/* Description: Main entry point for LDAP Connection endpoint lookup */
/*              functionality. This function calls other routines    */
/*              for the actual job.                                  */
/* Intended Function: Main entry point                               */
/*                                                                   */
/* Input Parameters: pExitParms - Pointer to an MQNXP structure      */
/*                                containing LDAP server information */
/*                                and lookup information.            */
/*                   pQMgrName  - Name of the QMgr whose connection  */
/*                                end points are required.           */
/*                   ppConnectOpts - MQCNO options. Not used by this */
/*                                exit. Hence remains unmodified.    */
/*                   pCompCode  - MQ Completion code.                */
/*                   pReasonCode - MQ Reason code.                   */
/*                                                                   */
/* Output Parameters: Depends on the Exit Reason.                    */
/*                   MQXR_INIT - Initializes Context information and */
/*                               connects to the given LDAP server.  */
/*                   MQXR_TERM - Frees up allocated resources and    */
/*                               disconnects from LDAP Server.       */
/*                   MQXR_PRECONNECT - returns an array of MQCD Ptrs */
/*                               if found.                           */
/*                                                                   */
/* Returns: void                                                     */
/*********************************************************************/
void MQENTRY LdapPreconnectExit ( PMQNXP  pExitParms
                                , PMQCHAR pQMgrName
                                , PPMQCNO ppConnectOpts
                                , PMQLONG pCompCode
                                , PMQLONG pReason)
{
  MQLONG rc = MQCC_OK;

  if(pExitParms == NULL)
  {
    *pCompCode = MQXCC_FAILED;
    *pReason = MQRC_API_EXIT_ERROR;
    goto MOD_EXIT;
  }

  /* Initialize ExitResponse and ExitResponse2 */
  pExitParms->ExitResponse = MQXCC_OK;
  pExitParms->ExitResponse2 = MQXR2_DEFAULT_CONTINUATION;

  /* What job are we doing */
  switch(pExitParms->ExitReason)
  {
    case MQXR_INIT:
    {
      /* Initialize context and connect to LDAP Server */
      rc = amqLdapContextInit(pExitParms);
      if( rc == MQCC_OK )
      {
        rc = amqLdapConnect(pExitParms);
      }
      /* We would have set the ExitResponse above. So just */
      /*set CC and RC here                                */
      if( rc != MQCC_OK )
      {
        *pCompCode = MQXCC_FAILED;
        *pReason = rc;
      }
    }
    break;

    case MQXR_TERM:
    {
      amqLdapContextDispose(pExitParms);
    }
    break;

    case MQXR_PRECONNECT:
    {
      amqLdapLookup(pExitParms,pQMgrName, *ppConnectOpts);
    }
    break;

    default:
    {
       pExitParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;
      *pCompCode = MQCC_FAILED;
      *pReason = MQRC_API_EXIT_ERROR;
    }
  }

MOD_EXIT:
  return;
}


/*********************************************************************/
/*                                                                   */
/* Function: amqLdapConnect                                          */
/*                                                                   */
/* Description:  Connects to the given ldap server.                  */
/*                                                                   */
/* Input Parameters: pExitParms - Exit parameter information.        */
/*                                                                   */
/* Output Parameters:                                                */
/*                                                                   */
/* Returns: Return code indicating success or failure.               */
/*********************************************************************/
MQLONG amqLdapConnect(PMQNXP pExitParms)
{
  MQLONG          rc                   = MQCC_OK;
  MQLONG          rc2                  = MQCC_OK;
  MQLONG          failureReasonCode    = 0;
  MQLONG          authmethod           = 0;
  MQLONG          msgidp               = 0;
  MQLONG          controlres           = 0;
  MQLONG          controlerr           = 0;
  MQLONG          controlwarn          = 0;
  MQLONG          deref                = 0;
  MQLONG          timelimit            = 0;
  MQLONG          hoplimit             = 10;
  MQLONG          DebugLevel           = 0;
  MQLONG          referrals            = 0;
  MQCHAR  *      bindDN               = NULL;
  LDAPControl **  returnedControls     = NULL;
  LDAPControl **  bind_client_controls = NULL;
  LDAPMessage *   res                  = NULL;
  struct berval   ber                  = { 0 };
  struct berval * server_creds         = NULL;
  MQCHAR *       mech                 = NULL;
  MQCHAR *       digest_username      = NULL;
  MQCHAR *       digest_realm         = NULL;
  LDAPControl **  bindControls         = NULL;
  PMQNLDAPCTX pLdapContext = (PMQNLDAPCTX)pExitParms->pExitUserAreaPtr;


  /* Check whether we are attempting to use an SSL connection type? */
  if (pLdapContext->ssl)
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_FAILED;
    goto MOD_EXIT;
  }

  /* Check the LDAP version requirements and initialise/open the connection. */
  if (pLdapContext->ldapVersion == LDAP_VERSION3)
  {
    if ((pLdapContext->objectDirectory = ldap_init( pLdapContext->host
                                                  , pLdapContext->port)) == NULL)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
  }
  else if (pLdapContext->ldapVersion == LDAP_VERSION2)
  {
    if ((pLdapContext->objectDirectory = ldap_open( pLdapContext->host
                                                  , pLdapContext->port)) == NULL)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
  }
  else
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
    pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    goto MOD_EXIT;
  }

  /* Set options (depending upon LDAP version). */
  if (pLdapContext->ldapVersion == LDAP_VERSION3)
  {
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_PROTOCOL_VERSION
                   , (void *) &(pLdapContext->ldapVersion));
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_DEBUG
                   , (void *) &DebugLevel);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_DEREF
                   , (void *) &deref);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_TIMELIMIT
                   , (void *) &timelimit);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_SIZELIMIT
                   , (void *) &pLdapContext->sizeLimit);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_REFHOPLIMIT
                   , (void *) &hoplimit);
    ldap_set_option(pLdapContext->objectDirectory
                   , LDAP_OPT_REFERRALS
                   , (void *) &referrals);
  }
  else
  {
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_PROTOCOL_VERSION
                   , (void *) &(pLdapContext->ldapVersion));
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_DEBUG
                   , (void *) &DebugLevel);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_DEREF
                   , (void *) &deref);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_TIMELIMIT
                   , (void *) &timelimit);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_SIZELIMIT
                   , (void *) &(pLdapContext->sizeLimit));
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_REFHOPLIMIT
                   , (void *) &hoplimit);
    ldap_set_option( pLdapContext->objectDirectory
                   , LDAP_OPT_REFERRALS
                   , (void *) &referrals);
  }

  /* Set the LDAP character set and convert as necessary.  */
  if (pLdapContext->charSet != NULL)
  {
    rc2 = ldap_set_iconv_local_charset(pLdapContext->charSet);

    if (rc2 != MQCC_OK)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
    else
    {
      rc2 = ldap_set_option( pLdapContext->objectDirectory
                           , LDAP_OPT_UTF8_IO
                           , (void *) LDAP_UTF8_XLATE_ON);

      if (rc2 != MQCC_OK)
      {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
      }
    }
  }

  /*  Depending upon the LDAP version, bind to the directory as necessary. */
  if (pLdapContext->ldapVersion == LDAP_VERSION3)
  {
    /* Simple bind */
    if (!mech)
    {
      ber.bv_len = (ber_len_t) strlen (pLdapContext->password);
      ber.bv_val = pLdapContext->password;

      rc2 = ldap_sasl_bind( pLdapContext->objectDirectory
                         , bindDN, mech, &ber
                         , bindControls, NULL,(int *) &msgidp);

      if (rc2 != MQCC_OK)
      {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
      }
      else
      {
        ldap_result(pLdapContext->objectDirectory, msgidp, 1, NULL, &res);
        rc2 = ldap_parse_result( pLdapContext->objectDirectory
                              , res, NULL, NULL, NULL, NULL
                              , &returnedControls, 1);

        if (rc2 != MQCC_OK)
        {
          rc = MQRC_PRECONN_EXIT_ERROR;
          pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
          pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
          goto MOD_EXIT;
        }
      }
    }
    /* Presence of mechanism means SASL bind  */
    else
    {
      /*
       * Special case for mech="EXTERNAL".  Unconditionally set bind DN
       * and credentials to NULL.  This option should be used in tandem
       * with SSL and client authentication.  For other SASL mechanisms,
       * use the specified bind DN and credentials.
       */
      if (strcmp(mech, LDAP_MECHANISM_EXTERNAL) == 0)
      {
        /* SASL bind */
        rc2 = ldap_sasl_bind_s( pLdapContext->objectDirectory
                             , NULL, mech, NULL, bindControls, NULL
                             , &server_creds);


        if (rc2 != MQCC_OK)
        {
          rc = MQRC_PRECONN_EXIT_ERROR;
          pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
          pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
          goto MOD_EXIT;
        }
      }
      else if (strcmp(mech, LDAP_MECHANISM_GSSAPI) == 0)
      {
        /*  SASL bind S */
        rc2 = ldap_sasl_bind_s( pLdapContext->objectDirectory
                              , NULL, mech, NULL, bindControls, NULL
                              , &server_creds);


        if (rc2 != MQCC_OK)
        {
          ldap_get_option( pLdapContext->objectDirectory
                         , LDAP_OPT_EXT_GSS_ERR
                         , &failureReasonCode);
          rc = MQRC_PRECONN_EXIT_ERROR;
          pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
          pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
          goto MOD_EXIT;
        }
      }
      else
      {
        /*  Other SASL mechanisms */
        if (strcmp(mech, LDAP_MECHANISM_DIGEST_MD5) == 0)
        {
          if (digest_username)
          {
            rc2 = ldap_add_control( IBM_CLIENT_DIGEST_USER_NAME_OID
                                  , (ber_len_t) strlen(digest_username)
                                  , digest_username
                                  , LDAP_TRUE
                                  , &bind_client_controls);

            if(rc2 != MQCC_OK)
            {
              rc = MQRC_PRECONN_EXIT_ERROR;
              pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
              pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
              goto MOD_EXIT;
            }
          }

          if (digest_realm && (rc2 == MQCC_OK))
          {
            rc2 = ldap_add_control( IBM_CLIENT_DIGEST_REALM_NAME_OID
                                  , (ber_len_t) strlen(digest_realm)
                                  , digest_realm, LDAP_TRUE
                                  , &bind_client_controls);

            if(rc2 != MQCC_OK)
            {
              rc = MQRC_PRECONN_EXIT_ERROR;
              pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
              pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
              goto MOD_EXIT;
            }
          }
        }

        if (rc2 == MQCC_OK)
        {
          ber.bv_len = (ber_len_t) strlen(pLdapContext->password);
          ber.bv_val = pLdapContext->password;


          rc2 = ldap_sasl_bind_s( pLdapContext->objectDirectory
                               , bindDN, mech, &ber, bindControls
                               , bind_client_controls, &server_creds);

          if (rc2 != MQCC_OK)
          {
            rc = MQRC_PRECONN_EXIT_ERROR;
            pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
            pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
            goto MOD_EXIT;
          }
        }
      }
    }

    /*  Get any controls sent by the server during the sasl_bind_s */
    rc2 = ldap_get_bind_controls( pLdapContext->objectDirectory
                                , &returnedControls);

    if (rc2 != MQCC_OK)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
    else
    {
      /* Free the client controls */
      ldap_controls_free(bind_client_controls);

      if (returnedControls != NULL)
      {
        ldap_parse_pwdpolicy_response( returnedControls
                                     , (int *)&controlerr
                                     , (int *)&controlwarn
                                     , (int *)&controlres);
      }
    }
  }
  else if (pLdapContext->ldapVersion == LDAP_VERSION2)
  {
    authmethod = LDAP_AUTH_SIMPLE;
    rc2  = ldap_bind_s( pLdapContext->objectDirectory
                      , bindDN, pLdapContext->password
                      , authmethod);

    if (rc2 != MQCC_OK)
    {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
    }
  }

  /* Check that we have a valid object directiory? */
  if (pLdapContext->objectDirectory == NULL)
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
    pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    goto MOD_EXIT;
  }

MOD_EXIT:
  return(rc);
}

/*********************************************************************/
/*                                                                   */
/* Function: amqLdapConvert                                          */
/*                                                                   */
/* Description: Searches a comma separted string and converts the    */
/*              found string into numeral.                           */
/*                                                                   */
/* Input Parameters: Comma seperated string.                         */
/*                                                                   */
/* Output Parameters: Pointer to Long for returning converted value. */
/*                                                                   */
/* Returns: String pointer poniting next character after comma       */
/*********************************************************************/
PMQCHAR amqLdapConvert(PMQCHAR pListStr, MQLONG *retVal)
{
  PMQCHAR pList = pListStr;
  MQCHAR headerCompValue[8];
  int length;
  PMQCHAR pToken;
  *retVal = -1;

  if(pListStr)
  {
    pToken = strchr(pList,',');
    if(pToken)
    {
      length = (int)(pToken - pList);
      strncpy(headerCompValue, pList, length);
      headerCompValue[length] = '\0';
      *retVal = atoi(headerCompValue);
      return ++pToken;
    }
  }
  return NULL;
}


/*********************************************************************/
/*                                                                   */
/* Function: amqLdapMakeChannelDefinition                            */
/*                                                                   */
/* Description: Prepares a MQCD structure with the given LDAP        */
/*              record.                                              */
/*                                                                   */
/* Input Parameters: entry - Output of a LDAP query.                 */
/*                   ld - LDAP connection info.                      */
/*                   MaxMQCDVersion - MQCD version for which MQCD    */
/*                                    will be prepared.              */
/*                                                                   */
/* Output Parameters: pMQCD - Buffer containing the prepared MQCD.   */
/*                                                                   */
/* Returns: Reason code                                              */
/*********************************************************************/
MQLONG amqLdapMakeChannelDefinition( PMQNXP pExitParms
                                   , LDAPMessage * entry
                                   , LDAP * ld
                                   , MQLONG MaxMQCDVersion
                                   , MQCD * pMQCD)
{
  MQLONG             rc   = MQCC_OK;
  MQCHAR *           pAttr = NULL;
  MQCHAR **          pVals = NULL;
  BerElement     *   ber = NULL;
  struct berval **   bvals = NULL;
  MQCD DefMQCD = { MQCD_DEFAULT };
  int index;
  int comprIndex;
  MQLONG  exitCount = 0;


  /* Initialize MQCD with defaults */
  memcpy(pMQCD, &DefMQCD, sizeof(MQCD));

  pMQCD->Version = MaxMQCDVersion;
  pMQCD->ChannelType = MQCHT_CLNTCONN;
  pMQCD->TransportType = MQXPT_TCP;

  /* Now go for the second pass to retrieve all other attributes. */
  for ( pAttr = ldap_first_attribute(ld, entry, &ber); ((rc==MQCC_OK) && (pAttr != NULL));
        pAttr = ldap_next_attribute(ld, entry, ber))
  {
    if ((bvals = ldap_get_values_len(ld, entry, pAttr)) != NULL)
    {
      pVals = ldap_get_values(ld, entry, pAttr);
      for (index = 0; ((rc==MQCC_OK) && (bvals[index] != NULL)); index++)
      {
        switch(MaxMQCDVersion)
        {
          /* MQCD version */
        case MQCD_VERSION_11:
          {
          }

          /* Deliberate fall thru */
        case MQCD_VERSION_10:
          {
          }

        case MQCD_VERSION_9:
          {
            /* Sharing Conversations */
            if(strcmp(pAttr,PPT_IBM_AMQ_SHARING_CONVERSATIONS) == 0)
            {
              pMQCD->SharingConversations = atoi(pVals[index]);
            }
            /* Connection affinity*/
            if(strcmp(pAttr,PPT_IBM_AMQ_CONNECTION_AFFINITY) == 0)
            {
              pMQCD->ConnectionAffinity = atoi(pVals[index]);
            }

            /* Channel weight */
            if(strcmp(pAttr,PPT_IBM_AMQ_CLIENT_CHANNEL_WEIGHT) == 0)
            {
              pMQCD->ClientChannelWeight = atoi(pVals[index]);
            }
          }

        /* Deliberate fall thru */
        case MQCD_VERSION_8:
          {
          }

        /* Deliberate fall thru */
        case MQCD_VERSION_7:
          {
            /* Keep alive interval */
            if(strcmp(pAttr,PPT_IBM_AMQ_KEEP_ALIVE_INTERVAL) == 0)
            {
              pMQCD->KeepAliveInterval = atoi(pVals[index]);
            }

            /* Header compression. This can have a maximum of two values */
            if(strcmp(pAttr,PPT_IBM_AMQ_HEADER_COMPRESSION) == 0)
            {
              /* This will be comma separated list */
              MQLONG headerCompression = 0;
              PMQCHAR pToken = pVals[index];
              for(comprIndex = 0; comprIndex < 2; comprIndex++)
              {
                pToken = amqLdapConvert(pToken,&headerCompression);
                switch(headerCompression)
                {
                case MQCOMPRESS_NONE:
                case MQCOMPRESS_SYSTEM:
                  pMQCD->HdrCompList[comprIndex] = headerCompression;
                  break;

                default:
                  break;
                }
              }
            }

            /* Message compression. This can have a maximum of 16 values.*/
            if(strcmp(pAttr,PPT_IBM_AMQ_MESSAGE_COMPRESSION) == 0)
            {
              /* This will be comma separated list */
              MQLONG messageCompression = 0;
              PMQCHAR pToken = pVals[index];
              for(comprIndex = 0; comprIndex < 16; comprIndex++)
              {
                pToken = amqLdapConvert(pToken,&messageCompression);
                switch(messageCompression)
                {
                case MQCOMPRESS_NONE:
                case MQCOMPRESS_RLE:
                case MQCOMPRESS_ZLIBFAST:
                case MQCOMPRESS_ZLIBHIGH:
                case MQCOMPRESS_LZ4FAST:
                case MQCOMPRESS_LZ4HIGH:
                case MQCOMPRESS_NOT_AVAILABLE:
                  pMQCD->MsgCompList[comprIndex] = messageCompression;
                  break;

                default:
                  break;
                }
              }
            }
          }

        /* Deliberate fall thru */
        case MQCD_VERSION_6:
          {
            /* Local address */
            if(strcmp(pAttr,PPT_IBM_AMQ_LOCAL_ADDRESS) == 0)
            {
              strncpy ( pMQCD->LocalAddress
                      , pVals[index]
                      , sizeof(pMQCD->LocalAddress));
            }

            /* Cipherspec */
            if(strcmp(pAttr,PPT_IBM_AMQ_SSL_CIPHER_SPEC) == 0)
            {
              strncpy ( pMQCD->SSLCipherSpec
                      , pVals[index]
                      , sizeof(pMQCD->SSLCipherSpec));
            }

            /* SSL Peer name */
            if(strcmp(pAttr,PPT_IBM_AMQ_SSL_PEER_NAME) == 0)
            {
              int sslPeerNameLength = (int)strlen(pVals[index]);
              MQCHAR * sslPeerName = NULL;

              if(sslPeerNameLength > 0)
              {
                sslPeerName = (MQCHAR *)malloc(sslPeerNameLength+1);
                if(sslPeerName == NULL)
                {
                    rc = MQRC_PRECONN_EXIT_ERROR;
                    break;
                }
                strcpy ( sslPeerName , pVals[index] );
                pMQCD->SSLPeerNamePtr = sslPeerName;
                pMQCD->SSLPeerNameLength = sslPeerNameLength;
              }
            }
          }

        /* Deliberate fall thru */

        case MQCD_VERSION_5:

        /* Deliberate fall thru */
        case MQCD_VERSION_4:
          {
            /* Heartbeat Intervals*/
            if(strcmp(pAttr,PPT_IBM_AMQ_HEART_BEAT_INTERVAL) == 0)
            {
              pMQCD->HeartbeatInterval = atoi(pVals[index]);
            }
          }

        /* Deliberate fall thru */
        case MQCD_VERSION_3:

        /* Deliberate fall thru */
        case MQCD_VERSION_2:
          {
            /* Connection name */
            if(strcmp(pAttr,PPT_IBM_AMQ_CONNECTION_NAME) == 0)
            {
              strncpy(pMQCD->ConnectionName, pVals[index], sizeof(pMQCD->ConnectionName));
            }
            /* Password */
            if(strcmp(pAttr,PPT_IBM_AMQ_PASSWORD) == 0)
            {
              strncpy(pMQCD->Password, pVals[index], sizeof(pMQCD->Password));
            }
          }

        /* Deliberate fall thru */
        case MQCD_VERSION_1:
          {
            /* Channel name */
            if(strcmp(pAttr,PPT_IBM_AMQ_CHANNEL_NAME) == 0)
            {
              strncpy(pMQCD->ChannelName, pVals[index], sizeof(pMQCD->ChannelName));
            }

            /* Description */
            if(strcmp(pAttr,PPT_IBM_AMQ_DESCRIPTION) == 0)
            {
              strncpy(pMQCD->Desc, pVals[index], sizeof(pMQCD->Desc));
            }

            /* Queue manager name */
            if(strcmp(pAttr,PPT_IBM_AMQ_QUEUE_MANAGER_NAME) == 0)
            {
              strncpy(pMQCD->QMgrName, pVals[index], sizeof(pMQCD->QMgrName));
            }

            /* Mode name */
            if(strcmp(pAttr,PPT_IBM_AMQ_MODE_NAME) == 0)
            {
              strncpy(pMQCD->ModeName, pVals[index], sizeof(pMQCD->ModeName));
            }

            /* Process Exits. */
            if(strcmp(pAttr,PPT_IBM_AMQ_RECEIVE_EXIT_NAME) == 0)
            {
              /* If there are multiple exits specified, then we need to set */
              if(strchr(pVals[index], ','))
              {
                rc = amqLdapProcessExitNames ( pExitParms
                                                    , (PMQCHAR *)&(pMQCD->ReceiveExitPtr)
                                                    , pVals[index]
                                                    , MQ_EXIT_NAME_LENGTH
                                                    , &exitCount);
                if(rc != MQCC_OK)
                {
                    break;
                }
                  pMQCD->ReceiveExitsDefined = exitCount;
                  pMQCD->ExitNameLength = MQ_EXIT_NAME_LENGTH;
              }
              else
              {
                /* No, only one value. Then simply copy it. */
                strncpy(pMQCD->ReceiveExit, pVals[index], sizeof(pMQCD->ReceiveExit));
              }
            }

            /* Set Receive exit data if a receive exit has been specified. */
            if(strcmp(pAttr,PPT_IBM_AMQ_RECEIVE_EXIT_USER_DATA) == 0)
            {
              if(strchr(pVals[index], ','))
              {
                rc = amqLdapProcessExitNames ( pExitParms
                                        , (PMQCHAR *)&(pMQCD->ReceiveUserDataPtr)
                                        , pVals[index]
                                        , MQ_EXIT_DATA_LENGTH
                                        , &exitCount );
                if(rc != MQCC_OK)
                {
                    break;
                }
                pMQCD->ExitDataLength = MQ_EXIT_DATA_LENGTH;
              }
              else
              {
                strncpy ( pMQCD->ReceiveUserData
                        , pVals[index]
                        , sizeof(pMQCD->ReceiveUserData));
              }
            }

            /* Security exit name */
            if(strcmp(pAttr,PPT_IBM_AMQ_SECURITY_EXIT_NAME) == 0)
            {
              strncpy ( pMQCD->SecurityExit
                      , pVals[index]
                      , sizeof(pMQCD->SecurityExit));
            }

            /* Security user data if a security exit has been specified */
            if(strcmp(pAttr,PPT_IBM_AMQ_SECURITY_EXIT_USER_DATA) == 0)
            {
              strncpy ( pMQCD->SecurityUserData
                      , pVals[index]
                      , sizeof(pMQCD->SecurityUserData));
            }

            /* Send exit name */
            if(strcmp(pAttr,PPT_IBM_AMQ_SEND_EXIT_NAME) == 0)
            {
              if(strchr(pVals[index], ','))
              {
                rc = amqLdapProcessExitNames (pExitParms
                                                    , (PMQCHAR *) &(pMQCD->SendExitPtr)
                                                    , pVals[index]
                                                    , MQ_EXIT_NAME_LENGTH
                                                    , &exitCount);
                if(rc != MQCC_OK)
                {
                    break;
                }
                  pMQCD->ExitNameLength = MQ_EXIT_NAME_LENGTH;
                  pMQCD->SendExitsDefined = exitCount;
              }
              else
              {
                strncpy ( pMQCD->SendExit, pVals[index], sizeof(pMQCD->SendExit));
              }
            }

            /* Set Send exit user data if Send exit has been specified */
            if(strcmp(pAttr,PPT_IBM_AMQ_SEND_EXIT_USER_DATA) == 0)
            {
              if(strchr(pVals[index], ','))
              {
                rc = amqLdapProcessExitNames ( pExitParms
                                        , (PMQCHAR *)&(pMQCD->SendUserDataPtr)
                                        , pVals[index]
                                        , MQ_EXIT_DATA_LENGTH
                                        , &exitCount);
                if(rc != MQCC_OK)
                {
                    break;
                }
                pMQCD->ExitDataLength = MQ_EXIT_DATA_LENGTH;
              }
              else
              {
                strncpy ( pMQCD->SendUserData
                        , pVals[index]
                        , sizeof(pMQCD->SendUserData));
              }
            }

            /* Transaction program name */
            if(strcmp(pAttr,PPT_IBM_AMQ_TRANSACTION_PROGRAM_NAME) == 0)
            {
              strncpy(pMQCD->TpName, pVals[index], sizeof(pMQCD->TpName));
            }

            /* Maximum message length */
            if(strcmp(pAttr,PPT_IBM_AMQ_MAXIMUM_MESSAGE_LENGTH) == 0)
            {
              pMQCD->MaxMsgLength = atoi(pVals[index]);
            }

            /* Transport type */
            if(strcmp(pAttr,PPT_IBM_AMQ_TRANSPORT_TYPE) == 0)
            {
              pMQCD->TransportType = atoi(pVals[index]);
            }
          }
          break;

        default:
          {
            rc = MQRC_CHANNEL_NOT_AVAILABLE;
          }
          break;
        }
      }

      ldap_value_free_len(bvals);
      ldap_value_free(pVals);
    }

    ldap_memfree(pAttr);
  }

  if (ber != NULL)
  {
      ldap_ber_free(ber);
  }

  return(rc);
}


/*********************************************************************/
/*                                                                   */
/* Function:amqLdapLookup                                            */
/*                                                                   */
/* Description: Lookup LDAP Server with given query.                 */
/*                                                                   */
/* Input Parameters: pExitParms - Exit information.                  */
/*                   pQMgrName - Queue manager name whose connection */
/*                               need to be retrieved.               */
/*                   pConnectOpts - MQCNO. Untouched.                */
/*                                                                   */
/* Returns: Reason code                                              */
/*********************************************************************/
MQLONG amqLdapLookup ( PMQNXP pExitParms
                     , PMQCHAR pQMgrName
                     , PMQCNO pConnectOpts)
{
  MQLONG         rc  = MQCC_OK;
  MQLONG         matches    = 0;
  LDAPMessage *  res              = NULL;
  LDAPMessage *  e                = NULL;
  MQLONG         scope            = LDAP_SCOPE_SUBTREE;
  MQLONG         attrsonly        = 0;
  MQCHAR **      attrs            = NULL;
  LDAPControl ** opControls       = NULL;
  PMQNLDAPCTX pLdapContext = (PMQNLDAPCTX)(pExitParms->pExitUserAreaPtr);
  int index = 0;
  int newIndex = 0;


  if (pLdapContext == NULL)
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
    pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    goto MOD_EXIT;
  }

  /* Check for filter. Qmgr name is a must. It could be blank or wildcard */
  /* characters                                                           */
  if (pQMgrName == NULL)
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
    pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    goto MOD_EXIT;
  }
  else
  {
    MQCHAR *pSearchFilter = NULL;
    MQCHAR *pObjectClass = "objectclass=ibm-amqClientConnection";
    MQCHAR *pLDAPQMName = "ibm-amqQueueManagerName=";
    int bufferLength;
    int qMgrNameLength = (int)strlen(pQMgrName);


    /* If the queue manager name begins with * or a blank, then we will   */
    /* search LDAP server for all entries whose 'ibm-amqIsClientDefault'  */
    /* attribute value is set to true.                                    */
    if(  (qMgrNameLength == 0) ||
         (
             ( qMgrNameLength == 1) &&
             ( (pQMgrName[0] == '*' ) || (pQMgrName[0] == ' '))
         )
      )
    {
      /* Search filter format:
         (&( objectclass=ibm-amqClientConnection)(ibm-amqIsClientDefault=true))
      */

      bufferLength = (int)strlen("(&( objectclass=ibm-amqClientConnection)(ibm-amqIsClientDefault=true))") + 1;
      /* Store the search filter in our search filter. */
      pSearchFilter = (PMQCHAR) malloc(sizeof(MQCHAR) * bufferLength);
      if(pSearchFilter == NULL)
      {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
      }

      strcpy( pSearchFilter
            , "(&( objectclass=ibm-amqClientConnection)(ibm-amqIsClientDefault=true))");
    }
    else
    {

      /* Skip the first * if there is one. */
      if(pQMgrName[0] == '*' )
        pQMgrName++;

      bufferLength = 67 /* length of (objectclass=ibm-amqClientConnection)
                           + (ibm-amqQueueManagerName=)*/
                           + (int)strlen(pQMgrName);

      /* Store the search filter in our search filter. */
      pSearchFilter = (PMQCHAR) malloc(sizeof(MQCHAR) * bufferLength);
      if(pSearchFilter == NULL)
      {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
      }

      sprintf( pSearchFilter, "(&(%s)(%s%s))"
             , pObjectClass
             , pLDAPQMName
             , pQMgrName);

    }

    /* Free previous filter if any */
    if(pLdapContext->searchFilter)
    {
      free(pLdapContext->searchFilter);
      pLdapContext->searchFilter = pSearchFilter;
    }
    else
    {
      pLdapContext->searchFilter = pSearchFilter;
    }
  }


  /* Now search */
  if (pLdapContext->ldapVersion == LDAP_VERSION3)
  {
    rc = ldap_search_ext_s( pLdapContext->objectDirectory
                          , pLdapContext->baseDN
                          , scope
                          , pLdapContext->searchFilter
                          , attrs
                          , attrsonly
                          , opControls
                          , NULL
                          , NULL
                          , pLdapContext->sizeLimit
                          , &res);


    if (rc != MQCC_OK)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
  }
  else if (pLdapContext->ldapVersion == LDAP_VERSION2)
  {
    rc = ldap_search_s( pLdapContext->objectDirectory
                      , pLdapContext->baseDN
                      , scope
                      , pLdapContext->searchFilter
                      , attrs
                      , attrsonly
                      , &res);


    if (rc != MQCC_OK)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
  }

  /* Count the number of entries found and allocate array of pointers*/
  matches = ldap_count_entries(pLdapContext->objectDirectory, res);

  /* Allocate for memory for the number of entries found */
  if(matches > 0)
  {
    PMQCD *pTempMQCDArray = NULL;


    /* If we already have an array MQCD, we will need to append new search */
    /* results to the existing array. Hence allocate for both existing     */
    /* array and new resuts.                                               */
    pTempMQCDArray = (PMQCD *)malloc((matches + pExitParms->MQCDArrayCount) * sizeof(MQCD *));
    if(pTempMQCDArray == NULL)
    {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
    }

    if(pExitParms->ppMQCDArrayPtr)
    {
      for(index = 0; index < pExitParms->MQCDArrayCount; index++)
      {
        /* Copy to new buffer */
        pTempMQCDArray[index] = pExitParms->ppMQCDArrayPtr[index];
      }
      free(pExitParms->ppMQCDArrayPtr);
    }
    /* Assign the new array */
    pExitParms->ppMQCDArrayPtr = pTempMQCDArray;
  }
  else
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
    pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    goto MOD_EXIT;
  }

  /*  Start parsing each entry in the search result */
  e = ldap_first_entry(pLdapContext->objectDirectory, res);
  for (newIndex = 0; newIndex < matches; index++,newIndex++)
  {

    /* Allocate for MQCD */
    pExitParms->ppMQCDArrayPtr[index] = (MQCD *) malloc(sizeof(MQCD));
    if(pExitParms->ppMQCDArrayPtr[index] == NULL)
    {
        rc = MQRC_PRECONN_EXIT_ERROR;
        pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
        pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
        goto MOD_EXIT;
    }

    // Convert the search result we got into MQCD.
    rc = amqLdapMakeChannelDefinition( pExitParms
                                     , e
                                     , pLdapContext->objectDirectory
                                     , pExitParms->MaxMQCDVersion
                                     , pExitParms->ppMQCDArrayPtr[index]);

    if (rc != MQCC_OK)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    }

    /* Get next entry */
    e = ldap_next_entry(pLdapContext->objectDirectory, e);
  }

  /* Assign the number of entries found */
  pExitParms->MQCDArrayCount += matches;
  ldap_msgfree(res);

MOD_EXIT:
  return(rc);
}

/*********************************************************************/
/*                                                                   */
/* Function: amqLdapContextInit                                      */
/*                                                                   */
/* Description: Initializes LDAP Connection information.             */
/*                                                                   */
/* Input Parameters: pExitParms - Exit informatoin.                  */
/*                                                                   */
/* Output Parameters: None                                           */
/*                                                                   */
/* Returns: Reason code                                              */
/*********************************************************************/
MQLONG amqLdapContextInit(PMQNXP pExitParms)
{
  MQLONG rc = MQCC_OK;
  PMQNLDAPCTX  pLdapContext = NULL;


  /* Free any previous instance data */
  if(pExitParms->pExitUserAreaPtr)
  {
    rc = amqLdapContextDispose(pExitParms);
    if(rc != MQCC_OK)
      goto MOD_EXIT;
  }

  /* Initialize a new context block */
  pLdapContext = (PMQNLDAPCTX)malloc (sizeof(MQNLDAPCTX));
  if(pLdapContext == NULL)
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
    pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
    goto MOD_EXIT;
  }

  /* Initialize LDAP Context */
  pLdapContext->objectDirectory = NULL;
  pLdapContext->Version         = MQNLDAPCTX_CURRENT_VERSION;
  pLdapContext->ldapVersion     = 3;
  pLdapContext->port            = 389;
  pLdapContext->ssl             = 0;
  pLdapContext->searchFilter    = NULL;
  pLdapContext->sizeLimit       = 0;
  pLdapContext->charSet         = NULL;

  pLdapContext->host = (PMQCHAR) malloc((strlen("localhost") + 1));
  if(pLdapContext->host == NULL)
  {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
  }
  strcpy(pLdapContext->host, "localhost");

  pLdapContext->password = (PMQCHAR) malloc((strlen("secret") + 1));
  if(pLdapContext->password == NULL)
  {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
  }
  strcpy(pLdapContext->password, "secret");

  pLdapContext->baseDN = (PMQCHAR) malloc((strlen("/") + 1));
  if(pLdapContext->baseDN == NULL)
  {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
  }
  strcpy(pLdapContext->baseDN, "/");

  pExitParms->pExitUserAreaPtr = (MQCHAR *)pLdapContext;

  /* Parse the given URI*/
  rc = amqLdapParseURI(pExitParms);

MOD_EXIT:
  return(rc);
}

/*********************************************************************/
/*                                                                   */
/* Function: amqLdapContextDispose                                   */
/*                                                                   */
/* Description: Frees up LDAP Context information                    */
/*                                                                   */
/* Input Parameters: pExitParms - Exit information                   */
/*                                                                   */
/* Output Parameters: None                                           */
/*                                                                   */
/* Returns: Reason code                                              */
/*********************************************************************/
MQLONG amqLdapContextDispose(PMQNXP pExitParms)
{
  MQLONG rc = MQCC_OK;
  PMQNLDAPCTX pLdapContext = (PMQNLDAPCTX)pExitParms->pExitUserAreaPtr;
  int index;

  if(!pLdapContext)
  {
    rc = MQRC_PRECONN_EXIT_ERROR;
    goto MOD_EXIT;
  }

  /* Free any associated resources */
  if (pLdapContext->host != NULL)
  {
    free(pLdapContext->host);
    pLdapContext->host = NULL;
  }

  if (pLdapContext->password != NULL)
  {
    free(pLdapContext->password);
    pLdapContext->password = NULL;
  }

  if (pLdapContext->searchFilter != NULL)
  {
    free(pLdapContext->searchFilter);
    pLdapContext->searchFilter = NULL;
  }

  if (pLdapContext->baseDN != NULL)
  {
    free(pLdapContext->baseDN);
    pLdapContext->baseDN = NULL;
  }

  /* Free memory allocated to MQCD pointers.*/
  if(pExitParms->ppMQCDArrayPtr)
  {
    for (index = 0; index < pExitParms->MQCDArrayCount; index++)
    {
      if(pExitParms->ppMQCDArrayPtr[index])
      {
        /* Free the memory we allocated for SSLPeerName */
        if((pExitParms->ppMQCDArrayPtr[index])->SSLPeerNamePtr)
        {
          free((pExitParms->ppMQCDArrayPtr[index])->SSLPeerNamePtr);
          (pExitParms->ppMQCDArrayPtr[index])->SSLPeerNamePtr = NULL;
        }

        /* Free the memory we allocated for SendExitPtr */
        if((pExitParms->ppMQCDArrayPtr[index])->SendExitPtr)
        {
          free((pExitParms->ppMQCDArrayPtr[index])->SendExitPtr);
          (pExitParms->ppMQCDArrayPtr[index])->SendExitPtr = NULL;
        }

        /* Free the memory we allocated for SendUserDataPtr */
        if((pExitParms->ppMQCDArrayPtr[index])->SendUserDataPtr)
        {
          free((pExitParms->ppMQCDArrayPtr[index])->SendUserDataPtr);
          (pExitParms->ppMQCDArrayPtr[index])->SendUserDataPtr = NULL;
        }

        /* Free the memory we allocated for ReceiveExitPtr */
        if((pExitParms->ppMQCDArrayPtr[index])->ReceiveExitPtr)
        {
          free((pExitParms->ppMQCDArrayPtr[index])->ReceiveExitPtr);
          (pExitParms->ppMQCDArrayPtr[index])->ReceiveExitPtr = NULL;
        }

        /* Free the memory we allocated for ReceiveUserDataPtr */
        if((pExitParms->ppMQCDArrayPtr[index])->ReceiveUserDataPtr)
        {
          free((pExitParms->ppMQCDArrayPtr[index])->ReceiveUserDataPtr);
          (pExitParms->ppMQCDArrayPtr[index])->ReceiveUserDataPtr = NULL;
        }

        /* Free the MQCD memory */
        free(pExitParms->ppMQCDArrayPtr[index]);
        pExitParms->ppMQCDArrayPtr[index] = NULL;
      }
    }

    free(pExitParms->ppMQCDArrayPtr);
    pExitParms->ppMQCDArrayPtr = NULL;
  }

  /* Free the instance control block  */
  if(pLdapContext)
  {
    free(pExitParms->pExitUserAreaPtr);
    pExitParms->pExitUserAreaPtr = NULL;
  }

MOD_EXIT:
  return(rc);
}


/*********************************************************************/
/*                                                                   */
/* Function:amqLdapParseURI                                          */
/*                                                                   */
/* Description: Parses the given URI into Host/Port/DN               */
/*                                                                   */
/* Input Parameters: pExitParms - Exit information.                  */
/*                                                                   */
/* Output Parameters: None                                           */
/*                                                                   */
/* Returns: Reason code.                                             */
/*********************************************************************/
MQLONG amqLdapParseURI(PMQNXP  pExitParms)
{
  MQLONG rc = MQCC_OK;
  LDAPURLDesc * pludpp = NULL;
  PMQNLDAPCTX  pLdapContext = (PMQNLDAPCTX)pExitParms->pExitUserAreaPtr;
  MQCHAR *ldapURI = (MQCHAR *)pExitParms->pExitDataPtr;


  /* Parse the given ldap uri*/
  rc = ldap_url_parse(ldapURI,&pludpp);
  if(rc != MQCC_OK)
  {
    goto MOD_EXIT;
  }

  /* Make copy of the parsed URI */
  if (pludpp->lud_host != NULL)
  {
    /* Host */
    if (pLdapContext->host != NULL)
      free(pLdapContext->host);

    pLdapContext->host = (PMQCHAR)malloc(strlen(pludpp->lud_host) + 1);
    if(pLdapContext->host == NULL)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
    else
    {
      strcpy(pLdapContext->host, pludpp->lud_host);
    }
  }

  /* Port */
  if (pludpp->lud_port > 0)
  {
    pLdapContext->port = pludpp->lud_port;
  }
  else
  {
    pLdapContext->port = 389;
  }

  /* If there is already a base from a previous lookup, free the current */
  /* resources before allocating memory for the new base.                */
  if (pludpp->lud_dn!= NULL)
  {
    if ((pLdapContext->baseDN) != NULL)
      free(pLdapContext->baseDN);

    pLdapContext->baseDN = (PMQCHAR) malloc(strlen(pludpp->lud_dn) + 1);
    if(pLdapContext->baseDN == NULL)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
    else
    {
      strcpy(pLdapContext->baseDN,pludpp->lud_dn);
    }
  }

  /* Search filter */
  if(pludpp->lud_filter != NULL)
  {
    if(pLdapContext->searchFilter)
      free(pLdapContext->searchFilter);

    pLdapContext->searchFilter = (PMQCHAR)malloc(strlen(pludpp->lud_filter) + 1);
    if(pLdapContext->searchFilter == NULL)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      pExitParms->ExitResponse = MQXCC_SUPPRESS_EXIT;
      pExitParms->ExitResponse2 = MQXR2_CONTINUE_CHAIN;
      goto MOD_EXIT;
    }
    else
    {
      strcpy(pLdapContext->searchFilter, pludpp->lud_filter);
    }
  }


MOD_EXIT:
  if(rc != MQCC_OK)
  {
    /* Parsing failed. Free up allocated memory*/
    if(pLdapContext->host)
    {
      free(pLdapContext->host);
      pLdapContext->host = NULL;
    }

    if(pLdapContext->baseDN)
    {
      free(pLdapContext->baseDN);
      pLdapContext->baseDN = NULL;
    }

    if(pLdapContext->searchFilter)
    {
      free(pLdapContext->searchFilter);
      pLdapContext->searchFilter = NULL;
    }
  }

  return rc;
}

/*********************************************************************/
/*                                                                   */
/* Function: amqLdapProcessExitNames                                 */
/*                                                                   */
/* Description: Process comma separated string containing exit       */
/*              names and associated exit data.                      */
/*                                                                   */
/* Input Parameters: Command seperated exit names, exit data         */
/*                                                                   */
/* Output Parameters:                                                */
/*                                                                   */
/* Returns: Reason code.                                             */
/*********************************************************************/
MQLONG amqLdapProcessExitNames ( PMQNXP pExitParms
                               , PMQCHAR *pExitPtr
                               , PMQCHAR pValue
                               , MQLONG exitLength
                               , PMQLONG exitnum)
{
  PMQCHAR pComma = NULL;
  PMQCHAR pTempValue = pValue;
  PMQCHAR pMultiExit = NULL;
  MQLONG rc = MQCC_OK;
  int exitCount = 0;
  int buffLength;
  int offSet = 0;


  *exitnum = 0;
  /* If there are multiple exits specified, then we need to set */
  pComma = strchr(pTempValue, ',');
  if(pComma)
  {
    /* Tokenize and find the longest value */
    pComma = strtok (pTempValue,",");
    while (pComma != NULL)
    {
      exitCount++;
      pComma = strtok (NULL, ",");
    }

    /* Allocate the required buffer length */
    buffLength = exitCount * exitLength;
    pMultiExit = (PMQCHAR) malloc(buffLength);
    if(pMultiExit == NULL)
    {
      rc = MQRC_PRECONN_EXIT_ERROR;
      goto MOD_EXIT;
    }
    memset(pMultiExit,' ', buffLength);

    /* Second pass. Now copy the names. */
    pComma = strtok (pTempValue,",");
    while (pComma != NULL)
    {
      strcpy((&pMultiExit[offSet]),pComma);
      offSet += exitLength;
      pComma = strtok (NULL, ",");
    }

    /* Now assign it to MQCD Exit field */
    *pExitPtr = pMultiExit;
  }

  *exitnum = exitCount;

MOD_EXIT:
  return rc;
}
/*********************************************************************/
/* End of LDAP Connection Endpoint lookup routines                   */
/*********************************************************************/
