###################################################################
# This file is used as input to the amquregn program shipped
# with IBM MQ. It is a list of keys from the Windows
# registry that (together with their subkeys) will have their
# values dumped to the console or to a redirected file. The
# details held under these keys may be useful to the IBM Service
# organisation in helping debug problems.
#
#  Note 1: Valid registry roots accepted by the utility are
#
#       HKEY_LOCAL_MACHINE   aka HKLM
#       HKEY_CLASSES_ROOT    aka HKCR
#       HKEY_CURRENT_USER    aka HKCU
#       HKEY_USERS           aka HKU
#       HKEY_CURRENT_CONFIG  aka HKCC
#       HKEY_DYN_DATA        aka HKDD
#
#  Note 2: You should ensure that any amendments to this file
#          do not have trailing spaces (unless of course the
#          registry key has trailing spaces).
# <copyright
# notice="lm-source-program"
# pids="5724-H72"
# years="2001,2016"
# crc="4231032549" >
# Licensed Materials - Property of IBM
#
# 5724-H72
#
# (C) Copyright IBM Corp. 2001, 2016 All Rights Reserved.
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with
# IBM Corp.
# </copyright>
###################################################################
HKEY_LOCAL_MACHINE\SOFTWARE\IBM\WebSphere MQ
HKEY_LOCAL_MACHINE\SOFTWARE\IBM\MQSeries
HKLM\SYSTEM\CurrentControlSet\Services
