/****************************************************************************
 * drivers/usbdev/usbdev_strings.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>

#include <nuttx/usb/usbdev_trace.h>

#ifdef CONFIG_USBDEV_TRACE_STRINGS

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* Class API call strings that may be enabled for more descriptive USB trace
 * output.
 */

const struct trace_msg_t g_usb_trace_strings_clsapi[] =
{
#if defined(CONFIG_CDCACM) || defined(CONFIG_PL2303)
  TRACE_STR(USBSER_TRACECLASSAPI_SETUP),
  TRACE_STR(USBSER_TRACECLASSAPI_SHUTDOWN),
  TRACE_STR(USBSER_TRACECLASSAPI_ATTACH),
  TRACE_STR(USBSER_TRACECLASSAPI_DETACH),
  TRACE_STR(USBSER_TRACECLASSAPI_IOCTL),
  TRACE_STR(USBSER_TRACECLASSAPI_RECEIVE),
  TRACE_STR(USBSER_TRACECLASSAPI_RXINT),
  TRACE_STR(USBSER_TRACECLASSAPI_RXAVAILABLE),
  TRACE_STR(USBSER_TRACECLASSAPI_SEND),
  TRACE_STR(USBSER_TRACECLASSAPI_TXINT),
  TRACE_STR(USBSER_TRACECLASSAPI_TXREADY),
  TRACE_STR(USBSER_TRACECLASSAPI_TXEMPTY),
#if defined(CONFIG_CDCACM_IFLOWCONTROL)
  TRACE_STR(USBSER_TRACECLASSAPI_FLOWCONTROL),
#endif
#endif
  TRACE_STR_END
};

/* Class state strings that may be enabled for more descriptive USB trace
 * output.
 */

const struct trace_msg_t g_usb_trace_strings_clsstate[] =
{
#if defined(CONFIG_USBMSC)
  TRACE_STR(USBMSC_CLASSSTATE_IDLECMDPARSE),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDFINISH),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDREAD6),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDREAD10),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDREAD12),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDWRITE6),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDWRITE10),
  TRACE_STR(USBMSC_CLASSSTATE_CMDPARSECMDWRITE12),
  TRACE_STR(USBMSC_CLASSSTATE_CMDREAD),
  TRACE_STR(USBMSC_CLASSSTATE_CMDREADCMDFINISH),
  TRACE_STR(USBMSC_CLASSSTATE_CMDWRITE),
  TRACE_STR(USBMSC_CLASSSTATE_CMDWRITECMDFINISH),
  TRACE_STR(USBMSC_CLASSSTATE_CMDFINISHCMDSTATUS),
  TRACE_STR(USBMSC_CLASSSTATE_CMDSTATUSIDLE),
#endif
  TRACE_STR_END
};

/* Class error strings that may be enabled for more descriptive USB trace
 * output.
 */

