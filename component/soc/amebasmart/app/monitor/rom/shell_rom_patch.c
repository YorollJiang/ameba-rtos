/*
 *  Routines to access hardware
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */
#include "ameba_soc.h"
#include <stdarg.h>
//#include "strproc.h"

volatile UART_LOG_CTL	shell_ctl;
u8						*shell_argv_array[MAX_ARGV];
UART_LOG_BUF			shell_buf;
UART_LOG_BUF			shell_rxbuf;

//=================================================

//======================================================
//<Function>:  shell_array_init
//<Usage   >:  This function is to initialize an array with a given value.
//<Argus    >:  pArrayToInit --> pointer to an array
//                   ArrayLen      --> number of array elements
//                   InitValue     --> initial value for the array
//<Return   >:  void
//<Notes    >:  NA
//======================================================
_LONG_CALL_
void shell_array_init(u8  *pArrayToInit,  u16 ArrayLen,  u8  InitValue)
{
	u16  ItemCount = 0;

	for (ItemCount = 0; ItemCount < ArrayLen; ItemCount++) {
		*(pArrayToInit + ItemCount) = InitValue;
	}
}


//======================================================
//<Function>:  shell_get_argc
//<Usage   >:  This function is to calcute how many parameters in the cmd string
//<Argus    >:  string      --> pointer to an input cmd string
//<Return   >:  number of parameters in the string.(****except the cmd itself****)
//<Notes    >:  NA
//======================================================
_LONG_CALL_
u8 shell_get_argc(const   u8  *string)
{
	u16 argc;
	u8  *pStr;

	argc = 0;
	pStr = (u8 *)string;

	while (*pStr) {
		if (*pStr != ' ') {
			argc++;
			while ((*pStr != ' ') && (*pStr)) {
				pStr++;
			}

			continue;
		}

		pStr++;
	}

	if (argc > MAX_ARGV) {
		argc = MAX_ARGV;
	}

	return argc;
}

//======================================================
//<Function>:  shell_get_argv
//<Usage   >:  This function is to transfer a cmd string to a char array.
//<Argus    >:  string      --> pointer to an input cmd string
//<Return   >:  pointer to a char array
//<Notes    >:  NA
//======================================================
_LONG_CALL_
u8 **shell_get_argv(const   u8  *string)
{
	u8  ArgvCount = 0;
	u8  *pStr;

	shell_array_init((u8 *)shell_argv_array, MAX_ARGV * sizeof(char *), 0);
	pStr = (u8 *)string;

	while (*pStr) {
		shell_argv_array[ArgvCount] = pStr;
		while ((*pStr != ' ') && (*pStr)) {
			pStr++;
		}

		*(pStr++) = '\0';

		while ((*pStr == ' ') && (*pStr)) {
			pStr++;
		}

		ArgvCount++;

		if (ArgvCount == MAX_ARGV) {
			break;
		}
	}

	return (u8 **)&shell_argv_array;
}




//======================================================
//<Function>:  shell_cmd_exec_rom
//<Usage   >:  This function is to parse Uart-Log cmds. If there's a matched
//                      one, it goes to execute that.
//<Argus    >:  pointer to Uart-Log cmd temp buffer
//<Return   >:  void
//<Notes    >:  NA
//======================================================
_LONG_CALL_
static void shell_cmd_exec_rom(PUART_LOG_CTL   pUartLogCtlExe)
{
	u8  CmdCnt = 0;
	u8  argc = 0;
	u8  **argv;
	u32  CmdNum;
	PUART_LOG_BUF   pUartLogBuf = pUartLogCtlExe->pTmpLogBuf;
	PCOMMAND_TABLE pCmdTbl = pUartLogCtlExe->pCmdTbl;

	argc = shell_get_argc((const u8 *) & ((*pUartLogBuf).UARTLogBuf[0]));
	argv = shell_get_argv((const u8 *) & ((*pUartLogBuf).UARTLogBuf[0]));
	CmdNum = pUartLogCtlExe->CmdTblSz;

	if (argc > 0) {
		_strupr(argv[0]);

		for (CmdCnt = 0; CmdCnt < CmdNum; CmdCnt++) {
			if ((_strcmp((const char *)argv[0], (const char *)(pCmdTbl[CmdCnt].cmd))) == 0) {
				pCmdTbl[CmdCnt].func((argc - 1), (argv + 1));
				shell_array_init(argv[0], sizeof(argv[0]), 0);
				break;
			}
		}
	}

	(*pUartLogBuf).BufCount = 0;
	shell_array_init(&(*pUartLogBuf).UARTLogBuf[0], UART_LOG_CMD_BUFLEN, '\0');
}


