/********************************************************************************************************/
/* Filename：mid_httpServ.c                                                                             */
/* Content：Middleware HTTP Server                                                                     */
/*                                                                                                      */
/*  [Revision History]                                                                                  */
/*    ・Newly created.                                                                                  */
/********************************************************************************************************/
/********************************************************************************************************/
/* Pragma Declaration                                                                                   */
/********************************************************************************************************/

/********************************************************************************************************/
/* Include Header File                                                                                  */
/********************************************************************************************************/
/* Standard C header file */
#include <string.h>
#include <stdlib.h>

/* Internal header */
#include "../../Common/common.h"
#include "../../Wrap/wrap_httpServ/wrap_httpServ.h"
#include "../mid_wifiCtrl/mid_wifiCtrl.h"
#include "cJSON.h"
#include "../../Mid/mid_nvm/mid_nvm.h"
#include "../../Mid/mid_nvm/mid_nvmType.h"
#include "../../config/config_snsrConfig.h"

/********************************************************************************************************/
/* Macro Declaration                                                                                    */
/********************************************************************************************************/
#define mcU1_MIDHTTPSERV_RES_HTML_NUM        ( (u1)(4u) )    /* Middleware HTTP Server resource HTML number */
#define mcU1_MIDHTTPSERV_RES_CSS_NUM         ( (u1)(1u) )    /* Middleware HTTP Server resource CSS number */
#define mcU1_MIDHTTPSERV_RES_GETAPI_NUM      ( (u1)(4u) )    /* Middleware HTTP Server resource GET API number */
#define mcU1_MIDHTTPSERV_RES_SETAPI_NUM      ( (u1)(5u) )    /* Middleware HTTP Server resource SET API number */
#define mcU1_MIDHTTPSERV_RES_GROUP_NUM       ( (u1)(4u) )    /* Middleware HTTP Server resource group number */

#define mcU1_MIDHTTPSERV_RUN_ST              ( (u1)(0u) )    /* Middleware HTTP Server state running */
#define mcU1_MIDHTTPSERV_STOP_ST             ( (u1)(1u) )    /* Middleware HTTP Server state stopped */

#define mcU1_MIDHTTPSERV_ADDR_MAX_CHAR       mcU1_VAL16      /* Middleware HTTP Server max address character */

/* Page Not found */
#define mcPC_MIDHTTPSERV_HTML_PAGE_NF \
"<!DOCTYPE html>\r\n" \
"<html>\r\n" \
"<head>\r\n" \
"  <meta charset=\"UTF-8\">\r\n" \
"  <title>System Error</title>\r\n" \
"  <script>\r\n" \
"    alert(\"Page Not Found\");\r\n" \
"  </script>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"  <p>Page Not Found</p>\r\n" \
"</body>\r\n" \
"</html>\r\n"

/* Save complete */
#define mcPC_MIDHTTPSERV_HTML_WIFI_CP \
"<!DOCTYPE html>\r\n" \
"<html>\r\n" \
"<head>\r\n" \
"  <meta charset=\"UTF-8\">\r\n" \
"  <title>Redirecting...</title>\r\n" \
"  <script>\r\n" \
"    alert(\"Saving Complete\");\r\n" \
"    window.location.href = \"/wifi_setting.html\";\r\n" \
"  </script>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"  <p>Saving Complete. Redirecting...</p>\r\n" \
"</body>\r\n" \
"</html>\r\n"

#define mcPC_MIDHTTPSERV_HTML_COM_CP \
"<!DOCTYPE html>\r\n" \
"<html>\r\n" \
"<head>\r\n" \
"  <meta charset=\"UTF-8\">\r\n" \
"  <title>Redirecting...</title>\r\n" \
"  <script>\r\n" \
"    alert(\"Saving Complete\");\r\n" \
"    window.location.href = \"/com_setting.html\";\r\n" \
"  </script>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"  <p>Saving Complete. Redirecting...</p>\r\n" \
"</body>\r\n" \
"</html>\r\n"

#define mcPC_MIDHTTPSERV_HTML_CLOUD_CP \
"<!DOCTYPE html>\r\n" \
"<html>\r\n" \
"<head>\r\n" \
"  <meta charset=\"UTF-8\">\r\n" \
"  <title>Redirecting...</title>\r\n" \
"  <script>\r\n" \
"    alert(\"Saving Complete\");\r\n" \
"    window.location.href = \"/cloud_setting.html\";\r\n" \
"  </script>\r\n" \
"</head>\r\n" \
"<body>\r\n" \
"  <p>Saving Complete. Redirecting...</p>\r\n" \
"</body>\r\n" \
"</html>\r\n"

#define mcPC_MIDHTTPSERV_TXT_FILE_NF       "Page Not Found" /* Page/File Not Found */

/********************************************************************************************************/
/* Struct Declaration                                                                                   */
/********************************************************************************************************/

/********************************************************************************************************/
/* Static Variable Declaration                                                                          */
/********************************************************************************************************/

static u1 u1s_midHttpServ_state;    /* Middleware HTTP Server state */

/********************************************************************************************************/
/* Global Variable Declaration                                                                          */
/********************************************************************************************************/

/********************************************************************************************************/
/* Constant Variable Declaration                                                                        */
/********************************************************************************************************/

