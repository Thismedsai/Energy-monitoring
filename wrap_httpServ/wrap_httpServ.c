/********************************************************************************************************/
/* Filename：mid_wifiCtrl.c                                                                             */
/* Content：Middleware WiFi Control                                                                     */
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

/* HTTP server task header file. */
#include "esp_http_server.h"

/* Standard C header file */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Internal header */
#include "../../Common/common.h"
#include "wrap_httpServ.h"
#include "mid_wifiCtrl.h"



/********************************************************************************************************/
/* Macro Declaration                                                                                    */
/********************************************************************************************************/
#define mcU4_WRAPHTTPSERV_SERVER_IP_ADDRESS     mfU4_MAKE_IPV4_ADDRESS( 0, 0, 0, 0 ) /* Wrapper HTTP Server server IP address */
#define mcU2_WRAPHTTPSERV_SERVER_PORT           ( ( u2 )( 80u ) )                    /* Wrapper HTTP Server server port     */
#define mcU4_WRAPHTTPSERV_SERVER_STACK          ( ( u4 )( 8192u ) )                  /* Wrapper HTTP Server server stack size */
/********************************************************************************************************/
/* Struct Declaration                                                                                   */
/********************************************************************************************************/

/* Structure for keep the callback function of Middleware HTTP Server */
typedef struct
{
    void (*fp_htmlCbk)( char*, st_payloadInfo* );                                    /* HTML Callback function pointer */
    void (*fp_cssCbk)( char*, st_payloadInfo* );                                     /* CSS Callback function pointer */
    void (*fp_getApiCbk)( char*, st_payloadInfo* );                                  /* GET API Callback function pointer */
    void (*fp_setApiCbk)( char*, st_payloadInfo* );                                  /* SET API Callback function pointer */
}st_httpServCbkFunc;

/********************************************************************************************************/
/* Static Variable Declaration                                                                          */
/********************************************************************************************************/
static httpd_handle_t sts_wrapHttpServ_httpServer;                                   /* Wrapper HTTP Server instance */
static httpd_req_t * psts_wrapHttpServ_requestHandle;
static st_httpServCbkFunc sts_wrapHttpServ_cbkFunc;                                  /* Wrapper HTTP Server callback function structure */
/********************************************************************************************************/
/* Global Variable Declaration                                                                          */
/********************************************************************************************************/

/********************************************************************************************************/
/* Constant Variable Declaration                                                                        */
/********************************************************************************************************/

/********************************************************************************************************/
/* Local function Declaration                                                                           */
/********************************************************************************************************/

/* Handle the callback of HTML page request */
static void vos_wrapHttpServ_htmlHandler( httpd_req_t *pst_req );

/* Handle the callback of css file request */
static void vos_wrapHttpServ_cssHandler( httpd_req_t *pst_req );

/* Handle the callback of GET API request */
static void vos_wrapHttpServ_getApiHandler( httpd_req_t *pst_req );

/* Handle the callback of SET API request */
static void vos_wrapHttpServ_setApiHandler( httpd_req_t *pst_req );


/********************************************************************************************************/
/* Function name : eng_wrapHttpServ_startServ		                                  		                */ 
/* Process name : Wrapper HTTP Server Initialization and start process                                	*/
/* Arguments : -	                                                                                    */
/* Return : en_status	                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
en_status eng_wrapHttpServ_startServ( void )
{
    en_status ent_ret;
    esp_err_t u4t_result;
    httpd_config_t stt_config ;

    stt_config = HTTPD_DEFAULT_CONFIG();
    stt_config.stack_size = mcU4_WRAPHTTPSERV_SERVER_STACK;
    stt_config.server_port = mcU2_WRAPHTTPSERV_SERVER_PORT;
    stt_config.max_open_sockets = MAX_SOCKETS;
    stt_config.max_uri_handlers = MAX_URI_HANDLERS;

    sts_wrapHttpServ_httpServer = NULL;

    /* Initialize internal variable */
    sts_wrapHttpServ_cbkFunc.fp_htmlCbk = NULL;
    sts_wrapHttpServ_cbkFunc.fp_cssCbk = NULL;
    sts_wrapHttpServ_cbkFunc.fp_getApiCbk = NULL;
    sts_wrapHttpServ_cbkFunc.fp_setApiCbk = NULL;
    ent_ret = en_fail;

    u4t_result = httpd_start( &sts_wrapHttpServ_httpServer, &stt_config );

    if( ESP_OK  == u4t_result )
    {
        ent_ret = en_success;
    }

    return( ent_ret );
}

