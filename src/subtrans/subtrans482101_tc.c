/****************************************
 *程序名:trans482101.c
 *功  能:体彩购彩交易
 *日  期:2010.07.22
 ****************************************/

#include        <stdio.h>
#include        <stdlib.h>
#include        <memory.h>
#include        <signal.h>
#include        <sys/types.h>
#include        <time.h>
#include        <errno.h>
#include        "trans_macro.h"
#include        "trans_struct.h"
#include        "netbank_mid.h"
#include        "error.h"


int ics_proc_482101_tc(char *send_buff,char *recv_buff)
{
  /*说明：send_buff-上传报文；
          recv_buff-下传报文;
          ics_send_buff-与后台通讯的发送串;
          ics_recv_buff-与后台通讯的接收串;
 */

  int     i;  
  int     len;
  int     ret;
  int     offset;
  int     i_biz_id;

  ICS_DEF_TIA       *pICS_TIA;
  ICS_DEF_TOA       *pICS_TOA;
  ICS_DEF_482101_I_TC  *pICS_482101_I;
  ICS_DEF_482101_N_TC  *pICS_482101_N;
  ICS_DEF_482101_E_TC  *pICS_482101_E;

  char      ics_send_buff[LEN_ICS_PROC_BUF];
  char      ics_recv_buff[LEN_ICS_PROC_BUF];
  char      ics_482101i_buff[215];
  char      ics_482101n_buff[279];
  char      ics_482101e_buff[69];

  char      ics_tia_buff[171];
  char      ics_toa_buff[114];
  char      tmp_val_str[LEN_TMP_VAL_STR];
  char      tmp_val_str2[LEN_TMP_VAL_STR];
  char      tmp_val_str3[LEN_TMP_VAL_STR];
  char      display_str[LEN_ICS_PROC_BUF];
  char      tmpvalue[200];   /*从上传报文中取得的某项值*/

  char			s_CDNO[LEN_CDNO]; /* 卡号 */
  char			s_PSWD[20]; /* 密码 */

  char      sLen[8];
  char      sLeft[14];
  char      sRight[3];
  char      sTranNo[16];
  char      sTranDate[11];
  char      ics_port[6];
  char      sTxnCnl[32];
  char      sErrMsg[64];
  char      sTellerNo[8];
  time_t    cur_time;
  struct tm   *my_tm;

  /*-------------------STEP1:通讯前处理-组成上传串--------------------*/

  /* STEP1-1:清理结构和变量 */

  pICS_482101_I=(ICS_DEF_482101_I_TC *)ics_482101i_buff;
  pICS_482101_N=(ICS_DEF_482101_N_TC *)ics_482101n_buff;
  pICS_482101_E=(ICS_DEF_482101_E_TC *)ics_482101e_buff;
  pICS_TIA=(ICS_DEF_TIA *)ics_tia_buff;
  pICS_TOA=(ICS_DEF_TOA *)ics_toa_buff;

  memset(ics_send_buff,'\0',sizeof(ics_send_buff));
  memset(ics_recv_buff,'\0',sizeof(ics_recv_buff));
  memset(ics_482101i_buff,'\0',sizeof(ics_482101i_buff));
  memset(ics_482101n_buff,'\0',sizeof(ics_482101n_buff));
  memset(ics_482101e_buff,'\0',sizeof(ics_482101e_buff));
  memset(ics_tia_buff,'\0',sizeof(ics_tia_buff));
  memset(ics_toa_buff,'\0',sizeof(ics_toa_buff));

  memset(tmp_val_str,'\0',sizeof(tmp_val_str));
  memset(tmp_val_str2,'\0',sizeof(tmp_val_str2));
  memset(tmp_val_str3,'\0',sizeof(tmp_val_str3));
  memset(display_str,'\0',sizeof(display_str));
  memset(tmpvalue,'\0',sizeof(tmpvalue));
  memset(sTranNo,'\0',sizeof(sTranNo));
	memset(s_CDNO, '\0', sizeof(s_CDNO));
	memset(s_PSWD, '\0', sizeof(s_PSWD));
  memset(sTxnCnl, 0, sizeof(sTxnCnl));
  memset(sTellerNo,'\0',sizeof(sTellerNo));
  memset(sTranDate,'\0',sizeof(sTranDate));

flog( STEP_LEVEL,"--482101 接收[%s]-------------------------------",send_buff);

 /* 注意：填充数据最好按照结构定义先后顺序，以免出现数据覆盖问题 */
  /* STEP1-2:填上传串的固定头 */
  strcpy(pICS_TIA->CCSCod,"TLU6");            /* CICS交易代码 */
  strcpy(pICS_TIA->TTxnCd,"482101");
  strcpy(pICS_TIA->FeCod,"482101");
  strcpy(pICS_TIA->TrmNo,"DVID");

  getValueOfStr(send_buff,"TXNSRC", sTxnCnl); /*交易渠道*/
  strcpy(pICS_TIA->TxnSrc,sTxnCnl);

  time(&cur_time);
  my_tm = localtime(&cur_time);
  sprintf(sTranNo,"%d%d%d%d%d%d11", my_tm->tm_year+1900, my_tm->tm_mon+1, my_tm->tm_mday, my_tm->tm_hour, my_tm->tm_min, my_tm->tm_sec);
  sprintf(sTranDate,"%d-%d-%d",my_tm->tm_year+1900,my_tm->tm_mon+1,my_tm->tm_mday);

  strcpy(pICS_TIA->NodTrc,sTranNo);           /*柜员号*/

  ret = get_config_value(CONFIG_FILE_NAME, "TELLER_NO", sTellerNo);
  if (ret != RETURN_OK)
    return ret;

  strcpy(pICS_TIA->TlrId,sTellerNo);          /*柜员号*/
  strcpy(pICS_TIA->TIATyp,"T");
  strcpy(pICS_TIA->AthLvl,"00");
  strcpy(pICS_TIA->Sup1Id," ");
  strcpy(pICS_TIA->Sup2Id," ");
  strcpy(pICS_TIA->Sup1Pw," ");
  strcpy(pICS_TIA->Sup2Pw," ");
  strcpy(pICS_TIA->Sup1Dv," ");
  strcpy(pICS_TIA->Sup2Dv," ");
  strcpy(pICS_TIA->AthTbl," ");
  strcpy(pICS_TIA->AthLog," ");
  strcpy(pICS_TIA->HLogNo," ");
  strcpy(pICS_TIA->CprInd,"0");
  strcpy(pICS_TIA->EnpInd,"0");
  strcpy(pICS_TIA->NodNo,"441200");
  strcpy(pICS_TIA->OprLvl," ");
  strcpy(pICS_TIA->TrmVer,"v0000001");
  strcpy(pICS_TIA->OutSys," ");
  strcpy(pICS_TIA->Fil," ");

  /* STEP1-3: 填上传串中的固定元素值*/
  /* 格式: strcpy(pICS_482101_I->RsFld1,"P001"); 第三方交易码(查询)*/

  /* STEP1-4: 从上传报文中获得其余值-用getValueOf函数*/
	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"CrdNo",tmpvalue);  /*卡号          */
  trim(tmpvalue);
  strcpy(s_CDNO, tmpvalue);
  strcpy(pICS_482101_I->CrdNo,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"PSWD",tmpvalue);   /*密码          */
  trim(tmpvalue);
  strcpy(s_PSWD,tmpvalue);

  strcpy(pICS_482101_I->PinBlk,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"TxnAmt",tmpvalue); /*交易金额      */
  strcpy(pICS_482101_I->TxnAmt,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"TrmCod",tmpvalue); /*期号          */
  strcpy(pICS_482101_I->TrmCod,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"TikMod",tmpvalue); /*购票方式      */
  strcpy(pICS_482101_I->TikMod,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"LotTyp",tmpvalue); /*彩票类型      */
  strcpy(pICS_482101_I->LotTyp,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"SigDup",tmpvalue); /*单复式区分    */
  strcpy(pICS_482101_I->SigDup,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"NotNum",tmpvalue); /*注数          */
  strcpy(pICS_482101_I->NotNum,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"MulTip",tmpvalue); /*倍数          */
  strcpy(pICS_482101_I->MulTip,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"ExtNum",tmpvalue); /*扩展号码      */
  strcpy(pICS_482101_I->ExtNum,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
        memset(pICS_482101_I->LotNum, ' ', sizeof(pICS_482101_I->LotNum));
  getValueOfStr(send_buff,"LotNum",tmpvalue); /*投注号码      */
  strcpy(pICS_482101_I->LotNum,tmpvalue);

	memset(tmpvalue, 0, sizeof(tmpvalue));
  getValueOfStr(send_buff,"CntTel",tmpvalue); /*手机号码      */
  strcpy(pICS_482101_I->CntTel,tmpvalue);


  /*STEP1-4:把结构中的结束符替换为空格，上传串末尾加结束符.*/
  len=sizeof(ICS_DEF_TIA);
  for(i=0;i<len;i++)
  {
    if(ics_tia_buff[i]==0)
      ics_tia_buff[i]=' ';
  }

  /* 校验密码 */
  /*flog( STEP_LEVEL,"----校验密码----\n");
  ret = ics_proc_928460( "1", s_CDNO, "1", s_PSWD, pICS_TOA->RspCod ) ;
  if ( ret < 0 )
  {
		flog( STEP_LEVEL,"CALL 928460 Fail [%d]",ret);
		sprintf( sErrMsg, "密码校验失败![%d]", ret );
		goto RETURN;
  }
  if( memcmp( pICS_TOA->RspCod, "000000", 6 ) != 0 )
  {
		flog( STEP_LEVEL,"928460 return [%s]", pICS_TOA->RspCod ) ;
		goto RETURN;
  }*/

  /*发往ICS需加8位报文长度*/
  offset=0;
  offset=offset+8;

  len=sizeof(ICS_DEF_TIA);
  for(i=0;i<len;i++)
  {
        if(ics_tia_buff[i]==0)
                ics_tia_buff[i]=' ';
  }
  memcpy(ics_send_buff+offset,ics_tia_buff,len);
  offset=offset+sizeof(ICS_DEF_TIA);

  len=sizeof(ICS_DEF_482101_I_TC);
  for(i=0;i<len;i++)
  {
    if(ics_482101i_buff[i]==0)
      ics_482101i_buff[i]=' ';
  }
  memcpy(ics_send_buff+offset,ics_482101i_buff,len);
  offset=offset+sizeof(ICS_DEF_482101_I_TC);

  /*发往ICS需加8位报文长度在头*/
  memcpy(sLen,'\0',8);
  sprintf(sLen,"%08d",offset-8);
  memcpy(ics_send_buff,sLen,8);

  ics_send_buff[offset] = '\0';  

  /*----------------------STEP2:与中间业务前置机通讯-----------------*/
   /* 与ICS通讯 */
   memset(ics_port,'\0',sizeof(ics_port));

   ret = get_config_value(CONFIG_FILE_NAME, "ICS_PORT_TC", ics_port);

   if (ret != RETURN_OK)
   {
       return -2;
   }

   ret=clientics( ics_send_buff,ics_recv_buff, atoi(ics_port) );

   if(ret != RETURN_OK)
       return -1;

    memcpy(pICS_TOA,ics_recv_buff,sizeof(ICS_DEF_TOA));

RETURN:

  /*--------------------STEP3:通讯后处理:组成回传报文------------------*/

  if(memcmp(pICS_TOA->RspCod,"000000",sizeof(pICS_TOA->RspCod))==0)/*成功*/
  {
    /* STEP3-1处理页面显示要素: 在这里填写的字段，就是在页面上显示的字段 */
    /* 注意，<br>是页面显示的换行符号 */
  
    memcpy(pICS_482101_N,ics_recv_buff+sizeof(ICS_DEF_TOA),sizeof(ICS_DEF_482101_N_TC));
  
    /* 调用setValueOf函数填充 */
      /*格式:setValueOfStr(recv_buff,"display_zone",display_str);*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_TOA->RspCod,sizeof(pICS_TOA->RspCod));
      setValueOfStr(recv_buff,"MGID",tmp_val_str);  /*返回码*/ 

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->TmpDat,sizeof(pICS_482101_N->TmpDat));
      setValueOfStr(recv_buff,"TmpDat",tmp_val_str);/*包体长度*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->ApCode,sizeof(pICS_482101_N->ApCode));
      setValueOfStr(recv_buff,"ApCode",tmp_val_str);/*格式码'SC'*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->OFmtCd,sizeof(pICS_482101_N->OFmtCd));
      setValueOfStr(recv_buff,"OFmtCd",tmp_val_str);/*格式码'D04'*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->TRspCd,sizeof(pICS_482101_N->TRspCd));
      setValueOfStr(recv_buff,"TRspCd",tmp_val_str);/*状态       */

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LogNo,sizeof(pICS_482101_N->LogNo));
      setValueOfStr(recv_buff,"LogNo",tmp_val_str);/*交行流水号 */

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->TLogNo,sizeof(pICS_482101_N->TLogNo));
      setValueOfStr(recv_buff,"TLogNo",tmp_val_str);/*购彩流水号 */

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->NotNum,sizeof(pICS_482101_N->NotNum));
      setValueOfStr(recv_buff,"NotNum",tmp_val_str);/*注数       */

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LotNum1,sizeof(pICS_482101_N->LotNum1));
      setValueOfStr(recv_buff,"LotNum1",tmp_val_str);/*注1*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LotNum2,sizeof(pICS_482101_N->LotNum2));
      setValueOfStr(recv_buff,"LotNum2",tmp_val_str);/*注2*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LotNum3,sizeof(pICS_482101_N->LotNum3));
      setValueOfStr(recv_buff,"LotNum3",tmp_val_str);/*注3*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LotNum4,sizeof(pICS_482101_N->LotNum4));
      setValueOfStr(recv_buff,"LotNum4",tmp_val_str);/*注4*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LotNum5,sizeof(pICS_482101_N->LotNum5));
      setValueOfStr(recv_buff,"LotNum5",tmp_val_str);/*注5*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->LotNum6,sizeof(pICS_482101_N->LotNum6));
      setValueOfStr(recv_buff,"LotNum6",tmp_val_str);/*复试注注*/

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->MulTip,sizeof(pICS_482101_N->MulTip));
      setValueOfStr(recv_buff,"MulTip",tmp_val_str);/*倍数       */ 

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_N->TrmCod,sizeof(pICS_482101_N->TrmCod));
      setValueOfStr(recv_buff,"TrmCod",tmp_val_str);/*期号       */

    /* STEP3-2 处理页面隐含要素: 这里填写的字段，就是在页面作为隐含input的要素 */
    /* 注意，1.这里设置参数的顺序必须和数据库中参数的顺序一致，否则回出错！ 
            2.返回码字段名定义注意与WEB上一致*/ 

  }
  else
  { /*失败*/
      /*获得错误返回包*/
      memcpy(pICS_482101_E,ics_recv_buff+sizeof(ICS_DEF_TOA),sizeof(ICS_DEF_482101_E_TC));
  
      /* 调用setValueOf函数填充 */
      /*格式:setValueOfStr(recv_buff,"display_zone",display_str);*/
      /*memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_E->RspCod,sizeof(pICS_482101_E->RspCod));
      setValueOfStr(recv_buff,"RspCod",tmp_val_str);*//*返回码       */

      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_TOA->RspCod,sizeof(pICS_TOA->RspCod));
      setValueOfStr(recv_buff,"RspCod",tmp_val_str);  /*返回码*/ 


      memset(tmp_val_str,'\0',sizeof(tmp_val_str));
      memcpy(tmp_val_str,pICS_482101_E->RspMsg,sizeof(pICS_482101_E->RspMsg));
      setValueOfStr(recv_buff,"RspMsg",tmp_val_str);/*返回码       */
  }
  flog( STEP_LEVEL,"**482101 返回[%s]******************************",recv_buff);
  return 0;

}
