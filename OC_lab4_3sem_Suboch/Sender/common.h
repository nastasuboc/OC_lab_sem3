#pragma once

const int MSG_LEN = 20;

struct Header {
    int capacity;
    int head;
    int tail;
};

#define MUTEX_NAME      "Lab4_FileMutex"
#define EMPTY_SEM_NAME  "Lab4_EmptySlots" 
#define FULL_SEM_NAME   "Lab4_FullSlots"    
#define READY_EVENT_FMT "Lab4_SenderReady_%d"