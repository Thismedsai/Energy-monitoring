/********************************************************************************************************/
/* 名称：wrap_httpServ.h                                                                                */
/* 内容：Wrapper HTTP Server                                                                            */
/*                                                                                                      */
/*  [コンパイラ]                                                                                        */
/*   MCU                      : TBD			                                                            */
/*   Assembler Ver.           : TBD                                                                     */
/*   Compiler Ver.            : TBD                                                                     */
/*   Type of a Car            : TBD                                                                     */
/*   Product                  : TBD                                                                     */
/*                                                                                                      */
/*  [変更履歴]				                                                                            */
/*    ・Newly created.                                                                                  */
/********************************************************************************************************/
#ifndef D_WRAP_HTTPSERV_H_
#define D_WRAP_HTTPSERV_H_
/********************************************************************************************************/
/* Include Header File                                                                                  */
/********************************************************************************************************/

/********************************************************************************************************/
/* Global type Declaration                                                                              */
/********************************************************************************************************/
/* Enumeration for resource type */
typedef enum
{
    en_html = 0u,       /* Content type HTML */
    en_css,             /* Content type CSS */
    en_getApi,          /* Content type GET API */
    en_setApi           /* Content type SET API */
}en_contentType;

/* Structure data for payload */
typedef struct
{
    u1* pu1_dataBuf;    /* Pointer to data buffer */
    u4  u4_dataLen;     /* Data length */
}   st_payloadInfo;

/* Structure data for resource */
typedef struct
{
    char*               pc_url;                                                   /* URL */
    char*               pc_filePath;                                              /* File path */
    void                (*fp_funcCbk)( st_payloadInfo*, st_payloadInfo* );        /* Callback function */
}st_resourceInfo;

/* Structure data for resource group */
typedef struct
{
    st_resourceInfo*        pstt_resourceTbl;                                      /* Pointer to resource table */
    u1                      u1_tblNum;                                             /* Table length */
    en_contentType          pc_contentType;                                        /* Content type */
    void                   (*fp_funcCbk)(char*, st_payloadInfo*);                  /* Callback function */
}st_resourceGroup;

/********************************************************************************************************/
/* Global Macro Declaration                                                                             */
/********************************************************************************************************/


/********************************************************************************************************/
/* Global Variable Declaration                                                                          */
/********************************************************************************************************/

/********************************************************************************************************/
/* Global Function Declaration                                                                          */
/********************************************************************************************************/
// extern en_status eng_wrapHttpServ_init( void );                               /* Wrapper HTTP Server Initialization process */
extern en_status eng_wrapHttpServ_registResource( st_resourceGroup astt_resource[], u1 u1t_resourceNum );
                                                                              /* Wrapper HTTP Server register resource process */
extern en_status eng_wrapHttpServ_startServ( void );                          /* Wrapper HTTP Server start process */
extern en_status eng_wrapHttpServ_stopServ( void );                           /* Wrapper HTTP Server stop process */
extern void vog_wrapHttpServ_writePayload( st_payloadInfo *pst_payload );     /* Wrapper HTTP Server write payload process */

#endif /* D_WRAP_HTTPSERV_H_ */
