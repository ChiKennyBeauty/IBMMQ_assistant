/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/amquregn.c */
/********************************************************************/
/*                                                                  */
/* Program name: AMQUREGN                                           */
/*                                                                  */
/* Description: Sample C program for use on NT that dumps the MQ    */
/*              values in the Registry to the console or (via       */
/*              standard redirection) to a file.                    */
/*   <copyright                                                     */
/*   notice="lm-source-program"                                     */
/*   pids="5724-H72,"                                               */
/*   years="1994,2020"                                              */
/*   crc="3857236284" >                                             */
/*   Licensed Materials - Property of IBM                           */
/*                                                                  */
/*   5724-H72,                                                      */
/*                                                                  */
/*   (C) Copyright IBM Corp. 1994, 2020 All Rights Reserved.        */
/*                                                                  */
/*   US Government Users Restricted Rights - Use, duplication or    */
/*   disclosure restricted by GSA ADP Schedule Contract with        */
/*   IBM Corp.                                                      */
/*   </copyright>                                                   */
/********************************************************************/
/* Function:                                                        */
/*                                                                  */
/*                                                                  */
/*   AMQUREGN is a sample C program that accesses the Windows       */
/*   registry, extracts the values pertinent to MQ and writes       */
/*   them to the console. The main use of this program is expected  */
/*   to be for problem documentation purposes. The control of the   */
/*   set of registry values to be displayed is via use of a control */
/*   file (amquregn.ctl) that is input on the command line.         */
/*                                                                  */
/*   Additionally this program issues a GetEnvironmentStrings()     */
/*   call to determine the environment under which we are running   */
/*                                                                  */
/********************************************************************/

#define MAXSTRING 512



/*********************************************************************/
/* Includes                                                          */
/*********************************************************************/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <mbstring.h>
#include <string.h>
#include <tchar.h>
#include <stddef.h>
#include <process.h>

#ifndef false
 #define false 0
#endif

#ifndef true
 #define true 1
#endif



void main(int argc, void * argv[]);
static   BOOL PrintKeysAndValues(HKEY hKeyConfig, CHAR *pszKey);

static   BOOL PrintKeyValue(CHAR *pszValueName, DWORD dwValueType,
                           BYTE *pValueData, DWORD dwcbValueData);

int       gLevel=0;     /* Level counter                             */
CHAR      gSpaces[256]; /* assume we never have more than 128 levels */




