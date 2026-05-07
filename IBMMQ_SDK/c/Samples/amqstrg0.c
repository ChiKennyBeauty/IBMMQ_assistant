/* @(#) MQMBID sn=p944-L251007.1.DE su=f84fdb91fb9772dfb286437c1f42e7849ffc8dc7 pn=samples/c/amqstrg0.c */
 /********************************************************************/
 /*                                                                  */
 /* Program name: AMQSTRG0                                           */
 /*                                                                  */
 /* Description: Sample C program that acts as a trigger monitor     */
 /*   <copyright                                                     */
 /*   notice="lm-source-program"                                     */
 /*   pids="5724-H72,"                                               */
 /*   years="1994,2019"                                              */
 /*   crc="4156666121" >                                             */
 /*   Licensed Materials - Property of IBM                           */
 /*                                                                  */
 /*   5724-H72,                                                      */
 /*                                                                  */
 /*   (C) Copyright IBM Corp. 1994, 2019 All Rights Reserved.        */
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
 /*   AMQSTRG0 is a sample C program that acts as a trigger          */
 /*   monitor - reads an initiation queue, then starts               */
 /*   the program associated with each trigger message               */
 /*                                                                  */
 /*   Note - the function shown in this sample is a subset           */
 /*          of the full triggering function that is in              */
 /*          the supplied runmqtrm command.  Trigger messages        */
 /*          in error are not written to the Dead Letter Queue;      */
 /*          triggering only applies to the DEFault ApplType         */
 /*                                                                  */
 /*      -- reads messages from an initiation queue named in         */
 /*         the parameter to the trigger monitor                     */
 /*                                                                  */
 /*      -- performs command for each valid trigger message          */
 /*                                                                  */
 /*         -- command runs command named in ApplId                  */
 /*         -- parm = string version of MQTMC2 structure             */
 /*         -- EnvData is used by the trigger monitor as an          */
 /*            extension to the invoking command string              */
 /*                                                                  */
 /*      -- writes a message for each MQI reason other than          */
 /*         MQRC_NONE; stops if there is a MQI completion code       */
 /*         of MQCC_FAILED                                           */
 /*                                                                  */
 /*                                                                  */
 /*   Program logic:                                                 */
 /*      Use program parameter as the initiation queue name          */
 /*      MQOPEN queue for INPUT                                      */
 /*      while no MQI failures,                                      */
 /*      .  MQGET next message, remove from queue                    */
 /*      .  invoke command based on trigger messages                 */
 /*      .  .  ApplId   is name of program to call                   */
 /*      .  .  MQTMC2   is parameter                                 */
 /*      MQCLOSE the trigger queue                                   */
 /*                                                                  */
 /*                                                                  */
 /********************************************************************/
 /*                                                                  */
 /*   AMQSTRG0 has two parameters,                                   */
 /*                   - name of initiation queue (required)          */
 /*                   - queue manager name (optional)                */
 /*                                                                  */
 /********************************************************************/
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
    /* includes for MQI  */
 #include <cmqc.h>

#if (MQAT_DEFAULT == MQAT_WINDOWS_NT)
   /* includes for Get and SetConsoleOutputCP */
 #include <windows.h>
