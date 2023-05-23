/*
 * @Author: 苗金标
 * @Date: 2023-04-11 10:31:45
 * @LastEditors: 苗金标
 * @LastEditTime: 2023-04-11 10:34:28
 * @Description: 
 */

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


static int is_batch_mode = false;

void sdb_set_batch_mode(){
    printf("hello sdb\n");
    is_batch_mode = true;
}

static int cmd_help(char *args);
static int cmd_q(char *args){
    return -1;
}
static struct 
{
    const char *name;
    const char *description;
    int (*handler) (char *);
} cmd_table[] = {
    { "help", "Display information about all supported commands", cmd_help },
    // { "c", "Continue the execution of the program", cmd_c },
    { "q", "Exit NEMU", cmd_q },
    // { "si", "single exe", cmd_si},
    // { "info", "print program status",cmd_info},
    //{ "x", "scan memory",cmd_x},
    // { "p", "expression result",cmd_p},
};

#define NR_CMD ARRLEN(cmd_table)
static int cmd_help(char *args){
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    int i;

    if (arg == NULL) {
    /* no argument given */
        for (i = 0; i < NR_CMD; i ++) {
            printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        }
    }
    else {
        for (i = 0; i < NR_CMD; i ++) {
            if (strcmp(arg, cmd_table[i].name) == 0) {
                printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
                return 0;
            }
        }
        printf("Unknown command '%s'\n", arg);
    }
    return 0;
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

static char* rl_gets(){
    static char *line_read = NULL;
    if(line_read){
        free(line_read);
        line_read = NULL;
    }//文本不需要时释放指针指向的空间，防止内存泄漏

    line_read = readline("(memu) ");
    if(line_read && *line_read){
        add_history(line_read);
    }
    return line_read;
}


void sdb_mainloop(){
    if(is_batch_mode){
        //cmd_c(NULL);
        printf("batch mode\n");
        return;
    }
    for(char *str; (str = rl_gets())!=NULL;){
        char *str_end = str + strlen(str);
        char *cmd = strtok(str," ");
        if(cmd == NULL){
            continue;
        }
        char *args = cmd + strlen(cmd) + 1;
        if(args >= str_end){
            args = NULL;
        }
            int i;
        for(i = 0;i <  NR_CMD; i++){
            if(strcmp(cmd,cmd_table[i].name) == 0){
                if(cmd_table[i].handler(args) < 0){return;}
                break;
            }
        }
        if(i == NR_CMD){
            printf("Unknown command '%s'\n",cmd);
        }
    }
}

void engine_start(){
    printf("hello engine_start\n");
    sdb_mainloop();
    return;
}
