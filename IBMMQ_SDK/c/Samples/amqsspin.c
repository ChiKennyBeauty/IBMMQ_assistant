/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/amqsspin.c */
/**********************************************************************/
/*                                                                    */
/* Module Name: AMQSSPIN.C                                            */
/*                                                                    */
/* Description: SSPI Channel Exit routines (Security)                 */
/*                                                                    */
/*   <copyright                                                       */
/*   notice="lm-source-program"                                       */
/*   pids="5724-H72,"                                                 */
/*   years="1994,2019"                                                */
/*   crc="3588738763" >                                               */
/*   Licensed Materials - Property of IBM                             */
/*                                                                    */
/*   5724-H72,                                                        */
/*                                                                    */
/*   (C) Copyright IBM Corp. 1994, 2019 All Rights Reserved.          */
/*                                                                    */
/*   US Government Users Restricted Rights - Use, duplication or      */
/*   disclosure restricted by GSA ADP Schedule Contract with          */
/*   IBM Corp.                                                        */
/*   </copyright>                                                     */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*  This file contains the SSPI Channel exit routines for MQ.         */
/*                                                                    */
/*  This module contains two main entry points :                      */
/*                                                                    */
/*    MQENTRY SCY_NTLM (Security)                                     */
/*    MQENTRY SCY_KERBEROS (Security)                                 */
/*                                                                    */
/*  These functions call the various internal routines depending on   */
/*  the reason they were invoked.                                     */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Notes:                                                             */
/*   Language:     C                                                  */
/*                                                                    */
/*   This source corresponds to the amqrspin object code except that  */
/*   all internal MQ function calls, tracing and error reporting      */
/*   have been stripped out.                                          */
/*                                                                    */
/*   The object amqrspin.dll is shipped in the MQ "exits"             */
/*   directory. DO NOT REPLACE OR REMOVE THIS OBJECT                  */
/*                                                                    */
/*                                                                    */
/**********************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <cmqc.h>
#include <cmqxc.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sspi.h>
#include <process.h>

/**********************************************************************/
/* Local defines                                                      */
/**********************************************************************/
/* Security methods */
#define PKGNAME_KERBEROS  "Kerberos"
#define PKGNAME_NTLM      "NTLM"

/* Types of message flowed to the partner exit */
#define SSPI_SEC_NONE         0
#define SSPI_SEC_REQUEST      1
#define SSPI_SEC_REPLY        2

/* This is the prefix to be used when forming the servicePrincipalName */
#define MQ_PRINCIPAL_NAME_PREFIX TEXT("ibmMQSeries")

/**********************************************************************/
/* Local structures                                                   */
/**********************************************************************/

/* Structure of the token data */
typedef struct
{
  struct
  {
    ULONG    msgType;
  }  header;
  MQBYTE   tok;
} SSPISECMSG, *PSSPISECMSG;

/* structure storing the state of the authentication sequence */
#define PRINCIPAL_NAME_LENGTH  (MQ_Q_MGR_NAME_LENGTH+16)
typedef struct
{
  PSecPkgInfo pSecurityPackages;
  PSecPkgInfo pSecurityPackage;
  PSSPISECMSG pSecurityMsgIn;
  PSSPISECMSG pSecurityMsgOut;
  BOOL        fIsClient;
  BOOL        fNewConversation;
  BOOL        fHaveCredentials;
  CredHandle  hCredential;         /* Handle to the credential */
  TimeStamp   tsExpiry;            /* Credentials' life time */
  CtxtHandle  hContext;            /* Handle to the security context */
  BOOL        fRequireTargetPrincipalName;
  char        szTargetPrincipalName[PRINCIPAL_NAME_LENGTH];
  char        szLocalPrincipalName[PRINCIPAL_NAME_LENGTH];
  void       *ptr1;
  void       *ptr2;

} STATEDATA, * PSTATEDATA;

/* definition of the exit user area for the exit */
/* cannot be more than 16 bytes long             */
typedef struct
{
  MQBYTE                 expectedSecMsg;
  PSecurityFunctionTable pSecurityInterface;
  HINSTANCE              DllHandle;
  PSTATEDATA             pStateData;
} SSPIEXITUSERAREA, * PSSPIEXITUSERAREA;

/**********************************************************************/
/* Local macros                                                       */
/**********************************************************************/

#define SECURITYEXITCALL (pParms->ExitId==MQXT_CHANNEL_SEC_EXIT)
#define EXPECTINGMSG(x)  ((x==MQXCC_SEND_AND_REQUEST_SEC_MSG)||(x==MQXCC_OK))
#define EXPECTSECMSG(x,y,z) (y)->expectedSecMsg=EXPECTINGMSG(x)?(z):SSPI_SEC_NONE

/**********************************************************************/
/*                                                                    */
/* Function Name: AcquireCredentials                                  */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Called to initialize:                                    */
/*                                                                    */
/*             The Local credential handles                           */
/*                                                                    */
/* Input Parameters:  pSecData                                        */
/*                    ulCredType                                      */
/*                                                                    */
/* Output Parameters: None                                            */
/*                                                                    */
/* InOut Parameters:  None.                                           */
/*                                                                    */
/* Returns: BOOL      bSuccess                                        */
/*                                                                    */
/* Exit Normal: TRUE                                                  */
/*                                                                    */
/* Exit Error:  FALSE Acquire failed                                  */
/*                                                                    */
/**********************************************************************/

