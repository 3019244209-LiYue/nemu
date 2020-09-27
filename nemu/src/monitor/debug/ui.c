#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
        char *arg = strtok(NULL," ");
        int step;
        if(arg == NULL)
               /*no argument given */
               step = 1;
        else
               step = atoi(arg);
        
        cpu_exec(step);
        return 0;
}

static int cmd_info(char *args) {
        char *arg = strtok(NULL," ");
        if(arg == NULL)
               printf("Please input 'info r' or 'info w'");
        else if(strcmp(arg,"r") == 0) {
               int i,j,k;
               for(i=0;i<8;i++)
                      printf("%s 0x%x %d\n",regsl[i],reg_l(i),reg_l(i));
               for(j=0;j<8;j++)
                      printf("%s 0x%x %d\n",regsw[j],reg_w(j),reg_w(j));
               for(k=0;k<8;k++)
                      printf("%s 0x%x %d\n",regsb[k],reg_b(k),reg_b(k));
        }
        return 0;
}

static int cmd_x(char *args) {
       char *arg1 = strtok(NULL, " ");
       char *arg2 = strtok(NULL, " ");
       int i,len;
       swaddr_t addr;
       len = atoi(arg1);
       sscanf(arg2,"%x", &addr);

       printf("0x%x:", addr);
       for(i=0;i<len;i++) {
               printf("0x%x ",swaddr_read(addr,4));
               addr+=4;
       }
       printf("\n");
       return 0;
} 
               
static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
        { "si", "Executes N instructions in a single step", cmd_si },
        { "info", "Print the status of the registers and the information of watching points", cmd_info },
        { "x", "Scan the memory", cmd_x },
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