/* HTML page table */
static const st_resourceInfo casts_webServer_htmlPageTbl[ mcU1_MIDHTTPSERV_RES_HTML_NUM ] = {
    /* URL                         File path                             Handler function */
    {  "/",                        "\\web_page\\dashboard.html",          mc_NULL                          },
    {  "/wifi_setting.html",       "\\web_page\\login.html",              mc_NULL                          },
    {  "/com_setting.html",        "\\web_page\\dashboard.html",          mc_NULL                          },
    {  "/cloud_setting.html",      "\\web_page\\dashboard.html",          mc_NULL                          }
};

/* CSS file table */
static const st_resourceInfo casts_webServer_cssTbl[ mcU1_MIDHTTPSERV_RES_CSS_NUM ] = {
    /* URL                         File path                             Handler function */
    { "/style.css",               "\\css\\style.css",                     mc_NULL                          }
};

/* GET API table */
static const st_resourceInfo casts_webServer_getApiTbl[ mcU1_MIDHTTPSERV_RES_GETAPI_NUM ]= {
    /* URL                        File path                              Callback function */
    { "/getWifiData",             "",                                     vos_midHttpServ_getWifiData      },
    { "/getComData",              "",                                     vos_midHttpServ_getComData       },
    { "/getCloudData",            "",                                     vos_midHttpServ_getCloudData     },
    { "/getDebugData",            "",                                     vos_midHttpServ_getDebugData     }
};

/* SET API table */
static const st_resourceInfo casts_webServer_setApiTbl[ mcU1_MIDHTTPSERV_RES_SETAPI_NUM ] = {
    /* URL                        File path                              Handler function */
    { "/setWifi",                 "",                                     vos_midHttpServ_setWifiData      },
    { "/setCom",                  "",                                     vos_midHttpServ_setComData       },
    { "/setCloud",                "",                                     vos_midHttpServ_setCloudData     },
    { "/setFacReset",             "",                                     vos_midHttpServ_setFacReset      },
    { "/setDebug",                "",                                     vos_midHttpServ_setDebugData     }
};

/* Resource group table */
static const st_resourceGroup casts_webServer_resourceGroupTbl[ mcU1_MIDHTTPSERV_RES_GROUP_NUM ] = {
    /* Resource table                                   Table length                       Content type    Callback Function */
    { (st_resourceInfo*)casts_webServer_htmlPageTbl,    mcU1_MIDHTTPSERV_RES_HTML_NUM,      en_html,        vos_midHttpServ_htmlHandler   },
    { (st_resourceInfo*)casts_webServer_cssTbl,         mcU1_MIDHTTPSERV_RES_CSS_NUM,       en_css,         vos_midHttpServ_cssHandler    },
    { (st_resourceInfo*)casts_webServer_getApiTbl,      mcU1_MIDHTTPSERV_RES_GETAPI_NUM,    en_getApi,      vos_midHttpServ_getApiHandler },
    { (st_resourceInfo*)casts_webServer_setApiTbl,      mcU1_MIDHTTPSERV_RES_SETAPI_NUM,    en_setApi,      vos_midHttpServ_setApiHandler }
};
/********************************************************************************************************/
/* Local function Declaration                                                                           */
/********************************************************************************************************/
static void vos_midHttpServ_runProc( void );                                                                  /* Middleware HTTP Server running process */
static void vos_midHttpServ_stopProc( void );                                                                 /* Middleware HTTP Server stop process */
static void vos_midHttpServ_htmlHandler( char* pc_urlPath, st_payloadInfo *pst_payload );                     /* Middleware HTTP Server HTML page request handler */
static void vos_midHttpServ_cssHandler( char* pc_urlPath, st_payloadInfo *pst_payload );                      /* Middleware HTTP Server CSS file request handler */
static void vos_midHttpServ_getApiHandler( char* pc_urlPath, st_payloadInfo *pst_payload );                   /* Middleware HTTP Server GET API request handler */
static void vos_midHttpServ_setApiHandler( char* pc_urlPath, st_payloadInfo *pst_payload );                   /* Middleware HTTP Server SET API request handler */
static void vos_midHttpServ_getWifiData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );   /* Middleware HTTP Server get WiFi data process */
static void vos_midHttpServ_setWifiData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );   /* Middleware HTTP Server set WiFi data process */
static void vos_midHttpServ_getComData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );    /* Middleware HTTP Server get COM data process */
static void vos_midHttpServ_setComData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );    /* Middleware HTTP Server set COM data process */
static void vos_midHttpServ_getCloudData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );  /* Middleware HTTP Server get Cloud data process */
static void vos_midHttpServ_setCloudData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );  /* Middleware HTTP Server set Cloud data process */
static void vos_midHttpServ_setFacReset( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );   /* Middleware HTTP Server factory reset process */
static void vos_midHttpServ_getDebugData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );  /* Middleware HTTP Server get debug data process */
static void vos_midHttpServ_setDebugData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload );  /* Middleware HTTP Server set debug data process */
static void vos_midHttpServ_urlDecode(char *pct_dst, const u1 *pu1t_src);                                     /* Middleware HTTP Server URL decode process */
static void vos_parseAddrString(const char *pu1t_addrStr, st_ipAddress *pstt_addr);                           /* Middleware HTTP Server parse address string process */