BOOL AcquireCredentials(PSSPIEXITUSERAREA pSecData,
                        ULONG ulCredType,
                        PMQCD        pChDef)
{
  BOOL bSuccess=TRUE;
  SECURITY_STATUS status;

  if (!pSecData->pStateData->fHaveCredentials)
  {
   /* Acquire a credential handle. */
    status = (*pSecData->pSecurityInterface->AcquireCredentialsHandle)
      (
      NULL,                                 /* principal=current user */
      pSecData->pStateData->pSecurityPackage->Name,   /* Package Name */
      ulCredType,
      NULL,                                    /* Default credentials */
      NULL,
      NULL,
      NULL,
      &pSecData->pStateData->hCredential,
      &pSecData->pStateData->tsExpiry);

    if (status == SEC_E_OK)
    {
      pSecData->pStateData->fHaveCredentials = TRUE;
    }
    else
      bSuccess = FALSE;
  }

  return bSuccess;
}

/**********************************************************************/
/*                                                                    */
/* Function Name: SetPrincipalName                                    */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Called to initialize:                                    */
/*                                                                    */
/*             a string with the servicePrincipalName associated      */
/*             the Queue Manager                                      */
/*                                                                    */
/* Input Parameters:  pQMName                                         */
/*                                                                    */
/*                                                                    */
/* Output Parameters: pszPrincipalName                                */
/*                                                                    */
/* InOut Parameters:  None.                                           */
/*                                                                    */
/* Returns:           None                                            */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

void SetPrincipalName(char *pszPrincipalName,
                      MQCHAR48 pQMName,
                      char *pszComputerName)
{
  /* build up the SPN from the information collected */
  memset(pszPrincipalName, 0, PRINCIPAL_NAME_LENGTH );
  strcpy(pszPrincipalName,MQ_PRINCIPAL_NAME_PREFIX);
  strcat(pszPrincipalName,"/");
  strncat(pszPrincipalName, pQMName, MQ_Q_MGR_NAME_LENGTH );
  strtok(pszPrincipalName, " " );
}

/**********************************************************************/
/*                                                                    */
/* Function Name: getUserNameFromContext                              */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*                                                                    */
/*      Given a security context, return the associated Username      */
/*                                                                    */
/*                                                                    */
/* Input Parameters:  pSecData                                        */
/*                                                                    */
/* Output Parameters: szUserName,pulUserName,                         */
/*                    szWinUserName, pulWinUserName                   */
/*                                                                    */
/* InOut Parameters:  None.                                           */
/*                                                                    */
/* Returns:           None                                            */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

void getUserNameFromContext(PSSPIEXITUSERAREA pSecData,
                            char              szUserName[],
                            PULONG            pulUserName,
                            char              szWinUserName[],
                            PULONG            pulWinUserName,
                            PMQCD        pChDef)
{
  SECURITY_STATUS status;
  ULONG ulAttribute = SECPKG_ATTR_NAMES;
  SecPkgContext_Names SecContext;
  ULONG ulUserName=0;
  char *pDomainSep=NULL;

  /* get the user name associated with the token  */
  status = (*pSecData->pSecurityInterface->QueryContextAttributes)
    ( &pSecData->pStateData->hContext,ulAttribute, &SecContext );

  ulUserName = strlen(SecContext.sUserName);

  /* Output the user name, in Windows style */
  *pulWinUserName = ulUserName;
  strcpy(szWinUserName,SecContext.sUserName);

  /* If sUserName is in the format "Domain\User", we need to reformat to "user@domain" */
  pDomainSep = memchr(SecContext.sUserName,'\\',ulUserName);
  if (pDomainSep)
  {
    char *pUserName = malloc(ulUserName);
    char *pDomainName = malloc(ulUserName);
    if (pUserName && pDomainName)
    {
      /* Split the Domain\user up into two strings */
      *pDomainSep = '\0';
      pDomainSep++;
      strcpy(pDomainName,SecContext.sUserName);
      strcpy(pUserName,pDomainSep);

	  /* reformat the name */
      sprintf(szUserName,"%s@%s",pUserName,pDomainName);
      *pulUserName = ulUserName;
    }
    if (pUserName) free(pUserName);
    if (pDomainName) free(pDomainName);
  }
  status = (*pSecData->pSecurityInterface->FreeContextBuffer)(SecContext.sUserName);
}

