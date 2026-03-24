#pragma once

#define HTTP_COMPUTE_SERVICE_Send_NewBar_M1_AndWaitForScanResult "/SendNewBarM1AndWaitForScanResult" // This service will be used to send the new bar to the server and wait for the scan result before sending the next bar -- This is for testing purposes and can be removed later

#define HTTP_COMPUTE_SERVICE_Send_NewBar_M1_NoWaitForScanResult "/SendNewBarM1NoWaitForScanResult" // This service will be used to send the new bar to the server without waiting for the scan result -- This is for testing purposes and can be removed later

#define HTTP_COMPUTE_SERVICE_StartLoadingHistoryData "/StartLoadingHistoryData" // This service will be used to start loading the history data into the server -- This is for testing purposes and can be removed later

#define HTTP_COMPUTE_SERVICE_IsReadyToScan "/IsReadyToScan" // This service will be used to check if the server is ready to scan after receiving the new bar -- This is for testing purposes and can be removed later

#define HTTP_COMPUTE_SERVICE_PostConfigurations "/PostConfigurations" // This service will be used to post the configurations to the server -- This is for testing purposes and can be removed later






#define HTTP_PARAM_HistoryFilePath "HistoryFilePath" // This header will be used to send the history file path to the server when starting to load history data -- This is for testing purposes and can be removed later