/********************************************************************************************************/
/* Function name : vog_midHttpServ_init		                                  		                    */ 
/* Process name : Middleware HTTP Server initialization process                                		    */
/* Arguments : None                                                                                     */
/* Return : None		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
void vog_midHttpServ_init( void )
{
    en_status ent_retSts;
    u1 u1t_staState;
    u1 u1t_apState;
    
    /* Initialize internal variable */
    u1s_midHttpServ_state = mcU1_MIDHTTPSERV_STOP_ST;

    /* Check WiFI State */
    u1t_staState = u1g_midWifiCtrl_getStaState();
    u1t_apState = u1g_midWifiCtrl_getApState();

    /* Start HTTP Server when the WiFi available */
    if( ( mcU1_MIDWIFICTRL_NORMAL_ST == u1t_staState ) ||
        ( mcU1_MIDWIFICTRL_NORMAL_ST == u1t_apState ) )
    {
        /* Start Wrapper HTTP Server */
        ent_retSts = eng_wrapHttpServ_startServ();

        /* Register resources */
        ( void )eng_wrapHttpServ_registResource( (st_resourceGroup*)casts_webServer_resourceGroupTbl,
                                                mcU1_MIDHTTPSERV_RES_GROUP_NUM );
        if( en_success == ent_retSts )
        {
            /* Set state to running */
            u1s_midHttpServ_state = mcU1_MIDHTTPSERV_RUN_ST;
        }
    }
}