/**********************************************************************/
/*                                                                    */
/* Function Name: initSecurityExit                                    */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Called to initialize:                                    */
/*                                                                    */
/*             The SSPIEXITUSERAREA in the ExitUserArea               */
/*                                                                    */
/*             The CONTEXTDATA in the ExitData                        */
/*                                                                    */
/* Input Parameters:  pszRequiredPackageName                          */
/*                    pChDef                                          */
/*                                                                    */
/* Output Parameters: pSecData                                        */
/*                                                                    */
/*                                                                    */
/* InOut Parameters:  None.                                           */
/*                                                                    */
/* Returns: MQLONG    return code value                               */
/*                                                                    */
/* Exit Normal: MQXCC_OK                                              */
/*                                                                    */
/* Exit Error:  MQXCC_SUPPRESS_FUNCTION     unable to allocate memory */
/*                                                                    */
/**********************************************************************/
MQLONG  initSecurityExit(PMQCHAR pszRequiredPackageName,
                         PMQCD pChDef,
                         PSSPIEXITUSERAREA pSecData)
{
  MQLONG rc = MQXCC_SUPPRESS_FUNCTION;
  ULONG ulIndex;
  ULONG ulNumOfPkgs;
  LONG lPkgToUse;
  INIT_SECURITY_INTERFACE InitSecurityInterface;
  SECURITY_STATUS status;

  pSecData->pSecurityInterface=NULL;
  pSecData->DllHandle=(HINSTANCE)NULL;

  /* Allocate and initialize the state buffer */
  pSecData->pStateData = malloc(sizeof(STATEDATA));
  memset(pSecData->pStateData, 0, sizeof(STATEDATA));
  pSecData->pStateData->fNewConversation = TRUE;
  pSecData->pStateData->fHaveCredentials = FALSE;
  pSecData->pStateData->fIsClient = FALSE;
  pSecData->pStateData->pSecurityPackages=NULL;
  pSecData->pStateData->ptr1=NULL;
  pSecData->pStateData->ptr2=NULL;

    /* Assume that Principal name is not required (E.g. NTLM) */
  pSecData->pStateData->fRequireTargetPrincipalName = FALSE;

  /* Look for conditions where principal name is required */
  if (!strcmp(pszRequiredPackageName,PKGNAME_KERBEROS))
  {
    /* Kerberos always requires a target principal name */
    pSecData->pStateData->fRequireTargetPrincipalName = TRUE;
  }

  /* Load the security provider DLL */
  pSecData->DllHandle = LoadLibrary (TEXT("secur32.dll"));
  if (!pSecData->DllHandle)
  {
    /* If the first one couldn't be found, try the old one */
    pSecData->DllHandle = LoadLibrary (TEXT("security.dll"));
  }
  if (pSecData->DllHandle)
  {

    /* Get the address of the function InitSecurityInterface. */
    InitSecurityInterface = (INIT_SECURITY_INTERFACE) GetProcAddress (
      pSecData->DllHandle,
      "InitSecurityInterfaceA");

    if (InitSecurityInterface)
    {
      /* Use InitSecurityInterface to get the function table. */
      pSecData->pSecurityInterface = (*InitSecurityInterface)();

      if (pSecData->pSecurityInterface && pSecData->pSecurityInterface->EnumerateSecurityPackages)
      {
        /* Retrieve the security packages supported by the provider. */
        status = (*pSecData->pSecurityInterface->EnumerateSecurityPackages)(
          &ulNumOfPkgs,
          &pSecData->pStateData->pSecurityPackages);

        if (status == SEC_E_OK)
        {
          /* Initialize dwPkgToUse. */
          lPkgToUse = -1;

          /* Determine which package should be used, based on Security Exit Data */
          for (ulIndex = 0; ulIndex < ulNumOfPkgs; ulIndex++)
          {
            if (!strcmp((char*)pSecData->pStateData->pSecurityPackages[ulIndex].Name,pszRequiredPackageName))
            {
              lPkgToUse = ulIndex;
              break;
            }
          }

          if (lPkgToUse >=0)
          {
            MQLONG lSecurityMsg;
            pSecData->pStateData->pSecurityPackage = &pSecData->pStateData->pSecurityPackages[lPkgToUse];
            pSecData->expectedSecMsg   = SSPI_SEC_NONE;

            /* Allocate two security buffers, each one                                */
            /* with enough space for a header and the biggest possible security token */
            lSecurityMsg = sizeof(pSecData->pStateData->pSecurityMsgIn->header)+pSecData->pStateData->pSecurityPackage->cbMaxToken;
            pSecData->pStateData->pSecurityMsgIn = malloc(lSecurityMsg);
            pSecData->pStateData->pSecurityMsgOut = malloc(lSecurityMsg);
            rc = MQXCC_OK; /* successful initialization */
          }
        }
      }
    }
  }
  return rc;
}
/**********************************************************************/
/*                                                                    */
/* Function Name: terminateSecurityExit                               */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Terminate the security context at end of the security    */
/*           exit call.                                               */
/*                                                                    */
/* Input Parameters: None                                             */
/*                                                                    */
/* Output Parameters: pParms.                                         */
/*                                                                    */
/* InOut Parameters:  pSecData.                                       */
/*                                                                    */
/* Returns: None                                                      */
/*                                                                    */
/**********************************************************************/
void terminateSecurityExit(PMQCXP pParms,PMQCD pChDef,
                           PSSPIEXITUSERAREA pSecData)
{
  SECURITY_STATUS status;

  if( pSecData->pStateData->ptr1 )
  {
    free(pSecData->pStateData->ptr1);
    pSecData->pStateData->ptr1=NULL;
  }

  if( pSecData->pStateData->ptr2 )
  {
    free(pSecData->pStateData->ptr2);
    pSecData->pStateData->ptr2=NULL;
  }

  if (pSecData->pStateData->pSecurityPackages)
  {
	  status = (*pSecData->pSecurityInterface->FreeContextBuffer)(pSecData->pStateData->pSecurityPackages);
	  pSecData->pStateData->pSecurityPackages = NULL;
  }

  if (pSecData->pStateData->fNewConversation == FALSE)
  {
    /* Free the context handle. */
    status = (*pSecData->pSecurityInterface->DeleteSecurityContext)
      (&pSecData->pStateData->hContext);
  }

  if (pSecData->pStateData->fHaveCredentials)
  {
    /* Free the credential handle. */
    status = (*pSecData->pSecurityInterface->FreeCredentialsHandle)
      ( &pSecData->pStateData->hCredential );

    if (status == SEC_E_OK)
    {
      pSecData->pStateData->fHaveCredentials = FALSE;

    }

  }

  if (pSecData->pStateData->pSecurityMsgIn)
	{
		free(pSecData->pStateData->pSecurityMsgIn);
		pSecData->pStateData->pSecurityMsgIn = NULL;
	}
	if (pSecData->pStateData->pSecurityMsgOut)
	{
		free(pSecData->pStateData->pSecurityMsgOut);
		pSecData->pStateData->pSecurityMsgOut = NULL;
	}

  if (pSecData->pStateData)
	{
		free(pSecData->pStateData);
		pSecData->pStateData = NULL;
	}
  if (pSecData->DllHandle)
  {
    FreeLibrary(pSecData->DllHandle);
    pSecData->DllHandle = NULL;
  }

  pParms->ExitResponse = MQXCC_OK;
}