void main(int argc, char * argv[])
{
  BOOL      fSuccess;
  LONG      lRc=ERROR_SUCCESS;
  FILE      *fpConfig = NULL;
  HKEY      hKeyConfig;
  CHAR      szKey[_MAX_PATH];
  HKEY      hRoot;


  /****************************************/
  /* first check that we have a parameter */
  /****************************************/

  if (argc != 2)
  {
		printf("Usage: amquregn fname\n");
		printf("    ... where fname is a file containing list of keys to be dumped\n");
		return;
  }

  /****************************************************/
  /* Try opening the file that was passed in          */
  /****************************************************/

  fpConfig = fopen(argv[1],"r");
  if (fpConfig == NULL)
  {
	  lRc = GetLastError();
	  printf("ERROR: unable to open file %s, fopen return code was %d\n",
		      argv[1],lRc);
  }



  if (lRc == ERROR_SUCCESS)
  {

    /*****************************************************************/
    /*  Now get the various lines from the file - anything starting  */
    /*  with a # is just a comment                                   */
    /*****************************************************************/

     BOOL finished = false;
     CHAR line[MAXSTRING];
     int slash = '\\';
     char * slash_pos ;
     char * zero_a_pos;
     /*****************************************************************/
     /*  Put some headings on the page                                */
     /*****************************************************************/
     printf("\nIBM MQ Registry Values Listing Utility\n");
     printf("========================================\n\n");
     printf("The values below show the registry entries held in\n");
     printf("the set of keys contained in file %s\n\n",argv[1]);

     while(!finished)
     {
       hRoot = NULL;
       memset(line,0,sizeof(line));
       gLevel=0;
       if ( fgets( line, MAXSTRING, fpConfig ) != NULL)
       {
         if (line[0] != '#') /* is it a real line? */
         {

          /*****************************************************************/
          /* Assume that everything up to first \ is the root              */
          /* and that everything past it is the szkey                      */
          /*****************************************************************/

          /* If we have an x0A replace it with null */
          zero_a_pos =strchr(line,0x0A);
          if (zero_a_pos != NULL) memset(zero_a_pos,0,1);

          slash_pos=strchr(line,slash);
          if (slash_pos != NULL)
          {
            memset(slash_pos,0,1); /* split into two strings */
            if (!strcmp(line,"HKEY_LOCAL_MACHINE") ||
       	        (!strcmp(line,"HKLM")))
              hRoot= HKEY_LOCAL_MACHINE;
            if (!strcmp(line,"HKEY_CLASSES_ROOT") ||
                (!strcmp(line,"HKCR")))
              hRoot= HKEY_CLASSES_ROOT;
            if (!strcmp(line,"HKEY_CURRENT_USER") ||
                (!strcmp(line,"HKCU")))
              hRoot= HKEY_CURRENT_USER;
            if (!strcmp(line,"HKEY_USERS") ||
                (!strcmp(line,"HKU")))
              hRoot= HKEY_USERS;
            if (!strcmp(line,"HKEY_CURRENT_CONFIG") ||
                (!strcmp(line,"HKCC")))
              hRoot= HKEY_CURRENT_CONFIG;
            if (!strcmp(line,"HKEY_DYN_DATA") ||
                (!strcmp(line,"HKDD")))
              hRoot= HKEY_DYN_DATA;
          }

          if (hRoot == NULL)
          {
              printf("\n\nThe line -%s- doesn't appear valid",line);
              printf("and has been ignored!!!!!!!!!!!!!!!!!\n\n");
          }
          else
          {
              sprintf(szKey, "%s",slash_pos+1);
              lRc = RegOpenKeyEx(hRoot,
                     szKey,
                     0,
                     KEY_ENUMERATE_SUB_KEYS
                      | KEY_QUERY_VALUE,
                     &hKeyConfig);

              if (lRc == ERROR_SUCCESS)
              {

                 fSuccess = PrintKeysAndValues(hKeyConfig, szKey);

                 /******************************************************************/
                 /* The key is open so close it                                    */
                 /******************************************************************/
                 RegCloseKey(hKeyConfig);
                printf("\n\n-------------------------------------------------------\n");
                printf("-------------------------------------------------------\n");
             }
             else
             {
                printf("Unable to open key: '%s' %d\n", szKey, lRc);
             }
          }   /* end of if (slash_pos .... */
        }   /* end of if (line[0]...  */

     } /* end of if (fgets ...... */
     else finished = true; /* fgets returned null */
   } /* end of while loop */

  }

  /*********************************************************************/
  /*  Print out the environment settings                               */
  /*********************************************************************/

  {
    int count=0;
    BOOL all_done=false;
    BOOL prev_was_null=false;
    char * pStart_of_string;
    char * pWork;
    LPVOID pEnvblock;

    pEnvblock = GetEnvironmentStrings();

    printf("\n\n\n---------------------------------------------------------\n");
    printf("---------------------------------------------------------\n");
    printf(" Environment follows....\n\n");

    pStart_of_string = pEnvblock;
    pWork = (char *)pEnvblock;
    while(!all_done)
    {
      if  (*pWork == 0x00)
      {
        printf("   %s\n",pStart_of_string);
        if (prev_was_null) all_done=true;
        else
        {
          prev_was_null=true;
        }
        pStart_of_string = pWork+1;
      }
      else prev_was_null=false;

      if (count > 32768) all_done=true; /* Just in case */
      pWork++;
      count++;

   }


   FreeEnvironmentStrings(pEnvblock);
  }

} /* end of main */