/********************************************************************************************************/
/* Function name : vog_midHttpServ_tick		                                        		            */
/* Process name : Middleware HTTP Server periodic process                                		        */
/* Arguments : None                                                                                     */
/* Return : None		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
void vog_midHttpServ_tick( void )
{
    /* State process */
    switch( u1s_midHttpServ_state )
    {
        /* Running state */
        case mcU1_MIDHTTPSERV_RUN_ST:
            vos_midHttpServ_runProc();
            break;

        /* Stopped state */
        case mcU1_MIDHTTPSERV_STOP_ST:
            vos_midHttpServ_stopProc();
            break;

        /* Default */
        default:
            /* Do nothing */
            break;
    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_runProc		                                  		                */
/* Process name : Middleware HTTP Server running process                                		        */
/* Arguments : None                                                                                     */
/* Return : None		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_runProc( void )
{
    u1 u1t_staState;
    u1 u1t_apState;

    /* Check WiFI State */
    u1t_staState = u1g_midWifiCtrl_getStaState();
    u1t_apState = u1g_midWifiCtrl_getApState();

    if( ( mcU1_MIDWIFICTRL_NORMAL_ST != u1t_staState ) &&
        ( mcU1_MIDWIFICTRL_NORMAL_ST != u1t_apState ) )
    {

        /* Stop Wrapper HTTP Server */
        ( void )eng_wrapHttpServ_stopServ();

        /* Set state to stopped */
        u1s_midHttpServ_state = mcU1_MIDHTTPSERV_STOP_ST;
    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_stopProc		                                  		                */
/* Process name : Middleware HTTP Server stop process                                		            */
/* Arguments : None                                                                                     */
/* Return : None		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_stopProc( void )
{
    u1 u1t_staState;
    u1 u1t_apState;
    en_status ent_result;

    /* Check WiFI State */
    u1t_staState = u1g_midWifiCtrl_getStaState();
    u1t_apState = u1g_midWifiCtrl_getApState();

    if( ( mcU1_MIDWIFICTRL_NORMAL_ST == u1t_staState ) ||
        ( mcU1_MIDWIFICTRL_NORMAL_ST == u1t_apState ) )
    {
        /* Start Wrapper HTTP Server */
        ent_result = eng_wrapHttpServ_startServ();
        if( en_success == ent_result )
        {
            /* Set state to running */
            u1s_midHttpServ_state = mcU1_MIDHTTPSERV_RUN_ST;
        }
    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_htmlHandler		                                    		        */
/* Process name : Middleware HTTP Server HTML page request handler process                            	*/
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             st_payloadInfo *pst_payload            : Pointer to payload information structure        */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_htmlHandler( char* pc_urlPath, st_payloadInfo *pst_payload )
{
    u1 u1t_index;
    u1 u1t_matchIndex;
    st_payloadInfo stt_respPayload;

    u1t_matchIndex = mcU1_MIDHTTPSERV_RES_HTML_NUM;

    /* Guard condition for parameter pointer */
    if( mc_NULL != pc_urlPath )
    {
        /* Loop for all html page in table */
        for( u1t_index = mcU1_VAL0; u1t_index < mcU1_MIDHTTPSERV_RES_HTML_NUM; u1t_index++ )
        {
            /* Check match URL path */
            if( mcS4_VAL0 == strcmp( pc_urlPath, casts_webServer_htmlPageTbl[ u1t_index ].pc_url ) )
            {
                /* Collect the maatch index */
                u1t_matchIndex = u1t_index;
                break;
            }
        }

        if( u1t_matchIndex < mcU1_MIDHTTPSERV_RES_HTML_NUM )
        {
            /* Read file content */
            // File Path: casts_webServer_htmlPageTbl[ u1t_matchIndex ].pc_filePath

            /* Prepare Response Payload */
            stt_respPayload.pu1_dataBuf = ( u1* )"TEST";
            stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );
            
        }
        else
        {
            /* Page not found */
            /* Prepare Response Payload */
            stt_respPayload.pu1_dataBuf = ( u1* )mcPC_MIDHTTPSERV_TXT_FILE_NF;
            stt_respPayload.u4_dataLen = strlen( mcPC_MIDHTTPSERV_TXT_FILE_NF );
        }

        /* Write payload */
        vog_wrapHttpServ_writePayload( &stt_respPayload );

    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_cssHandler		                              		                */
/* Process name : Middleware HTTP Server CSS file request handler process                               */
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             st_payloadInfo *pst_payload            : Pointer to payload information structure        */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_cssHandler( char* pc_urlPath, st_payloadInfo *pst_payload )
{
    u1 u1t_index;
    u1 u1t_matchIndex;
    st_payloadInfo stt_respPayload;

    u1t_matchIndex = mcU1_MIDHTTPSERV_RES_CSS_NUM;

    /* Guard condition for parameter pointer */
    if( mc_NULL != pc_urlPath )
    {
        /* Loop for all html page in table */
        for( u1t_index = mcU1_VAL0; u1t_index < mcU1_MIDHTTPSERV_RES_CSS_NUM; u1t_index++ )
        {
            /* Check match URL path */
            if( mcS4_VAL0 == strcmp( pc_urlPath, casts_webServer_cssTbl[ u1t_index ].pc_url ) )
            {
                /* Collect the maatch index */
                u1t_matchIndex = u1t_index;
                break;
            }
        }

        if( u1t_matchIndex < mcU1_MIDHTTPSERV_RES_CSS_NUM )
        {
            /* Read file content */
            // File Path: casts_webServer_cssTbl[ u1t_matchIndex ].pc_filePath

            /* Prepare Response Payload */
            stt_respPayload.pu1_dataBuf = ( u1* )"TEST";
            stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );
            
        }
        else
        {
            /* Page not found */
            /* Prepare Response Payload */
            stt_respPayload.pu1_dataBuf = ( u1* )mcPC_MIDHTTPSERV_TXT_FILE_NF;
            stt_respPayload.u4_dataLen = strlen( mcPC_MIDHTTPSERV_TXT_FILE_NF );
        }

        /* Write payload */
        vog_wrapHttpServ_writePayload( &stt_respPayload );
    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_getApiHandler		                              		            */
/* Process name : Middleware HTTP Server GET API request handler process                                */
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             st_payloadInfo *pst_payload            : Pointer to payload information structure        */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_getApiHandler( char* pc_urlPath, st_payloadInfo *pst_payload )
{
    u1 u1t_index;
    u1 u1t_matchIndex;
    st_payloadInfo stt_respPayload;

    u1t_matchIndex = mcU1_MIDHTTPSERV_RES_GETAPI_NUM;

    /* Guard condition for parameter pointer */
    if( mc_NULL != pc_urlPath )
    {
        /* Loop for all html page in table */
        for( u1t_index = mcU1_VAL0; u1t_index < mcU1_MIDHTTPSERV_RES_GETAPI_NUM; u1t_index++ )
        {
            /* Check match URL path */
            if( mcS4_VAL0 == strcmp( pc_urlPath, casts_webServer_getApiTbl[ u1t_index ].pc_url ) )
            {
                /* Collect the maatch index */
                u1t_matchIndex = u1t_index;
                break;
            }
        }

        if( u1t_matchIndex < mcU1_MIDHTTPSERV_RES_GETAPI_NUM )
        {
            /* Call function handler for each request */
            casts_webServer_getApiTbl[ u1t_matchIndex ].fp_funcCbk( pst_payload, &stt_respPayload );
        }
        else
        {
            /* Page not found */
            /* Prepare Response Payload */
            stt_respPayload.pu1_dataBuf = ( u1* )mcPC_MIDHTTPSERV_TXT_FILE_NF;
            stt_respPayload.u4_dataLen = strlen( mcPC_MIDHTTPSERV_TXT_FILE_NF );
        }

        /* Write payload */
        vog_wrapHttpServ_writePayload( &stt_respPayload );
    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setApiHandler		                              		            */
/* Process name : Middleware HTTP Server SET API request handler process                                */
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             st_payloadInfo *pst_payload            : Pointer to payload information structure        */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_setApiHandler( char* pc_urlPath, st_payloadInfo *pst_payload )
{
    u1 u1t_index;
    u1 u1t_matchIndex;
    st_payloadInfo stt_respPayload;

    u1t_matchIndex = mcU1_MIDHTTPSERV_RES_GETAPI_NUM;

    /* Guard condition for parameter pointer */
    if( mc_NULL != pc_urlPath )
    {
        /* Loop for all html page in table */
        for( u1t_index = mcU1_VAL0; u1t_index < mcU1_MIDHTTPSERV_RES_SETAPI_NUM; u1t_index++ )
        {
            /* Check match URL path */
            if( mcS4_VAL0 == strcmp( pc_urlPath, casts_webServer_setApiTbl[ u1t_index ].pc_url ) )
            {
                /* Collect the maatch index */
                u1t_matchIndex = u1t_index;
                break;
            }
        }

        if( u1t_matchIndex < mcU1_MIDHTTPSERV_RES_GETAPI_NUM )
        {
            /* Call function handler for each request */
            casts_webServer_setApiTbl[ u1t_matchIndex ].fp_funcCbk( pst_payload, &stt_respPayload );
        }
        else
        {
            /* Page not found */
            /* Prepare Response Payload */
            stt_respPayload.pu1_dataBuf = ( u1* )mcPC_MIDHTTPSERV_TXT_FILE_NF;
            stt_respPayload.u4_dataLen = strlen( mcPC_MIDHTTPSERV_TXT_FILE_NF );
        }

        /* Write payload */
        vog_wrapHttpServ_writePayload( &stt_respPayload );
    }
}

/********************************************************************************************************/
/* Function name : vos_parseAddrString		                                		            */
/* Process name : parsing address string and assign to structure                                                  */
/* Arguments : -                                                                                        */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_parseAddrString( const char *pu1t_addrStr, st_ipAddress *pstt_addr )
{
    if ( NULL != pu1t_addrStr )
    {
        int a, b, c, d;
        /* Parse the address string into four integer components */
        if ( sscanf( pu1t_addrStr, "%d.%d.%d.%d", &a, &b, &c, &d ) == 4 )
        {
            /* Assign parsed values to the address structure */
            pstt_addr->u1_address_1 = ( u1 ) a;
            pstt_addr->u1_address_2 = ( u1 ) b;
            pstt_addr->u1_address_3 = ( u1 ) c;
            pstt_addr->u1_address_4 = ( u1 ) d;
        }
    }
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_getWifiData		                                		            */
/* Process name : Middleware HTTP get WiFi data process                                                 */
/* Arguments : -                                                                                        */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_getWifiData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_payloadInfo stt_respPayload;
    u4 u4t_result;
    st_nvmConfigData stt_nvm_readData;
    
    /* Get WiFi data from EEPROM */
    u4t_result = eng_midNvm_readData( en_wifiDataType, &stt_nvm_readData );
    /* Create JSON root object */
    cJSON *pstt_root = cJSON_CreateObject();

    if ( en_success == u4t_result )
    {
        u1 u1t_ssidBuf[ mcU1_MIDNVM_SSID_LEN ];
        u1 u1t_passwordBuf[ mcU1_MIDNVM_PASSWORD_LEN ];

        /* Copy SSID and password from NVM data */
        ( void )vog_cmn_copyBufferValue( u1t_ssidBuf,
                                        stt_nvm_readData.u_data.st_wifiData.au1_ssid,
                                        mcU1_MIDNVM_SSID_LEN );
        ( void )vog_cmn_copyBufferValue( u1t_passwordBuf,
                                        stt_nvm_readData.u_data.st_wifiData.au1_password,
                                        mcU1_MIDNVM_PASSWORD_LEN );
        
        /* Pack Response Payload */
        cJSON_AddStringToObject( pstt_root, "ssid", u1t_ssidBuf );

        cJSON_AddStringToObject( pstt_root, "password", u1t_passwordBuf );

        char au1t_ipStr[ mcU1_VAL16 ];
        snprintf( au1t_ipStr, sizeof( au1t_ipStr ), "%d.%d.%d.%d",
            stt_nvm_readData.u_data.st_wifiData.st_ipAddress.u1_address_1,
            stt_nvm_readData.u_data.st_wifiData.st_ipAddress.u1_address_2,
            stt_nvm_readData.u_data.st_wifiData.st_ipAddress.u1_address_3,
            stt_nvm_readData.u_data.st_wifiData.st_ipAddress.u1_address_4);
        cJSON_AddStringToObject( pstt_root, "ip address", au1t_ipStr );

        char au1t_subnetStr[ mcU1_VAL16 ];
        snprintf( au1t_subnetStr, sizeof( au1t_subnetStr ), "%d.%d.%d.%d",
            stt_nvm_readData.u_data.st_wifiData.st_subnet.u1_address_1,
            stt_nvm_readData.u_data.st_wifiData.st_subnet.u1_address_2,
            stt_nvm_readData.u_data.st_wifiData.st_subnet.u1_address_3,
            stt_nvm_readData.u_data.st_wifiData.st_subnet.u1_address_4);
        cJSON_AddStringToObject( pstt_root, "subnet", au1t_subnetStr );

        char au1t_gatewayStr[ mcU1_VAL16 ];
        snprintf( au1t_gatewayStr, sizeof( au1t_gatewayStr ), "%d.%d.%d.%d",
            stt_nvm_readData.u_data.st_wifiData.st_gateway.u1_address_1,
            stt_nvm_readData.u_data.st_wifiData.st_gateway.u1_address_2,
            stt_nvm_readData.u_data.st_wifiData.st_gateway.u1_address_3,
            stt_nvm_readData.u_data.st_wifiData.st_gateway.u1_address_4 );
        cJSON_AddStringToObject( pstt_root, "gateway", au1t_gatewayStr );
    }
    else
    {
        /* Error message for get WiFi data error */
        cJSON_AddStringToObject( pstt_root, "status", "Get data from NVM error" );
       
    }
    
    /* Print JSON object to string */
    stt_respPayload.pu1_dataBuf = cJSON_Print( pstt_root );
    stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );

    /* Pass address of data string to parameter for response payload */
    *pst_respPayload = stt_respPayload;
    
    /* Delete JSON object to free memory */
    cJSON_Delete( pstt_root );
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setWifiData		                                		            */
/* Process name : Middleware HTTP set WiFi data process                                                 */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_setWifiData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_nvmConfigData stt_reqPayload;
    st_payloadInfo stt_respPayload;
    u1 u1t_result;

    /* Parse the data in JSON */
    cJSON *pstt_root = cJSON_Parse( pst_reqPayload->pu1_dataBuf );

    /* Assign parsed values to request payload */
    cJSON *pstt_item = cJSON_GetObjectItem( pstt_root, "ssid" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_wifiData.au1_ssid ) ) )
    {
        strcpy( (char*)stt_reqPayload.u_data.st_wifiData.au1_ssid, pstt_item->valuestring );
    }
    
    pstt_item = cJSON_GetObjectItem( pstt_root, "password" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_wifiData.au1_password ) ) )
    {
        strcpy( (char*)stt_reqPayload.u_data.st_wifiData.au1_password, pstt_item->valuestring );
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "ip address" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_wifiData.st_ipAddress ) ) )
    {
        vos_parseAddrString(pstt_item->valuestring, &stt_reqPayload.u_data.st_wifiData.st_ipAddress);
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "subnet" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_wifiData.st_subnet ) ) )
    {
        vos_parseAddrString(pstt_item->valuestring, &stt_reqPayload.u_data.st_wifiData.st_subnet);
    }

   pstt_item = cJSON_GetObjectItem( pstt_root, "gateway" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_wifiData.st_gateway ) ) )
    {
        vos_parseAddrString(pstt_item->valuestring, &stt_reqPayload.u_data.st_wifiData.st_gateway);
    }

    /* Delete JSON object to free memory */
    cJSON_Delete( pstt_root );

    /* Write data to EEPROM */
    u1t_result = eng_midNvm_writeData( en_wifiDataType, &stt_reqPayload );

    if (en_success == u1t_result)
    {
        /* Set write NVM status */
        stt_respPayload.pu1_dataBuf = "{\"result\":\"ok\"}";
        stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );
    }
    else
    {
        /* Set write NVM status */
        stt_respPayload.pu1_dataBuf = "{\"result\":\"fail\"}";
        stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );

    }

    /* Set value of response payload */
    *pst_respPayload = stt_respPayload;
    
}