/**********************************************************************/
/*                                                                    */
/* Function Name: initSecContext                                      */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*          Initiate Security context between this application and    */
/*          the security server.                                      */
/*                                                                    */
/* Input Parameters: pSecData                                         */
/*                   pSecurityMsgIn                                   */
/*                   inputMsgLength                                   */
/*                                                                    */
/*                                                                    */
/* Output Parameters: pParms                                          */
/*                    *pOutputMsgAddr                                 */
/*                    pOutputMsgLength                                */
/*                                                                    */
/* InOut Parameters:  None                                            */
/*                                                                    */
/* Returns: None                                                      */
/*                                                                    */
/**********************************************************************/

void initSecContext(PMQCXP       pParms,
                    PSSPIEXITUSERAREA pSecData,
                    PSSPISECMSG  pSecurityMsgIn,
                    MQLONG       inputMsgLength,
                    PMQBYTE      *pOutputMsgAddr,
                    PMQLONG      pOutputMsgLength,
                    PMQCD        pChDef)
{
  char        *pszTargetPrincipalName = NULL;
  ULONG ulContextReq;                    /* Required context attributes */
  ULONG ulContextAttributes;      /* Receives attributes of the context */
  SECURITY_STATUS status;                               /* Return codes */
  SecBuffer       OutSecBuffer;                        /* Output buffer */
  SecBufferDesc   OutBufferDesc;            /* Output buffer descriptor */
  SecBuffer       InSecBuffer;                         /* Output buffer */
  SecBufferDesc   InBufferDesc;             /* Output buffer descriptor */
  SecBufferDesc   *pInBufferDesc=NULL; /* Pointer to Input buffer descriptor */

  pSecData->pStateData->fIsClient = TRUE;
  /* Get some credentials */
  if ( AcquireCredentials(pSecData,SECPKG_CRED_OUTBOUND,pChDef) )
  {
    if (pSecData->pStateData->fRequireTargetPrincipalName)
    {
      MQCHAR ConnectionName[264+1];
      memcpy(ConnectionName,pChDef->ConnectionName,MQ_CONN_NAME_LENGTH);

      strtok(ConnectionName, " " );
      /* If a target principal is required, base it on the Partner name */
      SetPrincipalName(pSecData->pStateData->szTargetPrincipalName,pParms->PartnerName,ConnectionName);

      /* set the reference to it */
      pszTargetPrincipalName = pSecData->pStateData->szTargetPrincipalName;

      /* Set up context requirements */
       ulContextReq = ISC_REQ_MUTUAL_AUTH |
                      ISC_REQ_CONNECTION | ISC_REQ_SEQUENCE_DETECT |
                      ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY;

    }
    else
    {
      /* Set up context requirements */
      ulContextReq = ISC_REQ_CONNECTION | ISC_REQ_SEQUENCE_DETECT |
                     ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY;

    }

    /* Check if the pointer to SecurityFunctionTable is valid. */
    if (pSecData->pStateData->pSecurityPackage && pSecData->pSecurityInterface)
    {

      /* Initialize the OutSecBuffer structure. */
      OutSecBuffer.cbBuffer = pSecData->pStateData->pSecurityPackage->cbMaxToken;

      OutSecBuffer.BufferType = SECBUFFER_TOKEN;
      OutSecBuffer.pvBuffer = &pSecData->pStateData->pSecurityMsgOut->tok;

      if (pSecurityMsgIn)
      {
        /* prepare input buffer */
        InBufferDesc.ulVersion = 0;
        InBufferDesc.cBuffers = 1;
        InBufferDesc.pBuffers = &InSecBuffer;

        InSecBuffer.cbBuffer = inputMsgLength-sizeof(pSecData->pStateData->pSecurityMsgIn->header);
        InSecBuffer.BufferType = SECBUFFER_TOKEN;
        InSecBuffer.pvBuffer = &pSecurityMsgIn->tok;
        pInBufferDesc = &InBufferDesc;
      }
      /* Initialize the OutBufferDesc structure. */
      OutBufferDesc.ulVersion = 0;
      OutBufferDesc.cBuffers = 1;
      OutBufferDesc.pBuffers = &OutSecBuffer;

      /* Get the authentication token from the security package to send to */
      /* the server to request an authenticated token.                     */
      status = (*pSecData->pSecurityInterface->InitializeSecurityContext)(
        &pSecData->pStateData->hCredential,
        pSecData->pStateData->fNewConversation ? NULL : &pSecData->pStateData->hContext,
        pszTargetPrincipalName,
        ulContextReq,
        0,
        SECURITY_NATIVE_DREP,
        pInBufferDesc,
        0,
        &pSecData->pStateData->hContext,
        &OutBufferDesc,
        &ulContextAttributes,
        &pSecData->pStateData->tsExpiry);

      if (pSecData->pStateData->fNewConversation && !IS_ERROR(status))
        pSecData->pStateData->fNewConversation = FALSE;

      if ((SEC_I_COMPLETE_NEEDED == status) || (SEC_I_COMPLETE_AND_CONTINUE == status))
      {
        status = (*pSecData->pSecurityInterface->CompleteAuthToken)
          (&pSecData->pStateData->hContext, &OutBufferDesc);
      }

      if ( SEC_E_OK == status)
      {
        /* Authentication complete */
        char szUserName[MAX_PATH];
        ULONG ulUserName = sizeof(szUserName);
        char szWinUserName[MAX_PATH];
        ULONG ulWinUserName = sizeof(szWinUserName);
        getUserNameFromContext(pSecData,szUserName, &ulUserName,szWinUserName, &ulWinUserName, pChDef );
      }

      if (OutSecBuffer.cbBuffer)
      {
        *pOutputMsgAddr = (unsigned char *)pSecData->pStateData->pSecurityMsgOut;
        *pOutputMsgLength = OutSecBuffer.cbBuffer+sizeof(pSecData->pStateData->pSecurityMsgIn->header);
        pParms->ExitResponse2 = MQXR2_USE_EXIT_BUFFER;

        if ( SEC_I_CONTINUE_NEEDED == status)
        {
          pSecData->pStateData->pSecurityMsgOut->header.msgType = SSPI_SEC_REQUEST;
          pParms->ExitResponse = MQXCC_SEND_AND_REQUEST_SEC_MSG;
        }
        if ( SEC_E_OK == status)
        {
          pSecData->pStateData->pSecurityMsgOut->header.msgType = SSPI_SEC_REPLY;
          pParms->ExitResponse = MQXCC_SEND_SEC_MSG;
        }

      }
      else if ( SEC_E_OK == status)
      {
        pParms->ExitResponse = MQXCC_OK;
      }
      else
      {
        pParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;
      }
    }
  }
}
/**********************************************************************/
/*                                                                    */
/* Function Name: acceptSecContext                                    */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Establish a remotely initiated security context between  */
/*           this application and a remote Peer.                      */
/*                                                                    */
/* Input Parameters: pSecData                                         */
/*                   pInputMsgAddr                                    */
/*                   inputMsgLength                                   */
/*                   pChDef                                           */
/*                                                                    */
/* Output Parameters: pParms                                          */
/*                    pOutputMsgAddr                                  */
/*                    pOutputMsgLength                                */
/*                                                                    */
/* InOut Parameters:  None                                            */
/*                                                                    */
/* Returns: None                                                      */
/*                                                                    */
/* Notes:                                                             */
/*   Restrictions: List any restrictions (or None)                    */
/*                                                                    */
/**********************************************************************/
void acceptSecContext(PMQCXP       pParms,
                      PSSPIEXITUSERAREA pSecData,
                      PSSPISECMSG  pSecurityMsg,
                      MQLONG       inputMsgLength,
                      PMQBYTE      *pOutputMsgAddr,
                      PMQLONG      pOutputMsgLength,
                      PMQCD        pChDef)
{
  SECURITY_STATUS status;
  SecBufferDesc   OutBufferDesc;
  SecBuffer       OutSecBuffer;
  SecBufferDesc   InBufferDesc;
  SecBuffer       InSecBuffer;
  ULONG ulContextReq;             /* Required context attributes */
  ULONG ulContextAttributes;      /* Receives attributes of the context */
  BOOL  bContinue=TRUE;

  /* Get some credentials */
  if ( AcquireCredentials(pSecData,SECPKG_CRED_INBOUND,pChDef) )
  {
    if (pSecData->pStateData->fRequireTargetPrincipalName)
    {

      /* Set up context requirements */
      ulContextReq = ASC_REQ_MUTUAL_AUTH | ASC_REQ_CONNECTION | ASC_REQ_SEQUENCE_DETECT |
                     ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY;
    }
    else
    {
      /* Set up context requirements */
      ulContextReq = ASC_REQ_CONNECTION | ASC_REQ_SEQUENCE_DETECT |
                     ASC_REQ_REPLAY_DETECT | ASC_REQ_CONFIDENTIALITY;
    }

    /* prepare output buffer */
    OutBufferDesc.ulVersion = 0;
    OutBufferDesc.cBuffers = 1;
    OutBufferDesc.pBuffers = &OutSecBuffer;

    OutSecBuffer.cbBuffer = pSecData->pStateData->pSecurityPackage->cbMaxToken;
    OutSecBuffer.BufferType = SECBUFFER_TOKEN;
    OutSecBuffer.pvBuffer = &pSecData->pStateData->pSecurityMsgOut->tok;

    /* prepare input buffer */
    InBufferDesc.ulVersion = 0;
    InBufferDesc.cBuffers = 1;
    InBufferDesc.pBuffers = &InSecBuffer;

    InSecBuffer.cbBuffer = inputMsgLength-sizeof(pSecData->pStateData->pSecurityMsgIn->header);
    InSecBuffer.BufferType = SECBUFFER_TOKEN;
    InSecBuffer.pvBuffer = &pSecurityMsg->tok;

    status = (*pSecData->pSecurityInterface->AcceptSecurityContext) (
      &pSecData->pStateData->hCredential,
      pSecData->pStateData->fNewConversation ? NULL : &pSecData->pStateData->hContext,
      &InBufferDesc,
      ulContextReq,      /* context requirements */
      SECURITY_NATIVE_DREP,
      &pSecData->pStateData->hContext,
      &OutBufferDesc,
      &ulContextAttributes,
      &pSecData->pStateData->tsExpiry
      );

    if (pSecData->pStateData->fNewConversation)
      pSecData->pStateData->fNewConversation = FALSE;

    if ((SEC_I_COMPLETE_NEEDED == status) || (SEC_I_COMPLETE_AND_CONTINUE == status))
    {
      status = (*pSecData->pSecurityInterface->CompleteAuthToken)
        (&pSecData->pStateData->hContext, &OutBufferDesc);
    }

    if ( SEC_E_OK == status)
    {
      /* The user associated with the token is Authentic */
      BOOL bPopulateChannelDef = TRUE;
      char szUserName[MAX_PATH];
      ULONG ulUserName = sizeof(szUserName);
      char szWinUserName[MAX_PATH];
      ULONG ulWinUserName = sizeof(szWinUserName);
      int index=sizeof(pParms->ExitData)-1;
      unsigned int lName=0;

      getUserNameFromContext(pSecData,szUserName, &ulUserName,szWinUserName, &ulWinUserName, pChDef );

      /* Remove trailing spaces from ExitData */
      while (index >= 0 )
      {
        if (isspace(pParms->ExitData[index]))
          pParms->ExitData[index--]='\0';
        else
          break;
      }

      lName=strlen(pParms->ExitData);

      /* For SVRCONN channels, if a users name has been specified */
      /* specified in the supplied exit data                      */
      /* then restrict channel usage to a that named user         */
      if (pChDef->ChannelType == MQCHT_SVRCONN && lName>0)
      {
        bPopulateChannelDef = FALSE;

        /* if the supplied-name is NOT qualified by domain, just match on the userid*/
        /* otherwise try to match against either of the two forms of userid         */
        if (strchr(pParms->ExitData,'\\')==NULL && strchr(pParms->ExitData,'@')==NULL)
        {
          char sztmpUserName[MAX_PATH];
          strcpy(sztmpUserName,szUserName);
          strtok(sztmpUserName,"@");
          if(strlen(sztmpUserName)==lName && !strnicmp(sztmpUserName,pParms->ExitData,lName))
          {
            bPopulateChannelDef = TRUE;
          }
          else
          {
            bPopulateChannelDef = FALSE;
          }

        }
        else if (strlen(szWinUserName)==lName && !strnicmp(szWinUserName,pParms->ExitData,lName))
        {
           bPopulateChannelDef = TRUE;
        }
        else if (strlen(szUserName)==lName && !strnicmp(szUserName,pParms->ExitData,lName))
        {
           bPopulateChannelDef = TRUE;
        }
        else
        {
          bPopulateChannelDef = FALSE;
        }
      }
      if (bPopulateChannelDef)
      {
        pParms->ExitResponse = MQXCC_OK;

        /* Populate the channel definition with the user name */
        pChDef->LongMCAUserIdLength = ulUserName;
        pChDef->LongRemoteUserIdLength = ulUserName;
        /* allocate some buffers and save the pointers away */
        pChDef->LongMCAUserIdPtr = malloc(pChDef->LongMCAUserIdLength);
        pSecData->pStateData->ptr1 = pChDef->LongMCAUserIdPtr;
        pChDef->LongRemoteUserIdPtr = malloc(pChDef->LongRemoteUserIdLength);
        pSecData->pStateData->ptr2 = pChDef->LongRemoteUserIdPtr;

        memcpy(pChDef->LongRemoteUserIdPtr,(char*)szUserName,pChDef->LongRemoteUserIdLength);

        if (pChDef->LongRemoteUserIdLength > 12)
          memcpy(pChDef->RemoteUserIdentifier,(char*)szUserName,12);
        else
          memcpy(pChDef->RemoteUserIdentifier,(char*)szUserName,pChDef->LongRemoteUserIdLength );


        memcpy(pChDef->LongMCAUserIdPtr,(char*)szUserName,pChDef->LongMCAUserIdLength);

        if (pChDef->LongMCAUserIdLength > 12)
          memcpy(pChDef->MCAUserIdentifier,(char*)szUserName,12);
        else
          memcpy(pChDef->MCAUserIdentifier,(char*)szUserName,pChDef->LongMCAUserIdLength );

      }
      else
      {
         bContinue = FALSE;
         pParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;
      }
    }

    if (bContinue && OutSecBuffer.cbBuffer)
    {
      *pOutputMsgAddr = (unsigned char *)pSecData->pStateData->pSecurityMsgOut;
      *pOutputMsgLength = OutSecBuffer.cbBuffer+sizeof(pSecData->pStateData->pSecurityMsgIn->header);
      pParms->ExitResponse2 = MQXR2_USE_EXIT_BUFFER;

      if (SEC_I_CONTINUE_NEEDED == status)
      {
        pSecData->pStateData->pSecurityMsgOut->header.msgType = SSPI_SEC_REQUEST;
        pParms->ExitResponse = MQXCC_SEND_AND_REQUEST_SEC_MSG;
      }
      if (SEC_E_OK == status)
      {
        pSecData->pStateData->pSecurityMsgOut->header.msgType = SSPI_SEC_REPLY;
        pParms->ExitResponse = MQXCC_SEND_SEC_MSG;
      }
    }
  }
}