/********************************************************************************************************/
/* Function name : eng_wrapHttpServ_registResource		                                  		        */
/* Process name : Wrapper HTTP Server register resource process                                	    	*/
/* Arguments : st_resourceGroup astt_resource[] : Resource information structure array                  */
/*             u1 u1t_resourceNum          : Number of resource information structure                   */
/* Return : en_status		                                                   	  						*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
en_status eng_wrapHttpServ_registResource( st_resourceGroup astt_resource[], u1 u1t_resourceNum )
{
    u1 u1t_resourceIndex;
    u1 u1t_tableIndex;
    en_status ent_retSts;
    esp_err_t u4t_result;
    httpd_uri_t stt_uriHandler;

    ent_retSts = en_success;

    /* Register resources */
    /* Loop for all resource */
    for( u1t_resourceIndex = mcU1_VAL0; u1t_resourceIndex < u1t_resourceNum; u1t_resourceIndex++ )
    {
        stt_uriHandler.user_ctx = (void*)&astt_resource[ u1t_resourceIndex ];
    
        /* Prepare data of mime type and callback function */
        switch( astt_resource[ u1t_resourceIndex ].pc_contentType )
        {
            /* HRML Type */
            case en_html:
                stt_uriHandler.handler = vos_wrapHttpServ_htmlHandler;
                stt_uriHandler.method = HTTP_GET;
                sts_wrapHttpServ_cbkFunc.fp_htmlCbk = astt_resource[ u1t_resourceIndex ].fp_funcCbk;
                break;
            /* CSS Type */
            case en_css:
                stt_uriHandler.handler = vos_wrapHttpServ_cssHandler;
                stt_uriHandler.method = HTTP_GET;
                sts_wrapHttpServ_cbkFunc.fp_cssCbk = astt_resource[ u1t_resourceIndex ].fp_funcCbk;
                break;
            /* GET API Type */
            case en_getApi:
                stt_uriHandler.handler = vos_wrapHttpServ_getApiHandler;
                stt_uriHandler.method = HTTP_GET;
                sts_wrapHttpServ_cbkFunc.fp_getApiCbk = astt_resource[ u1t_resourceIndex ].fp_funcCbk;
                break;
            /* SET API Type */
            case en_setApi:
                stt_uriHandler.handler = vos_wrapHttpServ_setApiHandler;
                stt_uriHandler.method = HTTP_POST;
                sts_wrapHttpServ_cbkFunc.fp_setApiCbk = astt_resource[ u1t_resourceIndex ].fp_funcCbk;
                break;
            default:
                /* Do nothing */
                break;
        }

        for( u1t_tableIndex = mcU1_VAL0; u1t_tableIndex < astt_resource[ u1t_resourceIndex ].u1t_tableNum; u1t_tableIndex++ )
        {
            stt_uriHandler.uri = astt_resource[ u1t_resourceIndex ].pstt_resourceTbl[ u1t_tableIndex ].pc_url;
            u4t_result = httpd_register_uri_handler( sts_wrapHttpServ_httpServer, &stt_uriHandler );
            /* Check the result of resource registration */
            if( ESP_OK != u4t_result )
            {
                ent_retSts = en_fail;
                break;
            }
        }
    }

    return( ent_retSts );
}

/********************************************************************************************************/
/* Function name : eng_wrapHttpServ_stopServ		                                                    */
/* Process name : Wrapper HTTP Server stop process                                           		    */
/* Arguments : void	                                                          							*/
/* Return : en_status		                                                   	  						*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
en_status eng_wrapHttpServ_stopServ( void )
{
    esp_err_t u4t_result;
    en_status ent_retSts = en_fail;

    if ( NULL != sts_wrapHttpServ_httpServer ) 
    {
        u4t_result = httpd_stop( sts_wrapHttpServ_httpServer );
        if ( ESP_OK != u4t_result )
        {
            ent_retSts = en_fail;
        }
        else
        {
            ent_retSts = en_success;
        }
    }
    return ent_retSts; // Already stopped
}

/********************************************************************************************************/
/* Function name : vog_wrapHttpServ_writePayload		                                  		        */
/* Process name : Wrapper HTTP Server write payload process                                		        */
/* Arguments : st_payloadInfo *pst_payload : Pointer to payload information structure                   */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
void vog_wrapHttpServ_writePayload( st_payloadInfo *pst_payload )
{
    /* Check NULL pointer */
    if ( NULL != psts_wrapHttpServ_requestHandle && ( NULL != pst_payload->pu1_dataBuf ) && ( pst_payload->u4_dataLen > mcU4_VAL0 ) )
    {
         /* Set Status Code (Assuming EN_OK means HTTP 200) */ 
        // httpd_resp_set_status(psts_wrapHttpServ_requestHandle, HTTPD_200); 
        
        /* Write payload to HTTP server */
        httpd_resp_send(psts_wrapHttpServ_requestHandle, (const char*)pst_payload->pu1_dataBuf, pst_payload->u4_dataLen);
    }
   
}

/********************************************************************************************************/
/* Function name : vos_wrapHttpServ_htmlHandler		                                    		        */
/* Process name : Wrapper HTTP Server HTML page request handler process                            	    */
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             const char *pct_urlParam              : URL parameter                                    */
/*             cy_http_response_stream_t *pstt_stream : Pointer to response stream                      */
/*             void *pvt_arg                          : Pointer to argument                             */
/*             cy_http_message_body_t *pstt_httpMessageBody : Pointer to HTTP message body              */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_wrapHttpServ_htmlHandler( httpd_req_t *pst_req )
{
    /* Assign stream */ 
    psts_wrapHttpServ_requestHandle = pst_req;
    httpd_resp_set_type( pst_req, "text/html" );

    /* Call Middleware callback (pc_urlParam is NULL for now) */
    if ( NULL != sts_wrapHttpServ_cbkFunc.fp_htmlCbk )
    {
        sts_wrapHttpServ_cbkFunc.fp_htmlCbk( ( char* )pst_req->uri, NULL );
    }

    /* Clear stream as the request is now complete */
    psts_wrapHttpServ_requestHandle = NULL;
}