/**********************************************************************/
/* Function       : PrintKeysAndValues                                */
/**********************************************************************/
/* Description    : Print a registry configuration tree to console    */
/* Params In      :                                                   */
/* Params Out     :                                                   */
/* Params In/Out  : None                                              */
/* Returns        : BOOL       -                                      */
/* Side effects   :                                                   */
/* Notes          :                                                   */
/*                                                                    */
/**********************************************************************/
BOOL  PrintKeysAndValues(HKEY hKey, CHAR *pszKey)
{
  BOOL      fSuccess = TRUE;
  CHAR      szText[MAXSTRING];
  CHAR      szRegKey[MAXSTRING];
  CHAR      szSubKey[_MAX_PATH];
  CHAR      szFullSubKey[_MAX_PATH];
  DWORD     SubKeySize;
  DWORD     dwIndex;
  HKEY      hSubKey;
  LONG      lRc;
  CHAR      *pszValueName = NULL;
  BYTE      *pValueData = NULL;
  DWORD     dwValueType;
  DWORD     dwcbValueName;
  DWORD     dwcbValueData;
  DWORD     dwMaxValueName;
  DWORD     dwMaxValueData;
  FILETIME  FileTime;

  gLevel++;

  sprintf(szRegKey, "[HKEY_LOCAL_MACHINE\\%s]", pszKey);
  sprintf(szText, "Key %s", szRegKey);


  memset(gSpaces,'..',gLevel*2);
  memset(gSpaces+gLevel*2,0,1);
  /********************************************************************/
  /* Write the key to the .reg file                                   */
  /********************************************************************/
  printf("\n%s%s\n", gSpaces,szRegKey);


  /********************************************************************/
  /* Get the maximum lengths for the value names and data.            */
  /* Allocate buffers for these.                                      */
  /********************************************************************/
  if (fSuccess)
  {
    lRc = RegQueryInfoKey(hKey,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          &dwMaxValueName,
                          &dwMaxValueData,
                          NULL,
                          NULL);
    if (ERROR_SUCCESS == lRc)
    {
      dwMaxValueName++;
      pszValueName = (CHAR *)malloc(dwMaxValueName);
      if (!pszValueName)
      {
        fSuccess = FALSE;
        printf("Unable to allocate value name buffer %u\n", dwMaxValueName);
      }
      dwMaxValueData++;
      pValueData = (CHAR *)malloc(dwMaxValueData);
      if (!pValueData)
      {
        fSuccess = FALSE;
        printf("Unable to allocate value data buffer %u\n", dwMaxValueData);
      }
    }
    else
    {
      fSuccess = FALSE;
      printf("Can't query key info %d\n", lRc);
    }
  }

  /********************************************************************/
  /* Enumerate this key's values and save them                        */
  /********************************************************************/
  dwIndex = 0;
  lRc = ERROR_SUCCESS;
  while (fSuccess && ERROR_SUCCESS == lRc)
  {
    /******************************************************************/
    /* Enumerate the values                                           */
    /******************************************************************/
    dwcbValueName = dwMaxValueName;
    dwcbValueData = dwMaxValueData;
    lRc = RegEnumValue(hKey,
                       dwIndex,
                       (LPTSTR)pszValueName,
                       &dwcbValueName,
                       NULL,           // reserved
                       &dwValueType,
                       pValueData,
                       &dwcbValueData);

    if (ERROR_SUCCESS == lRc)
    {
      /**************************************************************/
      /* Save this value                                            */
      /**************************************************************/
      fSuccess &= PrintKeyValue(pszValueName, dwValueType,
                               pValueData, dwcbValueData);

    }
    else
    {
      if (ERROR_NO_MORE_ITEMS != lRc)
      {
        fSuccess = FALSE;
        printf("Can't open enumerate Key values: '%s' %d\n", pszKey, lRc);

      }
    }

    /****************************************************************/
    /* Increment the index to next value                            */
    /****************************************************************/
    dwIndex++;
  }

  /********************************************************************/
  /* Free the buffers we allocated                                    */
  /********************************************************************/
  if (pszValueName) free(pszValueName);
  if (pValueData) free(pValueData);

  /********************************************************************/
  /* Enumerate this key's subkeys and save them and their values      */
  /********************************************************************/
  dwIndex = 0;
  lRc = ERROR_SUCCESS;
  while (fSuccess && ERROR_SUCCESS == lRc)
  {
    /******************************************************************/
    /* Enumerate the subkeys                                          */
    /******************************************************************/
    SubKeySize = sizeof(szSubKey);
    lRc = RegEnumKeyEx(hKey,
                       dwIndex,
                       szSubKey,
                       &SubKeySize,
                       NULL,
                       NULL,
                       NULL,
                       &FileTime);

    if (ERROR_SUCCESS == lRc)
    {
      /****************************************************************/
      /* Open the subkey                                              */
      /****************************************************************/
      lRc = RegOpenKeyEx(hKey,
                         szSubKey,
                         0,
                         KEY_ENUMERATE_SUB_KEYS
                          | KEY_QUERY_VALUE,
                         &hSubKey);
      if (lRc == ERROR_SUCCESS)
      {
        /**************************************************************/
        /* Process the subkey and it's values                         */
        /**************************************************************/
        sprintf(szFullSubKey, "%s\\%s", pszKey, szSubKey);
        fSuccess &= PrintKeysAndValues(hSubKey, szFullSubKey);

        /**************************************************************/
        /* The subkey is open so close it                             */
        /**************************************************************/
        RegCloseKey(hSubKey);
      }
      else
      {
        fSuccess = FALSE;
        printf(szText, "Can't open subKey: '%s' %d\n", szSubKey, lRc);
      }
    }
    else
    {
      if (ERROR_NO_MORE_ITEMS != lRc)
      {
        fSuccess = FALSE;
        sprintf(szText, "Can't open enumerate key: '%s' %d", pszKey, lRc);
      }
    }

    /****************************************************************/
    /* Increment the index to next subkey                           */
    /****************************************************************/
    dwIndex++;
  }

  /*********************************************************************/
  /* Return to caller                                                  */
  /*********************************************************************/
  gLevel--;
  return(fSuccess);
} /* end of PrintKeysAndValues */