/**********************************************************************/
/*                                                                    */
/* Function Name: processSecurityMessage                              */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Process a security message                               */
/*           process the message dependent upon the value in :-       */
/*                                      pInMsgAddr->header.msgType    */
/*                                                                    */
/* Input Parameters: pChDef                                           */
/*                   pInMsgAddr                                       */
/*                   inMsgLength                                      */
/*                                                                    */
/* Output Parameters:pOutMsgAddr                                      */
/*                   pOutMsgLength                                    */
/*                                                                    */
/* InOut Parameters:  pParms.                                         */
/*                                                                    */
/* Returns: None                                                      */
/*                                                                    */
/**********************************************************************/

void processSecurityMessage(PMQCXP      pParms,
                            PMQCD       pChDef,
                            PSSPISECMSG pSecurityMsg,
                            MQLONG      inMsgLength,
                            PMQPTR      pOutMsgAddr,
                            PMQLONG     pOutMsgLength)
{

  PSSPIEXITUSERAREA pSecData;

  pSecData = (PSSPIEXITUSERAREA)&(pParms->ExitUserArea);
   switch ( pSecurityMsg->header.msgType )
   {
   case SSPI_SEC_REQUEST:
   case SSPI_SEC_REPLY:
     if (pSecData->pStateData->fIsClient)
     {
       initSecContext(pParms,
         pSecData,
         pSecurityMsg,
         inMsgLength,
         (PMQBYTE*)pOutMsgAddr,
         pOutMsgLength,
         pChDef);
     }
     else
     {
       acceptSecContext(pParms,
         pSecData,
         pSecurityMsg,
         inMsgLength,
         (PMQBYTE*)pOutMsgAddr,
         pOutMsgLength,
         pChDef);
     }
     break;
   default:
     pParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;
     break;
   }
}