#endif

 int main(int argc, char **argv)
 {
   int  i;  /* auxiliary counter */

   /*   Declare MQI structures needed                                */
   MQOD     od = {MQOD_DEFAULT};    /* Object Descriptor             */
   MQMD     md = {MQMD_DEFAULT};    /* Message Descriptor            */
   MQGMO   gmo = {MQGMO_DEFAULT};   /* get message options           */
      /** note, sample uses defaults where it can **/
   MQTM     trig;                   /* trigger message buffer        */

   MQHCONN  Hcon;                   /* connection handle             */
   MQHOBJ   Hobj;                   /* object handle                 */
   MQLONG   O_options;              /* MQOPEN options                */
   MQLONG   C_options;              /* MQCLOSE options               */
   MQLONG   CompCode;               /* completion code               */
   MQLONG   OpenCode;               /* MQOPEN completion code        */
   MQLONG   Reason;                 /* reason code                   */
   MQLONG   CReason;                /* reason code for MQCONN        */
   MQLONG   buflen;                 /* buffer length                 */
   MQLONG   triglen;                /* message length received       */

   MQCHAR   command[1100];          /* call command string ...       */
   MQCHAR   p1[600];                /* ApplId insert                 */
   MQCHAR   p2[900];                /* trigger insert                */
   MQCHAR   p3[600];                /* Environment insert            */
   char     QMName[50];             /* queue manager name            */

   printf("Sample AMQSTRG0 start\n");
   if (argc < 2)
   {
     printf("Required parameter missing - initiation queue name\n");
     exit(99);     /* stop if no parameter */
   }

   /******************************************************************/
   /*                                                                */
   /*   Initialize object descriptor for subject queue               */
   /*                                                                */
   /******************************************************************/
   strncpy(od.ObjectName, argv[1], MQ_Q_NAME_LENGTH);

   /******************************************************************/
   /*                                                                */
   /*   Connect to queue manager                                     */
   /*                                                                */
   /******************************************************************/
   QMName[0] = 0;    /* default */
   if (argc > 2)
     strncpy(QMName, argv[2], MQ_Q_MGR_NAME_LENGTH);

   MQCONN(QMName,                  /* queue manager                  */
          &Hcon,                   /* connection handle              */
          &CompCode,               /* completion code                */
          &CReason);               /* reason code                    */

   /* report reason and stop if it failed     */
   if (CompCode == MQCC_FAILED)
   {
     printf("MQCONN ended with reason code %d\n", CReason);
     exit(CReason);
   }

   /******************************************************************/
   /*                                                                */
   /*  Open specified initiation queue for input; exclusive or       */
   /*  shared use of the queue is controlled by the queue definition */
   /*                                                                */
   /******************************************************************/
   O_options = MQOO_INPUT_AS_Q_DEF /* open queue for input           */
           | MQOO_FAIL_IF_QUIESCING; /* but not if MQM stopping      */
   MQOPEN(Hcon,                    /* connection handle              */
          &od,                     /* object descriptor for queue    */
          O_options,               /* open options                   */
          &Hobj,                   /* object handle                  */
          &CompCode,               /* completion code                */
          &Reason);                /* reason code                    */

   /* report reason, if any; stop if failed      */
   if (Reason != MQRC_NONE)
   {
     printf("MQOPEN (%s) ==> %d\n", od.ObjectName, Reason);
   }

   OpenCode = CompCode;                /* keep for conditional close */

   /******************************************************************/
   /*                                                                */
   /*   Get messages from the message queue                          */
   /*   Loop until there is a failure, ask to fail if the queue      */
   /*   manager is quiescing                                         */
   /*                                                                */
   /******************************************************************/
   buflen = sizeof(trig);          /* size of all trigger messages   */
   gmo.Version = MQGMO_VERSION_2;     /* Avoid need to reset Message */
   gmo.MatchOptions = MQMO_NONE;      /* ID and Correlation ID after */
                                      /* every MQGET                 */
  gmo.Options = MQGMO_WAIT              /* wait for new messages ... */
     | MQGMO_FAIL_IF_QUIESCING          /* or until MQM stopping     */
     | MQGMO_ACCEPT_TRUNCATED_MSG       /* remove all long messages  */
     | MQGMO_NO_SYNCPOINT;              /* No syncpoint              */
   gmo.WaitInterval = MQWI_UNLIMITED;   /* no time limit             */

   while (CompCode != MQCC_FAILED)
   {

     /****************************************************************/
     /*                                                              */
     /*   Wait for a trigger                                         */
     /*                                                              */
     /****************************************************************/
     MQGET(Hcon,                /* connection handle                */
           Hobj,                /* object handle                    */
           &md,                 /* message descriptor               */
           &gmo,                /* get message options              */
           buflen,              /* buffer length                    */
           &trig,               /* trigger message buffer           */
           &triglen,            /* message length                   */
           &CompCode,           /* completion code                  */
           &Reason);            /* reason code                      */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       printf("MQGET ==> %d\n", Reason);
     }

     /****************************************************************/
     /*                                                              */
     /*   Process each message received                              */
     /*                                                              */
     /****************************************************************/
     if (CompCode != MQCC_FAILED)
     {
       if (triglen != buflen)
         printf("DataLength = %d?\n", triglen);
       else
       {
         /************************************************************/
         /*                                                          */
         /*   Copy appropriate parts of trigger message into the     */
         /*   call command string                                    */
         /*                                                          */
         /************************************************************/
         memcpy(p1, trig.ApplId, sizeof(trig.ApplId));
         memcpy(p3, trig.EnvData, sizeof(trig.EnvData));

         /* convert MQTM to MQTMC */
         memcpy(&trig.StrucId,MQTMC_STRUC_ID,sizeof(MQCHAR4));
         memcpy(&trig.Version, MQTMC_VERSION_2, 4); /* replace integers */
         memcpy(&trig.ApplType, "    ", 4);
         memcpy(p2, &trig, triglen);    /* copy modified trigger */
         p2[triglen] = '\0';

         /* strip trailing blanks */
         for (i=sizeof(trig.ApplId)-1; i>=0; i--)
           if (p1[i] != ' ')
             break;
         p1[i+1] = '\0';

         /* strip trailing blanks */
         for (i=sizeof(trig.EnvData)-1; i>=0; i--)
           if (p3[i] != ' ')
             break;
         p3[i+1] = '\0';

         sprintf(command, "%s \"%s%s\" %s", p1, p2, QMName, p3);

         {
#if (MQAT_DEFAULT == MQAT_WINDOWS_NT)
           /**********************************************************/
           /* Save and restore console codepage to prevent a trigger */
           /* program from modifying the monitors environment        */
           /**********************************************************/
           UINT console_cp = GetConsoleOutputCP(); 
#endif
           /**********************************************************/
           /*                                                        */
           /*   Call the program                                     */
           /*                                                        */
           /**********************************************************/
           printf("%s;\n", command);
           system(command);
#if (MQAT_DEFAULT == MQAT_WINDOWS_NT)
           if (console_cp) SetConsoleOutputCP(console_cp);
#endif
         }
       }   /* end trigger processing         */
     }     /* end process for successful GET */
   }       /* end message processing loop    */

   /******************************************************************/
   /*                                                                */
   /*   Close the initiation queue - if it was opened                */
   /*                                                                */
   /******************************************************************/
   if (OpenCode != MQCC_FAILED)
   {
     C_options = 0;                 /* no close options              */
     MQCLOSE(Hcon,                  /* connection handle             */
             &Hobj,                 /* object handle                 */
             C_options,
             &CompCode,             /* completion code               */
             &Reason);              /* reason code                   */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       printf("MQCLOSE ==> %d\n", Reason);
     }
   }

   /******************************************************************/
   /*                                                                */
   /*   Disconnect from MQM  (unless previously connected)           */
   /*                                                                */
   /******************************************************************/
   if (CReason != MQRC_ALREADY_CONNECTED)
   {
     MQDISC(&Hcon,                   /* connection handle            */
            &CompCode,               /* completion code              */
            &Reason);                /* reason code                  */

     /* report reason, if any     */
     if (Reason != MQRC_NONE)
     {
       printf("MQDISC ended with reason code %d\n", Reason);
     }
   }

   /******************************************************************/
   /*                                                                */
   /* END OF AMQSTRG0                                                */
   /*                                                                */
   /******************************************************************/
   printf("Sample AMQSTRG0 end\n");
   return(0);
 }
