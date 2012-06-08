/*----------------------------------------------------------------------------
 *      RL-ARM - TCPnet
 *----------------------------------------------------------------------------
 *      Name:    TELNET_UIF.C
 *      Purpose: Telnet Server User Interface Module
 *      Rev.:    V4.22
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2011 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <Net_Config.h>
#include <string.h>
#include <stdio.h>

/* Net_Config.c */
#define tcp_NumSocks    tcp_config.NumSocks
#define tcp_socket      tcp_config.Scb
#define tnet_EnAuth     tnet_config.EnAuth
#define tnet_auth_passw tnet_config.Passw

/* ANSI ESC Sequences for terminal control. */
#define CLS     "\033[2J"
#define TBLUE   "\033[37;44m"
#define TNORM   "\033[0m"

extern BOOL LEDrun;
extern BOOL send_msg;

/* My structure of a Telnet U32 storage variable. This variable is private */
/* for each Telnet Session and is not altered by Telnet Server. It is only */
/* set to zero when tnet_process_cmd() is called for the first time.       */
typedef struct {
  U8 id;
  U8 nmax;
  U8 idx;
} MY_BUF;
#define MYBUF(p)        ((MY_BUF *)p)

/* Local variables */
static U8 const tnet_header[] = {
  CLS "\r\n"
  "        " TBLUE
  "*===JieBao_PB325_V0067===*\r\n" TNORM
  };

static U8 const tcp_stat[] = {
  CLS "\r\n"
  "     " TBLUE
  "=============================================================\r\n" TNORM
  "     " TBLUE
  " Socket   State       Rem_IP       Rem_Port  Loc_Port  Timer \r\n" TNORM
  "     " TBLUE
  "=============================================================\r\n" TNORM
  };

static char const state[][11] = {
  "FREE",
  "CLOSED",
  "LISTEN",
  "SYN_REC",
  "SYN_SENT",
  "FINW1",
  "FINW2",
  "CLOSING",
  "LAST_ACK",
  "TWAIT",
  "CONNECT"};

static U32 unsol_msg;


/*----------------------------------------------------------------------------
 *      Functions
 *---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 *      Telnet CallBack Functions
 *---------------------------------------------------------------------------*/

/*--------------------------- tnet_cbfunc -----------------------------------*/

U16 tnet_cbfunc (U8 code, U8 *buf, U16 buflen) {
  /* This function is called by the Telnet Client to get formated system    */
  /* messages for different code values.                                    */
  /* Values for 'code':                                                     */
  /*    0 - initial header                                                  */
  /*    1 - prompt string                                                   */
  /*    2 - header for login only if authorization is enabled               */
  /*    3 - string 'Username' for login                                     */
  /*    4 - string 'Password' for login                                     */
  /*    5 - message 'Login incorrect'                                       */
  /*    6 - message 'Login timeout'                                         */
  /*    7 - Unsolicited messages from Server (ie. Basic Interpreter)        */
  U16 len = 0;

  /* Make a reference to disable compiler warning. */
  buflen = buflen;

  switch (code) {
    case 0:
      /* Write initial header after login. */
      len = str_copy (buf, (U8 *)&tnet_header);
      break;
    case 1:
      /* Write a prompt string. */
      len = str_copy (buf, "\r\n> ");
      break;
    case 2:
      /* Write Login header. */
      len = str_copy (buf, CLS "\r\nplease login...\r\n");
      break;
    case 3:
      /* Write 'username' prompt. */
      len = str_copy (buf, "\r\nUsername: ");
      break;
    case 4:
      /* Write 'Password' prompt. */
      len = str_copy (buf, "\r\nPassword: ");
      break;
    case 5:
      /* Write 'Login incorrect'.message. */
      len = str_copy (buf, "\r\nLogin incorrect");
      break;
    case 6:
      /* Write 'Login Timeout' message. */
      len = str_copy (buf, "\r\nLogin timed out after 60 seconds.\r\n");
      break;
    case 7:
      /* Write Unsolicited messages from the Application Layer above. */
      len = sprintf ((char *)buf, "\r\nUnsolicited message nr. %d\r\n", unsol_msg++);
      break;
  }
  return (len);
}

/*--------------------------- tnet_process_cmd ------------------------------*/