/**********************************************************************/
/*                                                                    */
/* Function Name: SSPI                                                */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Main entry point for channel security exit               */
/*           Check to see if this has been called as a security exit  */
/*           Then switch to whatever is defined by pParms->ExitReason */
/*                                                                    */
/* Input Parameters: pAgentBufferLength  UNUSED                       */
/*                                                                    */
/* Output Parameters: None                                            */
/*                                                                    */
/* InOut Parameters: pChannelExitParms   Channel exit parameter block */
/*                   pChannelDefinition  Channel definition           */
/*                   pDataLength         Length of data               */
/*                   pAgentBuffer        Agent buffer                 */
/*                   pExitBufferLength   Length of exit buffer        */
/*                   pExitBufferAddr     Address of exit buffer       */
/*                                                                    */
/*                                                                    */
/* Returns: None                                                      */
/*                                                                    */
/**********************************************************************/

void MQENTRY SSPI(
                  PMQCHAR  pszRequiredPackageName,/*                              */
                  PMQVOID  pChannelExitParms,     /* Channel exit parameter block */
                  PMQVOID  pChannelDefinition,    /* Channel definition           */
                  PMQLONG  pDataLength,           /* Length of data               */
                  PMQLONG  pAgentBufferLength,    /* Length of agent buffer       */
                  PMQVOID  pAgentBuffer,          /* Agent buffer                 */
                  PMQLONG  pExitBufferLength,     /* Length of exit buffer        */
                  PMQPTR   pExitBufferAddr)       /* Address of exit buffer       */
{
  PMQCXP   pParms = (PMQCXP)pChannelExitParms;
  PMQCD    pChDef = (PMQCD)pChannelDefinition;
  PSSPIEXITUSERAREA pSecData;

  pSecData = (PSSPIEXITUSERAREA)&(pParms->ExitUserArea);

  pParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;

  /* Invoked as security exit? */
  if (SECURITYEXITCALL)
  {
    /* free buffer used to pass back message on previous exit call */
    if ( (pExitBufferAddr != NULL) && (*pExitBufferAddr != NULL) )
    {
      *pExitBufferAddr = NULL;
      *pExitBufferLength = 0;
    } /* endif */

    /*******************************************************************/
    /* now switch to what ever function we were called to do           */
    /*******************************************************************/

    switch ( pParms->ExitReason )
    {
    case MQXR_INIT:
      pParms->ExitResponse =
        initSecurityExit(pszRequiredPackageName,
        pChDef,
        (PSSPIEXITUSERAREA)&(pParms->ExitUserArea));
      EXPECTSECMSG(pParms->ExitResponse,
        ((PSSPIEXITUSERAREA)&(pParms->ExitUserArea)),
        SSPI_SEC_REQUEST);
      break;

    case MQXR_INIT_SEC:
		if (pChDef->ChannelType == MQCHT_SVRCONN)
		  pParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;
		else
		{
			initSecContext(pParms,
				pSecData,
				NULL,
				0,
				(PMQBYTE *)pExitBufferAddr,
				pExitBufferLength,
        pChDef);

			EXPECTSECMSG(pParms->ExitResponse,
				pSecData,
				SSPI_SEC_REPLY);

		}
      break;

    case MQXR_SEC_MSG:
      if (*pDataLength)
      {
        processSecurityMessage(pParms, pChDef,
          pAgentBuffer,
          *pDataLength,
          pExitBufferAddr,
          pExitBufferLength);
      }
      else
        pParms->ExitResponse = MQXCC_OK;
      break;

    case MQXR_SEC_PARMS:
        /* Do nothing */
        pParms->ExitResponse = MQXCC_OK;
        break;

    case MQXR_TERM:
      terminateSecurityExit(pParms,pChannelDefinition,(PSSPIEXITUSERAREA)&(pParms->ExitUserArea));
      break;

    default:
      pParms->ExitResponse = MQXCC_SUPPRESS_FUNCTION;
      break;
    } /* endswitch */

    if ( pParms->ExitResponse == MQXCC_SUPPRESS_FUNCTION )
    {
      pParms->ExitResponse2 = MQXR2_USE_AGENT_BUFFER;
    }
    else
    {
      if ( pParms->ExitResponse2 == MQXR2_USE_EXIT_BUFFER )
      {
        *pDataLength = *pExitBufferLength;
      } /* endif */
    } /* end if */

  } /* endif */
}