/********************************************************************************************************/
/* Function name : vos_wrapHttpServ_cssHandler		                                    		        */
/* Process name : Wrapper HTTP Server css file request handler process                            	    */
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             const char *pct_urlParam              : URL parameter                                    */
/*             cy_http_response_stream_t *pstt_stream : Pointer to response stream                      */
/*             void *pvt_arg                          : Pointer to argument                             */
/*             cy_http_message_body_t *pstt_httpMessageBody : Pointer to HTTP message body              */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_wrapHttpServ_cssHandler( httpd_req_t *pst_req )
{
    /* Assign stream */ 
    psts_wrapHttpServ_requestHandle = pst_req;
    httpd_resp_set_type( pst_req, "text/css" );
    /* Call Middleware callback (pc_urlParam is NULL for now) */
    if ( NULL != sts_wrapHttpServ_cbkFunc.fp_cssCbk )
    {
        sts_wrapHttpServ_cbkFunc.fp_cssCbk( ( char* )pst_req->uri, NULL );
    }

    /* Clear stream as the request is now complete */
    psts_wrapHttpServ_requestHandle = NULL;
}

/********************************************************************************************************/
/* Function name : vos_wrapHttpServ_getApiHandler		                                    		    */
/* Process name : Wrapper HTTP Server GET API request handler process                               	*/
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             const char *pct_urlParam              : URL parameter                                    */
/*             cy_http_response_stream_t *pstt_stream : Pointer to response stream                      */
/*             void *pvt_arg                          : Pointer to argument                             */
/*             cy_http_message_body_t *pstt_httpMessageBody : Pointer to HTTP message body              */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_wrapHttpServ_getApiHandler( httpd_req_t *pst_req )
{
    /* Assign stream */ 
    psts_wrapHttpServ_requestHandle = pst_req;
    httpd_resp_set_type( pst_req, "application/json" );

    /* Call Middleware callback (pc_urlParam is NULL for now) */
    if ( NULL != sts_wrapHttpServ_cbkFunc.fp_getApiCbk )
    {
        sts_wrapHttpServ_cbkFunc.fp_getApiCbk( ( char* )pst_req->uri, NULL );
    }

    /* Clear stream as the request is now complete */
    psts_wrapHttpServ_requestHandle = NULL;
}

/********************************************************************************************************/
/* Function name : vos_wrapHttpServ_setApiHandler		                                    		    */
/* Process name : Wrapper HTTP Server SET API request handler process                            	    */
/* Arguments : char* pc_urlPath                        : URL path                                       */
/*             const char *pct_urlParam              : URL parameter                                    */
/*             cy_http_response_stream_t *pstt_stream : Pointer to response stream                      */
/*             void *pvt_arg                          : Pointer to argument                             */
/*             cy_http_message_body_t *pstt_httpMessageBody : Pointer to HTTP message body              */
/* Return : void		                                                   	  							*/
/* Note: -		        						  													 	*/
/********************************************************************************************************/
static void vos_wrapHttpServ_setApiHandler( httpd_req_t *pst_req )
{
    st_payloadInfo stt_payloadInfo = {0};
    char *pu1t_buf = NULL;
    u4 u4t_recv = 0;

    // Assign stream
    psts_wrapHttpServ_requestHandle = pst_req;
    httpd_resp_set_type( pst_req, "application/json" );
    
    // Read POST Body Data
    if ( pst_req->content_len > 0 ) 
    {
        pu1t_buf = ( char* ) malloc( pst_req->content_len + mcU1_VAL1 );
        if ( NULL != pu1t_buf )
        {
            u4t_recv = httpd_req_recv( pst_req, pu1t_buf, pst_req->content_len );
            if (u4t_recv > 0)
            {
                pu1t_buf[ u4t_recv ] = '\0'; // Null-terminate
                stt_payloadInfo.pu1_dataBuf = ( u1* )pu1t_buf;
                stt_payloadInfo.u4_dataLen = ( u4 )u4t_recv;
            }
        }
    }
    
    // Call Middleware SET API callback
    if( NULL != sts_wrapHttpServ_cbkFunc.fp_setApiCbk )
    {
        // Call the generic SET API callback, passing the URL path and payload info
        sts_wrapHttpServ_cbkFunc.fp_setApiCbk( ( char* )pst_req->uri, &stt_payloadInfo );
    }

    // --- Cleanup ---
    if ( pu1t_buf != NULL )
    {
        free( pu1t_buf );
    }
    
    // Clear the static request handle
    psts_wrapHttpServ_requestHandle = NULL;
}

/********************************************************************************************************/
/* End of File                                                                                          */ 
/********************************************************************************************************/  