/**********************************************************************/
/* Function       : PrintKeyValue()                                    */
/**********************************************************************/
/* Description    :                                                   */
/* Params In      :                                                   */
/* Params Out     :                                                   */
/* Params In/Out  : None                                              */
/* Returns        :                                                   */
/* Side effects   :                                                   */
/* Notes          :                                                   */
/*                                                                    */
/**********************************************************************/
BOOL  PrintKeyValue(CHAR *pszValueName, DWORD dwValueType,
                   BYTE *pValueData, DWORD dwcbValueData)
{
  BOOL      fSuccess = TRUE;
  CHAR      szValueNameText[MAXSTRING];
  CHAR      szRegValue[MAXSTRING];
  CHAR      szValueData[MAXSTRING];
  CHAR      *pSlash;
  CHAR      *pStart;
  CHAR      *pNext;

  /*********************************************************************/
  /* A zero-length value name indicates the default key value.         */
  /*********************************************************************/
  if (!pszValueName[0])
  {
    strcpy(szValueNameText, "{Default}");      // Indicate default value
  }
  else
  {
    sprintf(szValueNameText, "\"%s\"", pszValueName); // Name in quotes
  }

  /********************************************************************/
  /* Process the value according to value type                        */
  /********************************************************************/
  switch(dwValueType)
  {
    case REG_SZ:
    case REG_EXPAND_SZ:
      /****************************************************************/
      /* String value: turn backslashes into double backslashes       */
      /* because the string's in double-quotes.                       */
      /****************************************************************/
      szValueData[0] = 0;              // Initialise Target string
      pStart = pValueData;
      pSlash = _mbschr(pStart, '\\');  // Look for first backslash
      while(pSlash)
      {
        pNext = _mbsinc(pSlash);       // Point past backslash
        pSlash[0] = 0;                 // Terminate string for concat.
        _mbscat(szValueData, pStart);  // Add next part of string
        _mbscat(szValueData, "\\\\");  // Add double backslash
        pStart = pNext;                // Start of next part of string
        pSlash = _mbschr(pStart, '\\'); // Look for next backslash
      }
      _mbscat(szValueData, pStart);    // Add last part of string

      sprintf(szRegValue, "%s=\"%s\"", szValueNameText, szValueData);
      printf("    %s%s\n",gSpaces, szRegValue); // Log the value string
      break;

    case REG_DWORD:
      /****************************************************************/
      /* DWORD value                                                  */
      /****************************************************************/
      sprintf(szValueData, "dword:%8.8x(hex) %u(decimal)", *(DWORD *)pValueData,
		  *(DWORD *)pValueData);
      sprintf(szRegValue, "%s=%s", szValueNameText, szValueData);
      printf("    %s%s\n",gSpaces, szRegValue); // Log the value string
      break;

    case REG_BINARY:
    case 0x07:
      /****************************************************************/
      /* BINARY value                                                 */
      /* 0x07 seems to be zero length binary on NT                    */
      /****************************************************************/

          printf("    %s%s:binary of length %u\n",gSpaces,
                    szValueNameText,dwcbValueData);
          if (dwcbValueData)
          {
            int count;
            int count16=0;
            for(count=0;count<(int)dwcbValueData;count++,count16++)
            {
              if (count16 == 16)
              {
                printf("\n");
                count16 = 0;
              }
              if (count16 == 0)
              {
                 printf("        %s ",gSpaces);
              }
              printf("%02X " ,*(pValueData+count));
            }
            printf("\n");
          }
          break;

    default:
      /****************************************************************/
      /* We don't support all possible value types                    */
      /****************************************************************/
      printf("!!!Unsupported type: %s 0x%x\n",
                      szValueNameText, dwValueType);
      break;
  }



  /*********************************************************************/
  /* Return to caller                                                  */
  /*********************************************************************/
  return(fSuccess);
} /* end of PrintKeyValue */


