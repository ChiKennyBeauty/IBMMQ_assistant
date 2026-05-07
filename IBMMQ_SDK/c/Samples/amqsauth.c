 /* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/amqsauth.c */
 /********************************************************************/
 /*                                                                  */
 /* Program name: AMQSAUTH                                           */
 /*                                                                  */
 /* Description: Set of functions which provide authentication       */
 /*              functionality to C sample programs.                 */
 /*   <copyright                                                     */
 /*   notice="lm-source-program"                                     */
 /*   pids="5724-H72"                                                */
 /*   years="1994,2024"                                              */
 /*   crc="2248028677" >                                             */
 /*   Licensed Materials - Property of IBM                           */
 /*                                                                  */
 /*   5724-H72                                                       */
 /*                                                                  */
 /*   (C) Copyright IBM Corp. 1994, 2024 All Rights Reserved.        */
 /*                                                                  */
 /*   US Government Users Restricted Rights - Use, duplication or    */
 /*   disclosure restricted by GSA ADP Schedule Contract with        */
 /*   IBM Corp.                                                      */
 /*   </copyright>                                                   */
 /********************************************************************/
 /*                                                                  */
 /* Function:                                                        */
 /*                                                                  */
 /*                                                                  */
 /*   AMQSAUTH is a set of functions which provide authentication    */
 /*              functionality to C sample programs.                 */
 /*                                                                  */
 /********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* includes for MQI */
#include <cmqc.h>
/* header to include auth functions to sample programs */
#include "./amqsauth.h"

/* Platform includes for masked input */
#if (MQAT_DEFAULT == MQAT_OS400)
  #include <qp0ztrml.h>
#elif (MQAT_DEFAULT == MQAT_WINDOWS_NT)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <io.h>
#elif (MQAT_DEFAULT == MQAT_UNIX)
  #include <termios.h>
  #include <unistd.h>
#endif

char     UserTok[MQ_CSP_TOKEN_LENGTH +1] = {0};        /* For auth  */
char     Password[MQ_CSP_PASSWORD_LENGTH + 1] = {0}; /* For auth  */
 /********************************************************************/
 /* Function name:    get_auth_info                                  */
 /*                                                                  */
 /* Description:      Setup any authentication information supplied  */
 /*                   in the local environment.                      */
 /*                                                                  */
 /* Called by:        main in sample programs with authetication     */
 /*                   capabilities.                                  */
 /*                                                                  */
 /* Receives:         Variables for User ID/Token and password and   */
 /*                   and their length.                              */
 /*                                                                  */
 /* Calls:            get_password function                          */
 /*                                                                  */
 /********************************************************************/
 void getAuthInfo(MQCNO* cno, MQCSP* csp)
 {
   char *userData = getenv("MQSAMP_USER_ID");
   if (userData != NULL)
   {
     strncpy(UserTok, userData, sizeof(UserTok) - 1);
     UserTok[sizeof(UserTok) - 1] = 0;

     /****************************************************************/
     /* Get the password, using masked input if possible             */
     /****************************************************************/
     printf("Enter password: ");
     get_password(Password,sizeof(Password) - 1);
     if (strlen(Password) > 0 && Password[strlen(Password) - 1] == '\n')
       Password[strlen(Password) - 1] = 0;

     /****************************************************************/
     /* Set the connection options to use the security structure and */
     /* set version information to ensure the structure is processed.*/
     /****************************************************************/
     cno->SecurityParmsPtr = csp;
     cno->Version = MQCNO_VERSION_5;

     csp->AuthenticationType = MQCSP_AUTH_USER_ID_AND_PWD;
     csp->CSPUserIdPtr = UserTok;
     csp->CSPUserIdLength = (MQLONG)strlen(UserTok);

     csp->CSPPasswordPtr = Password;
     csp->CSPPasswordLength = (MQLONG)strlen(csp->CSPPasswordPtr);
   }
   if (getenv("MQSAMP_TOKEN") != NULL)
   {
     if (userData != NULL)
     {
       printf("Cannot supply both a token and a password\n");
       exit(99);
     }
    /****************************************************************/
    /* Get the token, using masked input if possible                */
    /* This shows one method of obtaining a token, in practice      */
    /* a programmatic method to obtain the token directly from the  */
    /* authorization server is likely to be used.                   */
    /****************************************************************/
    printf("Enter token: ");
    get_password(UserTok,sizeof(UserTok)-1);
    if (strlen(UserTok) > 0 && UserTok[strlen(UserTok) - 1] == '\n')
      UserTok[strlen(UserTok) - 1] = 0;

    /****************************************************************/
    /* Set the connection options to use the security structure and */
    /* set version information to ensure the structure is processed.*/
    /****************************************************************/
    cno->SecurityParmsPtr = csp;
    cno->Version = MQCNO_VERSION_5;

    csp->Version = MQCSP_VERSION_3;
    csp->AuthenticationType = MQCSP_AUTH_ID_TOKEN;
    csp->TokenPtr = UserTok;
    csp->TokenLength = (MQLONG)strlen(UserTok);
   }
 }

 /********************************************************************/
 /* Function name:    get_password                                   */
 /*                                                                  */
 /* Description:      Gets a password string from stdin, if possible */
 /*                   using masked input.                            */
 /*                                                                  */
 /* Called by:        getAuthInfo                                    */
 /*                                                                  */
 /* Receives:         buffer and size                                */
 /*                                                                  */
 /* Calls:            platform specific functions / fgets            */
 /*                                                                  */
 /********************************************************************/