const struct trace_msg_t g_usb_trace_strings_clserror[] =
{
#if defined(CONFIG_CDCACM) || defined(CONFIG_PL2303)
  TRACE_STR(USBSER_TRACEERR_ALLOCCTRLREQ),
  TRACE_STR(USBSER_TRACEERR_ALLOCDEVSTRUCT),
  TRACE_STR(USBSER_TRACEERR_ALREADYCLOSED),
  TRACE_STR(USBSER_TRACEERR_ALREADYCONFIGURED),
  TRACE_STR(USBSER_TRACEERR_CONFIGIDBAD),
  TRACE_STR(USBSER_TRACEERR_CONFIGNONE),
  TRACE_STR(USBSER_TRACEERR_CONSOLEREGISTER),
  TRACE_STR(USBSER_TRACEERR_DEVREGISTER),
  TRACE_STR(USBSER_TRACEERR_EPRESPQ),
  TRACE_STR(USBSER_TRACEERR_GETUNKNOWNDESC),
  TRACE_STR(USBSER_TRACEERR_INVALIDARG),
  TRACE_STR(USBSER_TRACEERR_EP0NOTBOUND),
  TRACE_STR(USBSER_TRACEERR_EPBULKINALLOCFAIL),
  TRACE_STR(USBSER_TRACEERR_EPBULKINCONFIGFAIL),
  TRACE_STR(USBSER_TRACEERR_EPBULKOUTALLOCFAIL),
  TRACE_STR(USBSER_TRACEERR_EPINTINALLOCFAIL),
  TRACE_STR(USBSER_TRACEERR_EPINTINCONFIGFAIL),
  TRACE_STR(USBSER_TRACEERR_EPBULKOUTCONFIGFAIL),
  TRACE_STR(USBSER_TRACEERR_RDALLOCREQ),
  TRACE_STR(USBSER_TRACEERR_RDSHUTDOWN),
  TRACE_STR(USBSER_TRACEERR_RDSUBMIT),
  TRACE_STR(USBSER_TRACEERR_RDUNEXPECTED),
  TRACE_STR(USBSER_TRACEERR_REQRESULT),
  TRACE_STR(USBSER_TRACEERR_RXOVERRUN),
  TRACE_STR(USBSER_TRACEERR_SETUPNOTCONNECTED),
  TRACE_STR(USBSER_TRACEERR_SUBMITFAIL),
  TRACE_STR(USBSER_TRACEERR_UARTREGISTER),
  TRACE_STR(USBSER_TRACEERR_UARTUNREGISTER),
  TRACE_STR(USBSER_TRACEERR_UNSUPPORTEDCTRLREQ),
  TRACE_STR(USBSER_TRACEERR_UNSUPPORTEDCLASSREQ),
  TRACE_STR(USBSER_TRACEERR_UNSUPPORTEDSTDREQ),
  TRACE_STR(USBSER_TRACEERR_UNSUPPORTEDTYPE),
  TRACE_STR(USBSER_TRACEERR_WRALLOCREQ),
  TRACE_STR(USBSER_TRACEERR_WRSHUTDOWN),
  TRACE_STR(USBSER_TRACEERR_WRUNEXPECTED),
#endif
#if defined(CONFIG_USBDEV_COMPOSITE)
  TRACE_STR(USBCOMPOSITE_TRACEERR_REQRESULT),
  TRACE_STR(USBCOMPOSITE_TRACEERR_ALLOCCTRLREQ),
  TRACE_STR(USBCOMPOSITE_TRACEERR_INVALIDARG),
  TRACE_STR(USBCOMPOSITE_TRACEERR_EP0NOTBOUND),
  TRACE_STR(USBCOMPOSITE_TRACEERR_SETUPINVALIDARGS),
  TRACE_STR(USBCOMPOSITE_TRACEERR_EP0NOTBOUND2),
  TRACE_STR(USBCOMPOSITE_TRACEERR_GETUNKNOWNDESC),
  TRACE_STR(USBCOMPOSITE_TRACEERR_UNSUPPORTEDSTDREQ),
  TRACE_STR(USBCOMPOSITE_TRACEERR_EPRESPQ),
  TRACE_STR(USBCOMPOSITE_TRACEERR_ALLOCDEVSTRUCT),
  TRACE_STR(USBCOMPOSITE_TRACEERR_CLASSOBJECT),
  TRACE_STR(USBCOMPOSITE_TRACEERR_DEVREGISTER),
#endif
#if defined(CONFIG_USBMSC)
  TRACE_STR(USBMSC_TRACEERR_ALLOCCTRLREQ),
  TRACE_STR(USBMSC_TRACEERR_ALLOCDEVSTRUCT),
  TRACE_STR(USBMSC_TRACEERR_ALLOCIOBUFFER),
  TRACE_STR(USBMSC_TRACEERR_ALREADYCONFIGURED),
  TRACE_STR(USBMSC_TRACEERR_ALREADYUNINIT),
  TRACE_STR(USBMSC_TRACEERR_BADREQUEST),
  TRACE_STR(USBMSC_TRACEERR_BINDLUNINVALIDARGS2),
  TRACE_STR(USBMSC_TRACEERR_BINDLUNINVALIDARGS3),
  TRACE_STR(USBMSC_TRACEERR_BINDLUNINVALIDARGS4),
  TRACE_STR(USBMSC_TRACEERR_BINLUNINVALIDARGS1),
  TRACE_STR(USBMSC_TRACEERR_BLKDRVEOPEN),
  TRACE_STR(USBMSC_TRACEERR_CMDBADLUN),
  TRACE_STR(USBMSC_TRACEERR_CMDFINISHRESIDUE),
  TRACE_STR(USBMSC_TRACEERR_CMDFINISHRQEMPTY),
  TRACE_STR(USBMSC_TRACEERR_CMDFINISHSHORTPKT),
  TRACE_STR(USBMSC_TRACEERR_CMDFINISHSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_CMDFINSHDIR),
  TRACE_STR(USBMSC_TRACEERR_CMDFINSHSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_CMDPARSEWRREQLISTEMPTY),
  TRACE_STR(USBMSC_TRACEERR_CMDREADREADFAIL),
  TRACE_STR(USBMSC_TRACEERR_CMDREADSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_CMDREADWRRQEMPTY),
  TRACE_STR(USBMSC_TRACEERR_CMDSTATUSWRREQLISTEMPTY),
  TRACE_STR(USBMSC_TRACEERR_CMDUNEVIOLATION),
  TRACE_STR(USBMSC_TRACEERR_CMDWRITERDSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_CMDWRITERDRQEMPTY),
  TRACE_STR(USBMSC_TRACEERR_CMDWRITEWRITEFAIL),
  TRACE_STR(USBMSC_TRACEERR_CONFIGIDBAD),
  TRACE_STR(USBMSC_TRACEERR_CONFIGNONE),
  TRACE_STR(USBMSC_TRACEERR_DEFERREDRESPINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_DEFERREDRESPSTALLED),
  TRACE_STR(USBMSC_TRACEERR_DEFERREDRESPSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_DEVREGISTER),
  TRACE_STR(USBMSC_TRACEERR_DISCONNECTINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_EP0NOTBOUND1),
  TRACE_STR(USBMSC_TRACEERR_EP0NOTBOUND2),
  TRACE_STR(USBMSC_TRACEERR_EP0NOTBOUND3),
  TRACE_STR(USBMSC_TRACEERR_EPBULKINALLOCFAIL),
  TRACE_STR(USBMSC_TRACEERR_EPBULKINCONFIGFAIL),
  TRACE_STR(USBMSC_TRACEERR_EPBULKOUTALLOCFAIL),
  TRACE_STR(USBMSC_TRACEERR_EPBULKOUTCONFIGFAIL),
  TRACE_STR(USBMSC_TRACEERR_EPRESPQ),
  TRACE_STR(USBMSC_TRACEERR_EXPORTLUNSINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_GETMAXLUNNDX),
  TRACE_STR(USBMSC_TRACEERR_GETUNKNOWNDESC),
  TRACE_STR(USBMSC_TRACEERR_IDLERDREQLISTEMPTY),
  TRACE_STR(USBMSC_TRACEERR_IDLERDSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_INQUIRYFLAGS),
  TRACE_STR(USBMSC_TRACEERR_INTERNALCONFUSION1),
  TRACE_STR(USBMSC_TRACEERR_INTERNALCONFUSION2),
  TRACE_STR(USBMSC_TRACEERR_INVALIDCBWCONTENT),
  TRACE_STR(USBMSC_TRACEERR_INVALIDCBWSIGNATURE),
  TRACE_STR(USBMSC_TRACEERR_INVALIDSTATE),
  TRACE_STR(USBMSC_TRACEERR_LUNALREADYBOUND),
  TRACE_STR(USBMSC_TRACEERR_LUNNOTBOUND),
  TRACE_STR(USBMSC_TRACEERR_MODEPAGEFLAGS),
  TRACE_STR(USBMSC_TRACEERR_MODESENSE10FLAGS),
  TRACE_STR(USBMSC_TRACEERR_MODESENSE6FLAGS),
  TRACE_STR(USBMSC_TRACEERR_MSRESETNDX),
  TRACE_STR(USBMSC_TRACEERR_NOGEOMETRY),
  TRACE_STR(USBMSC_TRACEERR_NOTCONFIGURED),
  TRACE_STR(USBMSC_TRACEERR_NOTREMOVABLE),
  TRACE_STR(USBMSC_TRACEERR_PCSAVED),
  TRACE_STR(USBMSC_TRACEERR_PHASEERROR1),
  TRACE_STR(USBMSC_TRACEERR_PHASEERROR2),
  TRACE_STR(USBMSC_TRACEERR_PHASEERROR3),
  TRACE_STR(USBMSC_TRACEERR_PREVENTMEDIUMREMOVALPREVENT),
  TRACE_STR(USBMSC_TRACEERR_RDALLOCREQ),
  TRACE_STR(USBMSC_TRACEERR_RDCOMPLETEINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_RDCOMPLETERDSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_RDSHUTDOWN),
  TRACE_STR(USBMSC_TRACEERR_RDSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_RDUNEXPECTED),
  TRACE_STR(USBMSC_TRACEERR_READ10FLAGS),
  TRACE_STR(USBMSC_TRACEERR_READ10LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_READ10MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_READ12FLAGS),
  TRACE_STR(USBMSC_TRACEERR_READ12LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_READ12MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_READ6LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_READ6MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_READCAPACITYFLAGS),
  TRACE_STR(USBMSC_TRACEERR_REALLOCIOBUFFER),
  TRACE_STR(USBMSC_TRACEERR_REQRESULT),
  TRACE_STR(USBMSC_TRACEERR_SCSICMDCONTROL),
  TRACE_STR(USBMSC_TRACEERR_SETCONFIGINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_SETUPINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_SNDCSWFAIL),
  TRACE_STR(USBMSC_TRACEERR_SNDPHERROR),
  TRACE_STR(USBMSC_TRACEERR_SNDSTATUSSUBMIT),
  TRACE_STR(USBMSC_TRACEERR_SYNCCACHEMEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_THREADCREATE),
  TRACE_STR(USBMSC_TRACEERR_TOOMANYLUNS),
  TRACE_STR(USBMSC_TRACEERR_UNBINDINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_UNBINDLUNINVALIDARGS1),
  TRACE_STR(USBMSC_TRACEERR_UNBINDLUNINVALIDARGS2),
  TRACE_STR(USBMSC_TRACEERR_UNINITIALIZEINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_UNSUPPORTEDSTDREQ),
  TRACE_STR(USBMSC_TRACEERR_VERIFY10FLAGS),
  TRACE_STR(USBMSC_TRACEERR_VERIFY10LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_VERIFY10MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_VERIFY10NOBLOCKS),
  TRACE_STR(USBMSC_TRACEERR_VERIFY10READFAIL),
  TRACE_STR(USBMSC_TRACEERR_WRALLOCREQ),
  TRACE_STR(USBMSC_TRACEERR_WRCOMPLETEINVALIDARGS),
  TRACE_STR(USBMSC_TRACEERR_WRITE10FLAGS),
  TRACE_STR(USBMSC_TRACEERR_WRITE10LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_WRITE10MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_WRITE10READONLY),
  TRACE_STR(USBMSC_TRACEERR_WRITE12FLAGS),
  TRACE_STR(USBMSC_TRACEERR_WRITE12LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_WRITE12MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_WRITE12READONLY),
  TRACE_STR(USBMSC_TRACEERR_WRITE6LBARANGE),
  TRACE_STR(USBMSC_TRACEERR_WRITE6MEDIANOTPRESENT),
  TRACE_STR(USBMSC_TRACEERR_WRITE6READONLY),
  TRACE_STR(USBMSC_TRACEERR_WRSHUTDOWN),
  TRACE_STR(USBMSC_TRACEERR_WRUNEXPECTED),
  TRACE_STR(USBMSC_TRACEERR_UNSUPPORTEDTYPE),
#endif
  TRACE_STR_END
};

/****************************************************************************
 * Public Functions
 ****************************************************************************/

#endif /* CONFIG_USBDEV_TRACE_STRINGS */
