#pragma once

// Server configuration
#define SERVER_HOST "192.168.1.4"  // Change this to match your server's IP address
#define SERVER_PORT 5000
#define SERVER_URL "http://" SERVER_HOST ":" STR(SERVER_PORT)

// Refresh intervals
#define MIN_REFRESH_INTERVAL 20000  // 20 seconds
#define DISPLAY_PAGE_INTERVAL 60000 // 60 seconds
#define TIME_REFRESH_INTERVAL 60000 // 1 minute
#define FULL_REFRESH_INTERVAL 3600000 // 1 hour

// Error handling
#define MAX_DOWNLOAD_RETRIES 3
#define DOWNLOAD_RETRY_DELAY 1000 // 1 second

// Helper macro
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)