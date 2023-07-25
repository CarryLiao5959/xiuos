/* cmd definition */
#define GET_SYSTEM_INFO          1
#define GET_PROCESS_INFO         2

/* process_info definition */
typedef struct {
    int pid;
    int ppid;
    char name[32];
    char state[16];
    char stime[16];
} process_info_t;


/* system_info definition */
typedef struct {
    int   task_total, task_running, task_sleeping, task_stopped, task_zombie;
    float cpu_us, cpu_sy, cpu_ni, cpu_id, cpu_wa, cpu_hi, cpu_si, cpu_st;
    float mem_total, mem_free, mem_used, mem_buff;
    float swap_total, swap_free, swap_used, swap_avail;
} SystemInfo_t;

int rt_client(int argc, char **argv);