/**********************************************************************/
/*                                                                    */
/* Function Name: SCY_KERBEROS                                        */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Main entry point for Kerberos channel security exit      */
/*                                                                    */
/**********************************************************************/


__declspec( dllexport ) void MQENTRY SCY_KERBEROS(
    PMQVOID  pChannelExitParms,   /* Channel exit parameter block */
    PMQVOID  pChannelDefinition,  /* Channel definition           */
    PMQLONG  pDataLength,         /* Length of data               */
    PMQLONG  pAgentBufferLength,  /* Length of agent buffer       */
    PMQVOID  pAgentBuffer,        /* Agent buffer                 */
    PMQLONG  pExitBufferLength,   /* Length of exit buffer        */
    PMQPTR   pExitBufferAddr)     /* Address of exit buffer       */

{
  SSPI( PKGNAME_KERBEROS,     /*                              */
        pChannelExitParms,    /* Channel exit parameter block */
        pChannelDefinition,   /* Channel definition           */
        pDataLength,          /* Length of data               */
        pAgentBufferLength,   /* Length of agent buffer       */
        pAgentBuffer,         /* Agent buffer                 */
        pExitBufferLength,    /* Length of exit buffer        */
        pExitBufferAddr);     /* Address of exit buffer       */
}

/**********************************************************************/
/*                                                                    */
/* Function Name: SCY_NTLM                                            */
/*                                                                    */
/**********************************************************************/
/*                                                                    */
/* Function:                                                          */
/*           Main entry point for NTLM channel security exit          */
/*                                                                    */
/**********************************************************************/

__declspec( dllexport ) void MQENTRY SCY_NTLM(
    PMQVOID  pChannelExitParms,   /* Channel exit parameter block */
    PMQVOID  pChannelDefinition,  /* Channel definition           */
    PMQLONG  pDataLength,         /* Length of data               */
    PMQLONG  pAgentBufferLength,  /* Length of agent buffer       */
    PMQVOID  pAgentBuffer,        /* Agent buffer                 */
    PMQLONG  pExitBufferLength,   /* Length of exit buffer        */
    PMQPTR   pExitBufferAddr)     /* Address of exit buffer       */

{

  SSPI( PKGNAME_NTLM,        /*                              */
        pChannelExitParms,   /* Channel exit parameter block */
        pChannelDefinition,  /* Channel definition           */
        pDataLength,         /* Length of data               */
        pAgentBufferLength,  /* Length of agent buffer       */
        pAgentBuffer,        /* Agent buffer                 */
        pExitBufferLength,   /* Length of exit buffer        */
        pExitBufferAddr);    /* Address of exit buffer       */

}