#if (MQAT_DEFAULT == MQAT_OS400)
 void get_password(char *buffer, size_t size)
 {
   if (Qp0zIsATerminal(fileno(stdin)))
   {
     Qp0zSetTerminalMode( QP0Z_TERMINAL_INPUT_MODE, QP0Z_TERMINAL_HIDDEN, NULL );
     fgets(buffer, size, stdin);
     Qp0zSetTerminalMode( QP0Z_TERMINAL_INPUT_MODE, QP0Z_TERMINAL_PREVIOUS, NULL );
   }
   else
   {
     fgets(buffer, size, stdin);
   }
 }
#elif (MQAT_DEFAULT == MQAT_WINDOWS_NT)
 void get_password(char *buffer, size_t size)
 {
   int c;
   size_t i;
   HANDLE h;
   DWORD  readChars, oldMode, mode;
   BOOL b;
   char charBuf[1];

   h = GetStdHandle(STD_INPUT_HANDLE);
   if (_isatty(fileno(stdin)) && h != INVALID_HANDLE_VALUE)
   {
     GetConsoleMode(h, &mode);
     oldMode = mode;
     mode = (mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
     SetConsoleMode(h, mode);

     i=0;
     do
     {
       b = ReadConsole(h, charBuf, 1, &readChars, NULL);
       c = charBuf[0];
       if (b && readChars != 0 && c != '\n' && c != '\r')
       {
         if (c == '\b')
         {
           if (i > 0)
           {
             buffer[--i]=0;
             fprintf(stdout, "\b \b");
             fflush(stdout);
           }
         }
         else
         {
           fputc('*', stdout);
           fflush(stdout);
           buffer[i++] = c;
         }
       }
     } while (b && c != '\n' && c != '\r' && i <= size);
     printf("\n");
     SetConsoleMode(h, oldMode);
   }
   else
   {
     fgets(buffer, (int)size, stdin);
   }
 }
#elif (MQAT_DEFAULT == MQAT_UNIX)
 void get_password(char *buffer, size_t size)
 {
   int c;
   size_t i;
   struct termios savetty, newtty;
   const char BACKSPACE=8;
   const char DELETE=127;
   const char RETURN=10;
   int min = 1;
   int time = 0;

   if (isatty(fileno(stdin)))
   {
     tcgetattr(fileno(stdin), &savetty);
     newtty = savetty;
     newtty.c_cc[VMIN] = min;
     newtty.c_cc[VTIME] = time;
     newtty.c_lflag &= ~(ECHO|ICANON);
     tcsetattr(fileno(stdin), TCSANOW, &newtty);

     i=0;
     do
     {
       c = fgetc(stdin);
       if (c != EOF && c != RETURN)
       {
         if ( (c == BACKSPACE) || (c == DELETE) )
         {
           if (i > 0)
           {
             buffer[--i]=0;
             fprintf(stdout, "\b \b");
             fflush(stdout);
           }
         }
         else
         {
           fputc('*', stdout);
           fflush(stdout);
           buffer[i++] = c;
         }
       }
       else
       {
         buffer[i]=0;
       }
     } while (c != EOF && c != RETURN && i <= size);

     printf("\n");
     fflush(stdout);
     tcsetattr(fileno(stdin), TCSANOW, &savetty);
   }
   else
   {
     fgets(buffer, size, stdin);
   }
 }
#else
 void get_password(char *buffer, size_t size)
 {
   fgets(buffer, (int)size, stdin);
 }
#endif