/********************************************************************************************************/
/* Function name : vos_midHttpServ_getComData		                                		            */
/* Process name : Middleware HTTP get Communication data process                                        */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_getComData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_payloadInfo stt_respPayload;
    u4 u4t_result;
    st_nvmConfigData stt_nvm_readData;
    u1 u1t_slotIdx;
    

    /* Get WiFi data from EEPROM */
    u4t_result = eng_midNvm_readData( en_comDataType, &stt_nvm_readData );
    /* Create JSON root object */
    cJSON *pstt_root = cJSON_CreateObject();

    if ( en_success == u4t_result )
    {
        u1 u1t_snsrModBuf[ mcU1_MIDSNSR_SLOT_NUM ][ mcU1_MIDNVM_MODEL_NAME_LEN + mcU1_VAL1 ];

        /* Pack Response Payload */
        cJSON_AddNumberToObject( pstt_root, "device mode", stt_nvm_readData.u_data.st_comData.u1_deviceMode );
        cJSON_AddNumberToObject( pstt_root, "baudrate", stt_nvm_readData.u_data.st_comData.u4_baudRate );
        cJSON_AddNumberToObject( pstt_root, "parity bit", stt_nvm_readData.u_data.st_comData.u1_parityBit );
        cJSON_AddNumberToObject( pstt_root, "stop bit", stt_nvm_readData.u_data.st_comData.u1_stopBit );
        cJSON_AddNumberToObject( pstt_root, "config flag", stt_nvm_readData.u_data.st_comData.u1_confFlg );
        cJSON *devSlot = cJSON_AddArrayToObject( pstt_root, "channel info" );

        for( u1t_slotIdx = mcU1_VAL0; u1t_slotIdx < mcU1_MIDSNSR_SLOT_NUM; u1t_slotIdx++ )
        {
            cJSON *devSlotData = cJSON_CreateObject();
            vos_midHttpServ_arrayToString( u1t_snsrModBuf[ u1t_slotIdx ], stt_nvm_readData.u_data.st_comData.st_chInfo[ u1t_slotIdx ].au1_sensorModel, mcU1_MIDNVM_MODEL_NAME_LEN );
            cJSON_AddNumberToObject( devSlotData, "chanel enable", stt_nvm_readData.u_data.st_comData.st_chInfo[ u1t_slotIdx ].u1_isChEnable );
            cJSON_AddNumberToObject( devSlotData, "slave ID", stt_nvm_readData.u_data.st_comData.st_chInfo[ u1t_slotIdx ].u1_slaveId );
            cJSON_AddStringToObject( devSlotData, "sensor model", u1t_snsrModBuf[ u1t_slotIdx ] );
            cJSON_AddNumberToObject( devSlotData, "phase", stt_nvm_readData.u_data.st_comData.st_chInfo[ u1t_slotIdx ].u1_phase );  
            cJSON_AddItemToArray( devSlot, devSlotData );
        }
    }
    else
    {
        /* Error message for get WiFi data error */
        cJSON_AddStringToObject( pstt_root, "status", "Get data from NVM error" );
       
    }
    
    /* Print JSON object to string */
    stt_respPayload.pu1_dataBuf = cJSON_Print( pstt_root );
    stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );

    /* Pass address of data string to parameter for response payload */
    pst_respPayload = &stt_respPayload;
    
    /* Delete JSON object to free memory */
    cJSON_Delete( pstt_root );
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setComData		                                		            */
/* Process name : Middleware HTTP set Communication process                                             */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_setComData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_nvmConfigData stt_reqPayload;
    st_payloadInfo stt_respPayload;
    u1 u1t_result;
    u1 u1t_slotIdx;

    cJSON *pstt_root = cJSON_Parse( pst_reqPayload->pu1_dataBuf );
    cJSON *pstt_item = cJSON_GetObjectItem( pstt_root, "device mode" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_comData.u1_deviceMode = pstt_item->valueint;
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "device mode" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_comData.u1_deviceMode = pstt_item->valueint;
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "baudrate" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_comData.u4_baudRate = pstt_item->valueint;
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "parity bit" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_comData.u1_parityBit = pstt_item->valueint;
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "stop bit" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_comData.u1_stopBit = pstt_item->valueint;
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "config flag" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_comData.u1_confFlg = pstt_item->valueint;
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "channel info" );
    for( u1t_slotIdx = mcU1_VAL0; u1t_slotIdx < mcU1_MIDSNSR_SLOT_NUM; u1t_slotIdx++)
    {
        cJSON *pstt_item_slot = cJSON_GetObjectItem( pstt_item, "channel enable" );
        if ( cJSON_IsString( pstt_item_slot ) )
        {
            stt_reqPayload.u_data.st_comData.st_chInfo[ u1t_slotIdx ].u1_isChEnable = pstt_item_slot->valueint;
        }

        pstt_item_slot = cJSON_GetObjectItem( pstt_item, "slave id" );
        if ( cJSON_IsString( pstt_item_slot ) )
        {
            stt_reqPayload.u_data.st_comData.st_chInfo[ u1t_slotIdx ].u1_slaveId = pstt_item_slot->valueint;
        }

        pstt_item_slot = cJSON_GetObjectItem( pstt_item, "sensor model" );
        if ( cJSON_IsString( pstt_item_slot ) 
            && ( strlen( pstt_item_slot->valuestring ) < sizeof( stt_reqPayload.u_data.st_comData.st_chInfo[ u1t_slotIdx ].au1_sensorModel ) ) )
        {
            strcpy( (char*)stt_reqPayload.u_data.st_comData.st_chInfo[ u1t_slotIdx ].au1_sensorModel, pstt_item_slot->valuestring );
        }

        pstt_item_slot = cJSON_GetObjectItem( pstt_item, "phase" );
        if ( cJSON_IsString( pstt_item_slot ) )
        {
            stt_reqPayload.u_data.st_comData.st_chInfo[ u1t_slotIdx ].u1_phase = pstt_item_slot->valueint;
        }
    }
    

    cJSON_Delete( pstt_root );

    /* Write data to EEPROM */
    u1t_result = eng_midNvm_writeData( en_comDataType, &stt_reqPayload );
    
    if (en_success == u1t_result)
    {
        /* Set write NVM status */
        stt_respPayload.pu1_dataBuf = "{\"result\":\"ok\"}";
        stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );
    }
    else
    {
        /* Set write NVM status */
        stt_respPayload.pu1_dataBuf = "{\"result\":\"fail\"}";
        stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );

    }

    /* Set value of response payload */
    *pst_respPayload = stt_respPayload;
    
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_getCloudData		                                		            */
/* Process name : Middleware HTTP get Cloud data process                                                */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_getCloudData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_payloadInfo stt_respPayload;
    u4 u4t_result;
    st_nvmConfigData stt_nvm_readData;
    

    /* Get WiFi data from EEPROM */
    u4t_result = eng_midNvm_readData( en_cloudDataType, &stt_nvm_readData );
    /* Create JSON root object */
    cJSON *pstt_root = cJSON_CreateObject();

    if ( en_success == u4t_result )
    {
        u1 u1t_urlBuf[ mcU1_MIDNVM_SSID_LEN + mcU1_VAL1 ];
        u1 u1t_keyBuf[ mcU1_MIDNVM_PASSWORD_LEN + mcU1_VAL1 ];

        vos_midHttpServ_arrayToString( u1t_urlBuf, stt_nvm_readData.u_data.st_cloudData.au1_apiUrl, mcU1_MIDNVM_API_URL_LEN );
        vos_midHttpServ_arrayToString( u1t_keyBuf, stt_nvm_readData.u_data.st_cloudData.au1_apiKey, mcU1_MIDNVM_API_KEY_LEN );
        
        /* Pack Response Payload */
        cJSON_AddStringToObject( pstt_root, "api url", u1t_urlBuf );
        cJSON_AddStringToObject( pstt_root, "api key", u1t_keyBuf );
        cJSON_AddNumberToObject( pstt_root, "upload interval", stt_nvm_readData.u_data.st_cloudData.u1_uploadInterval );         
    }
    else
    {
        /* Error message for get WiFi data error */
        cJSON_AddStringToObject( pstt_root, "status", "Get data from NVM error" );
    }
    
    /* Print JSON object to string */
    stt_respPayload.pu1_dataBuf = cJSON_Print( pstt_root );
    stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );

    /* Pass address of data string to parameter for response payload */
    pst_respPayload = &stt_respPayload;
    
    /* Delete JSON object to free memory */
    cJSON_Delete( pstt_root );
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setCloudData		                                		            */
/* Process name : Middleware HTTP set Cloud process                                                     */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_setCloudData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_nvmConfigData stt_reqPayload;
    st_payloadInfo stt_respPayload;
    u1 u1t_result;

    cJSON *pstt_root = cJSON_Parse( pst_reqPayload->pu1_dataBuf );
    cJSON *pstt_item = cJSON_GetObjectItem( pstt_root, "url" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_cloudData.au1_apiUrl ) ) )
    {
        strcpy( (char*)stt_reqPayload.u_data.st_cloudData.au1_apiUrl, pstt_item->valuestring );
    }
    
    pstt_item = cJSON_GetObjectItem( pstt_root, "key" );
    if ( cJSON_IsString( pstt_item ) && ( strlen( pstt_item->valuestring ) < sizeof( stt_reqPayload.u_data.st_cloudData.au1_apiKey ) ) )
    {
        strcpy( (char*)stt_reqPayload.u_data.st_cloudData.au1_apiKey, pstt_item->valuestring );
    }

    pstt_item = cJSON_GetObjectItem( pstt_root, "upload interval" );
    if ( cJSON_IsNumber( pstt_item ) )
    {
        stt_reqPayload.u_data.st_cloudData.u1_uploadInterval = pstt_item->valueint;
    }

    cJSON_Delete( pstt_root );

    /* Write data to EEPROM */
    u1t_result = eng_midNvm_writeData( en_wifiDataType, &stt_reqPayload );

    if (en_success == u1t_result)
    {
        /* Set write NVM status */
        stt_respPayload.pu1_dataBuf = "{\"result\":\"ok\"}";
        stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );
    }
    else
    {
        /* Set write NVM status */
        stt_respPayload.pu1_dataBuf = "{\"result\":\"fail\"}";
        stt_respPayload.u4_dataLen = strlen( stt_respPayload.pu1_dataBuf );

    }

    /* Set value of response payload */
    *pst_respPayload = stt_respPayload;
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setFacReset		                                		            */
/* Process name : Middleware HTTP Factory reset process                                                 */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_setFacReset( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    /* Write data to EEPROM */

    /* Pack Response Payload */
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_getDebugData		                                		            */
/* Process name : Middleware HTTP get Debug data process                                                */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_getDebugData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{
    st_payloadInfo stt_respPayload;
    /* Get WiFi data from EEPROM */

    /* Pack Response Payload */
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setDebugData		                                		            */
/* Process name : Middleware HTTP set Debug data process                                                */
/* Arguments : st_payloadInfo *pst_reqPayload : Request Payload                                         */
/*             st_payloadInfo *pst_respPayload : Response Payload                                       */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_setDebugData( st_payloadInfo *pst_reqPayload, st_payloadInfo *pst_respPayload )
{


    /* Pack Response Payload */
}

/********************************************************************************************************/
/* Function name : vos_midHttpServ_setDebugData		                                		            */
/* Process name : Middleware HTTP set Debug data process                                                */
/* Arguments : char* pst_dst: Pointer to to whch decoded data should be copied                          */
/*             const uint8_t* pst_src: Pointer to url encoded data                                      */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_midHttpServ_urlDecode(char *pct_dst, const u1 *pu1t_src)
{
    char ct_currentChar, ct_nextChar;
    /* Ensure that the received character is a valid character. */
    while ((*pu1t_src) && ((*pu1t_src) < mcU1_VALID_CHARACTER_ASCII_VAL))
    {
        /* URL encoding replaces unsafe ASCII characters with a "%" followed 
         * by two hexadecimal digits. Check for character "%" followed by
         * two hexadecimal digits to decode unsafe ASCII characters.
         */
        if ((*pu1t_src == mcU1_MODUS_OPR_ASCII_VAL) && ((ct_currentChar = pu1t_src[ mcU1_VAL1 ]) && (ct_nextChar = pu1t_src[ mcU1_VAL2 ])) && (isxdigit(ct_currentChar) && isxdigit(ct_nextChar)))
        {
            if (ct_currentChar >= mcU1_SMALL_LETTER_A_ASCII_VAL)
            {
                ct_currentChar -= mcU1_SMALL_LETTER_A_ASCII_VAL - mcU1_CAPITAL_LETTER_A_ASCII_VAL;
            }
            if (ct_currentChar >= mcU1_CAPITAL_LETTER_A_ASCII_VAL)
            {
                ct_currentChar -= (mcU1_CAPITAL_LETTER_A_ASCII_VAL - mcU1_LF_OPR_ASCII_VAL);
            }
            else
            {
                ct_currentChar -= mcU1_NUMBER_ZERO_ASCII_VAL;
            }
            if (ct_nextChar >= mcU1_SMALL_LETTER_A_ASCII_VAL)
            {
                ct_nextChar -= mcU1_SMALL_LETTER_A_ASCII_VAL - mcU1_CAPITAL_LETTER_A_ASCII_VAL;
            }
            if (ct_nextChar >= mcU1_CAPITAL_LETTER_A_ASCII_VAL)
            {
                ct_nextChar -= (mcU1_CAPITAL_LETTER_A_ASCII_VAL - mcU1_LF_OPR_ASCII_VAL);
            }
            else
            {
                ct_nextChar -= mcU1_NUMBER_ZERO_ASCII_VAL ;
            }
            *pct_dst++ = mcU1_DLE_OPR_ASCII_VAL * ct_currentChar + ct_nextChar;
            pu1t_src += mcU1_URL_DECODE_ASCII_OFT_VAL;
        }
        /* A space character is URL encoded as "+". Decode space character. */
        else if (*pu1t_src == mcU1_PLUS_OPR_ASCII_VAL)
        {
            *pct_dst++ = mcU1_SPACE_CHARACTER_ASCII_VAL;
            pu1t_src++;
        }
        /* Decode other characters. */
        else
        {
            *pct_dst++ = *pu1t_src++;
        }

    }

    *pct_dst++ = mcU1_NULL_CHARACTER_ASCII_VAL;
}

/********************************************************************************************************/
/* End of File                                                                                          */
/********************************************************************************************************/  