//======================================================
//<Function>:  shell_show_backspace
//<Usage   >:  To dispaly backspace on the target interface.
//<Argus    >:  BackLen      --> backspace number to show
//                   EchoFlag     --> a control flag to show msg or not.
//                   pfINPUT      --> func pointer to the display function
//<Return   >:  void
//<Notes    >:  NA
//======================================================
_LONG_CALL_
static void shell_show_backspace(u8  BackLen, u8  EchoFlag, void *pfINPUT)
{
	u8  BackCnt;
	ECHOFUNC pfEcho;
	pfEcho = (ECHOFUNC)pfINPUT;

	for (BackCnt = 0; BackCnt < BackLen; BackCnt++) {
		if (EchoFlag) {
			pfEcho((u8 *)"%c", KB_ASCII_BS);
			pfEcho((u8 *)" ");
			pfEcho((u8 *)"%c", KB_ASCII_BS);
		}
	}
}

//======================================================
//<Function>:  shell_cmd_chk
//<Usage   >:  To deal with Uart-Log input characters
//<Argus    >:  RevData         --> a received data.
//                   prvUartLogCtl  --> pointer to UART_LOG_CTL
//                   EchoFlag        --> a control flag to show msg or not.
//<Return   >:  void
//<Notes    >:  NA
//======================================================
_LONG_CALL_
u8 shell_cmd_chk(
	u8  RevData,
	UART_LOG_CTL    *prvUartLogCtl,
	u8  EchoFlag)
{
	u8  RTNSTS = 0;
	u8  SpaceCount = 0;

	//Cast the input function pointer to the actual echo function pointer type-----
	ECHOFUNC pfEcho;
	pfEcho = (ECHOFUNC)(*prvUartLogCtl).pfINPUT;
	//----------------------------------------------------------------

	if (((*(*prvUartLogCtl).pTmpLogBuf).BufCount < (UART_LOG_CMD_BUFLEN - 1)) ||
		(((*(*prvUartLogCtl).pTmpLogBuf).BufCount == (UART_LOG_CMD_BUFLEN - 1)) &&
		 ((RevData == KB_ASCII_LF) || (RevData == '\0') ||
		  (RevData == KB_ASCII_BS) || (RevData == KB_ASCII_BS_7F)))) {

		if (RevData == 0xFF) {  //invlid char, ignore it
			RTNSTS = 1;
		}

		if ((RevData == KB_ASCII_LF) && ((*(*prvUartLogCtl).pTmpLogBuf).BufCount > 0) &&
			((*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[(*(*prvUartLogCtl).pTmpLogBuf).BufCount - 1] == KB_ASCII_CR)) {
			RTNSTS = 2;
			(*prvUartLogCtl).CRSTS = 1;
			(*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[(*(*prvUartLogCtl).pTmpLogBuf).BufCount - 1] = '\0';
			(*(*prvUartLogCtl).pTmpLogBuf).BufCount--;
			if (EchoFlag) {
				pfEcho((u8 *)"\r\n");
			}
		} else if (RevData == '\0') {
			RTNSTS = 2;
			(*prvUartLogCtl).CRSTS = 1;
			(*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[(*(*prvUartLogCtl).pTmpLogBuf).BufCount] = '\0';
			if (EchoFlag) {
				pfEcho((u8 *)"\r\n");
			}

			if ((*(*prvUartLogCtl).pTmpLogBuf).BufCount != 0) {
				//del log history code
			}
		} else if ((RevData == KB_ASCII_BS) || (RevData == KB_ASCII_BS_7F)) {
			RTNSTS = 1;

			if ((*(*prvUartLogCtl).pTmpLogBuf).BufCount > 0) {
				(*(*prvUartLogCtl).pTmpLogBuf).BufCount--;

				shell_show_backspace(1, 1, (void *)pfEcho);
			}
		} else if (RevData == KB_ASCII_TAB) { //the TAB is fixed to be one space(' ')
			RTNSTS = 1;
			for (SpaceCount = 0; SpaceCount < KB_SPACENO_TAB; SpaceCount++) {
				(*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[(*(*prvUartLogCtl).pTmpLogBuf).BufCount] = ' ';
				(*(*prvUartLogCtl).pTmpLogBuf).BufCount++;
			}

			if (EchoFlag) {
				pfEcho((u8 *)" ");
			}

		} else {
			RTNSTS = 1;
			if (EchoFlag) {
				pfEcho((u8 *)"%c", RevData);
			}

			(*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[(*(*prvUartLogCtl).pTmpLogBuf).BufCount] = RevData;
			(*(*prvUartLogCtl).pTmpLogBuf).BufCount++;
		}
	} else if ((*(*prvUartLogCtl).pTmpLogBuf).BufCount == (UART_LOG_CMD_BUFLEN - 1)) {
		RTNSTS = 1;

		pfEcho((u8 *)"\n\r <<<Too long cmd string.>>> \n");
		(*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[(*(*prvUartLogCtl).pTmpLogBuf).BufCount] = '\0';
		pfEcho(&(*(*prvUartLogCtl).pTmpLogBuf).UARTLogBuf[0]);
	}

	return RTNSTS;
}


//======================================================
//<Function>:  shell_uart_irq_rom
//<Usage   >:  To deal with Uart-Log RX IRQ
//<Argus    >:  void
//<Return   >:  void
//<Notes    >:  NA
//======================================================
_LONG_CALL_
u32 shell_uart_irq_rom(void *Data)
{
	/* To avoid gcc warnings */
	(void) Data;

	u16 i = 0;
	u32 rxnum = 0;
	u32 loguart_lsr = LOGUART_GetStatus(LOGUART_DEV);

	/* moves on only for rx data ready intr, in case other intr is enable in linux*/
	if (!(loguart_lsr & LOGUART_BIT_DRDY)) {
		return 0;
	}

	/* init pTmpRxLogBuf for store log data and count */
	shell_rxbuf.BufCount = 0;
	PUART_LOG_BUF pTmpRxLogBuf = &shell_rxbuf;

	//For Test
	bool    PullMode = FALSE;
	u32 IrqEn = LOGUART_GetIMR();

	LOGUART_SetIMR(0);

recv_again:

	rxnum = LOGUART_GetRxCount();

	/* fetch all data in Uart-Log rx fifo before processing each character */
	for (u8 idx = 0; idx < rxnum; idx++) {
		if (pTmpRxLogBuf->BufCount >= UART_LOG_CMD_BUFLEN) {
			break;
		}
		pTmpRxLogBuf->UARTLogBuf[pTmpRxLogBuf->BufCount] = LOGUART_GetChar(PullMode);
		(pTmpRxLogBuf->BufCount)++;
	}

	/* if fetch no data in rx fifo, return */
	if (pTmpRxLogBuf->BufCount == 0) {
		return 0;
	}

	//KB_ESC chk is for cmd history, it's a special case here.
	if (pTmpRxLogBuf->UARTLogBuf[i] == KB_ASCII_ESC) {
		i++;
		//4 Esc detection is only valid in the first stage of boot sequence (few seconds)
		if (shell_ctl.ExecuteEsc != TRUE) {
			shell_ctl.ExecuteEsc = TRUE;
			shell_ctl.EscSTS = 0;
		} else {
			//4 the input commands are valid only when the task is ready to execute commands
			if ((shell_ctl.BootRdy == 1)  || (shell_ctl.shell_task_rdy == 1)) {
				if (shell_ctl.EscSTS == 0) {
					shell_ctl.EscSTS = 1;
				}
			} else {
				shell_ctl.EscSTS = 0;
			}
		}
	} else if (shell_ctl.EscSTS == 1) {
		if (pTmpRxLogBuf->UARTLogBuf[i] != KB_ASCII_LBRKT) {
			shell_ctl.EscSTS = 0;
		} else {
			shell_ctl.EscSTS = 2;
		}
		i++;
	} else {
		if (shell_ctl.EscSTS == 2) {
			shell_ctl.EscSTS = 0;
		} else {
			if (shell_cmd_chk(pTmpRxLogBuf->UARTLogBuf[i++], (UART_LOG_CTL *)&shell_ctl, 1) == 2) {
				//4 check UartLog buffer to prevent from incorrect access
				if (shell_ctl.pTmpLogBuf != NULL) {
					shell_ctl.ExecuteCmd = TRUE;

					if (shell_ctl.shell_task_rdy) {
						shell_ctl.GiveSema();
					}
				} else {
					shell_array_init((u8 *)shell_ctl.pTmpLogBuf->UARTLogBuf, UART_LOG_CMD_BUFLEN, '\0');
				}
			}
		}
	}

	/* recv all data one time */
	if ((pTmpRxLogBuf->BufCount != i) && (pTmpRxLogBuf->BufCount != 0)) {
		goto recv_again;
	}

	LOGUART_SetIMR(IrqEn);

	return 0;
}


_LONG_CALL_
void shell_init_rom(u32 TBLSz, void *pTBL)
{
	shell_buf.BufCount = 0;
	shell_array_init(&shell_buf.UARTLogBuf[0], UART_LOG_CMD_BUFLEN, '\0');

	shell_rxbuf.BufCount = 0;
	shell_array_init(&shell_rxbuf.UARTLogBuf[0], UART_LOG_CMD_BUFLEN, '\0');

	shell_ctl.NewIdx = 0;
	shell_ctl.SeeIdx = 0;
	shell_ctl.RevdNo = 0;
	shell_ctl.EscSTS = 0;
	shell_ctl.BootRdy = 0;
	shell_ctl.pTmpLogBuf = &shell_buf;
	shell_ctl.pfINPUT = (void *)&DiagPrintf;

	if (TBLSz != 0) {
		shell_ctl.pCmdTbl = (PCOMMAND_TABLE) pTBL;
		shell_ctl.CmdTblSz = TBLSz;
	} else {
		void *PTable = NULL;
		u32 TblSize = cmd_rom_table(&PTable);

		shell_ctl.pCmdTbl = (PCOMMAND_TABLE) PTable;
		shell_ctl.CmdTblSz = TblSize;
	}
	shell_ctl.shell_task_rdy = 0;

	//executing boot sequence
	shell_ctl.ExecuteCmd = FALSE;
	shell_ctl.ExecuteEsc = FALSE;

	//CONSOLE_AMEBA();
}

_LONG_CALL_
void shell_task_rom(void *Data)
{
	/* To avoid gcc warnings */
	(void) Data;

	//4 Set this for UartLog check cmd history
#ifndef CONFIG_KERNEL
	shell_ctl.BootRdy = 1;
#endif
	do {

		if ((shell_ctl.ExecuteCmd) == TRUE) {
			shell_cmd_exec_rom((PUART_LOG_CTL)&shell_ctl);
			CONSOLE_AMEBA();
			shell_ctl.ExecuteCmd = FALSE;
		}
	} while (1);
}

_LONG_CALL_
static bool shell_exit(u32 MaxWaitCount)
{
	u32 WaitCount = 0;

	do {
		if (WaitCount > MaxWaitCount) {
			return TRUE;// go back to the normal boot sequence
		}

		DelayUs(100);

		//        DBG_8195A("Wait %d\n",WaitCount);
		WaitCount++;

		//4 there is a ESC key input in Boot Sequence check stage
		if (shell_ctl.ExecuteEsc == TRUE) {
			CONSOLE_AMEBA();

			shell_ctl.EscSTS = 0;

			break;
		}
	} while (1);

	return FALSE;

}


_LONG_CALL_
void shell_rom(u32 MaxWaitCount)
{

	if (shell_exit(MaxWaitCount)) {
		//Go back to normal boot sequence
		return;
	}

	/* rom cmd consol need this */
	LOG_MASK(LEVEL_ERROR, 0xFFFFFFFF);

	shell_ctl.BootRdy = 1;

	//4 Stay in console stage
	do {
		if ((shell_ctl.ExecuteCmd) == TRUE) {
			shell_cmd_exec_rom((PUART_LOG_CTL)&shell_ctl);
			CONSOLE_AMEBA();
			shell_ctl.ExecuteCmd = FALSE;
		}
	} while (1);
}