U16 tnet_process_cmd (U8 *cmd, U8 *buf, U16 buflen, U32 *pvar) {
  /* This is a Telnet Client callback function to make a formatted output   */
  /* for 'stdout'. It returns the number of bytes written to the out buffer.*/
  /* Hi-bit of return value (len is or-ed with 0x8000) is a disconnect flag.*/
  /* Bit 14 (len is or-ed with 0x4000) is a repeat flag for the Tnet client.*/
  /* If this bit is set to 1, the system will call the 'tnet_process_cmd()' */
  /* again with parameter 'pvar' pointing to a 4-byte buffer. This buffer   */
  /* can be used for storing different status variables for this function.  */
  /* It is set to 0 by Telnet server on first call and is not altered by    */
  /* Telnet server for repeated calls. This function should NEVER write     */
  /* more than 'buflen' bytes to the buffer.                                */
  /* Parameters:                                                            */
  /*   cmd    - telnet received command string                              */
  /*   buf    - Telnet transmit buffer                                      */
  /*   buflen - length of this buffer (500-1400 bytes - depends on MSS)     */
  /*   pvar   - pointer to local storage buffer used for repeated loops     */
  /*            This is a U32 variable - size is 4 bytes. Value is:         */
  /*            - on 1st call = 0                                           */
  /*            - 2nd call    = as set by this function on first call       */
  TCP_INFO *tsoc;
  U16 len = 0;

  switch (MYBUF(pvar)->id) {
    case 0:
      /* First call to this function, the value of '*pvar' is 0 */
      break;

    case 1:
      /* Repeated call, TCP status display. */
      while (len < buflen-80) {
        /* Let's use as much of the buffer as possible. */
        /* This will produce less packets and speedup the transfer. */
        if (MYBUF(pvar)->idx == 0) {
          len += str_copy (buf, (U8 *)tcp_stat);
        }
        tsoc = &tcp_socket[MYBUF(pvar)->idx];
        len += sprintf   ((char *)(buf+len), "\r\n%9d %10s  ", MYBUF(pvar)->idx, 
                          state[tsoc->State]);
        if (tsoc->State <= TCP_STATE_CLOSED) {
          len += sprintf ((char *)(buf+len),
                          "        -             -         -       -\r\n");
        }
        else if (tsoc->State == TCP_STATE_LISTEN) {
          len += sprintf ((char *)(buf+len),
                          "        -             -     %5d       -\r\n",
                          tsoc->LocPort);
        }
        else {
          /* First temporary print for alignment. */
          sprintf ((char *)(buf+len+16),"%d.%d.%d.%d",tsoc->RemIpAdr[0],
                tsoc->RemIpAdr[1],tsoc->RemIpAdr[2],tsoc->RemIpAdr[3]);
          len += sprintf ((char *)(buf+len),"%15s    %5d    %5d     %4d\r\n",
                          buf+len+16,tsoc->RemPort,tsoc->LocPort,tsoc->AliveTimer);
        }
        if (++MYBUF(pvar)->idx >= tcp_NumSocks) {
          /* OK, we are done, reset the index counter for next callback. */
          MYBUF(pvar)->idx = 0;
          /* Setup a callback delay. This function will be called again after    */
          /* delay has expired. It is set to 20 system ticks 20 * 100ms = 2 sec. */
          tnet_set_delay (500);
          break;
        }
      }
      /* Request a repeated call, bit 14 is a repeat flag. */
      return (len |= 0x4000);
  }

  /* Simple Command line parser */
  len = strlen ((const char *)cmd);
  
  if (tnet_ccmp (cmd, "RESET") == __TRUE) {
    /* 'RESET' command received */
	sys_Reset();
    return (len);
  }

  if (tnet_ccmp (cmd, "SPIF") == __TRUE) {
  	extern int dbg_SpifInfo(uint8_t *pBuf);
    return dbg_SpifInfo(buf);
  }

  if (tnet_ccmp (cmd, "BYE") == __TRUE) {
    /* 'BYE' command, send message and disconnect */
    len = str_copy (buf, "\r\nDisconnect...\r\n");
    /* Hi bit of return value is a disconnect flag */
    return (len | 0x8000);
  }

  if (tnet_ccmp (cmd, "TCPSTAT") == __TRUE) {
    /* Display a TCP status similar to that in HTTP_Demo example. */
    /* Here the local storage '*pvar' is initialized to 0 by Telnet Server.    */
    MYBUF(pvar)->id = 1;
    len = str_copy (buf, CLS);
    return (len | 0x4000);
  }

  /* Unknown command, display message */
  len = str_copy  (buf, "\r\n==> Unknown Command: ");
  len += str_copy (buf+len, cmd);
  return (len);
}


/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
