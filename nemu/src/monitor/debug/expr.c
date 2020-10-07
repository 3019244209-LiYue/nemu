#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ = 255, NOTEQ = 254, NUM = 10, HNUM = 16, OR = 253, AND = 252,
	NOT = 251, REG = 250 

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},					// equal
        {"!=", NOTEQ},                                  // not equal
        {"-", '-'},                                     // minus
        {"\\*", '*'},                                   // multiply
        {"/", '/'},                                     // divide
        {"\\(", '('},                                   
        {"\\)", ')'},                                   
        {"^[1-9][0-9]*|0$", NUM},                       // number
	{"\\b0[xX][0-9a-fA-F]+\\b", HNUM},			// hex number
        {"\\|\\|", OR},                                 // or
        {"&&", AND},                                // and
	{"!", NOT},					// not
	{"\\$[a-dA-D][hlHL]|\\$[eE]?(ax|dx|cx|bx|bp|si|di|sp|AX|DX|CX|BX|BP|SI|DI|SP)", REG}	// register
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;
				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case NOTYPE:
						break;
					case REG:
						tokens[nr_token].type = rules[i].token_type;
						strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
						tokens[nr_token].str[substr_len - 1] = '\0';
						nr_token++;
						break;
                                        case '+':
					case '-':
					case '*':
					case '/':
					case '(':
					case ')':
					case EQ:
					case NOTEQ:
					case AND:
					case OR:
					case NOT:
					case HNUM:
					case NUM:
						tokens[nr_token].type = rules[i].token_type;
						strncpy(tokens[nr_token].str, substr_start, substr_len);
						tokens[nr_token].str[substr_len] = '\0';
						nr_token++;
						break;
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}

bool check_parentheses(int p, int q) {
	int i;
	if(tokens[p].type == '('&&tokens[q].type == ')') {
		int flag = 0;
		for(i=p+1;i<q;i++) {
			if(tokens[i].type == '(')
				flag++;
			else if(tokens[i].type == ')')
				flag--;
			if(flag < 0)
				return false;
		}
		if(flag == 0)
			return true;
	}
	return false;
}

int pre(int x) {
	if(x == '(' || x == ')')
		return 1;
	else if(x == '!')
		return 2;
	else if(x == '*' || x == '/')
		return 3;
	else if(x == '+' || x == '-')
		return 4;
	else if(x == EQ || x == NOTEQ)
		return 5;
	else if(x == AND)
		return 6;
	else if(x == OR)
		return 7;
	return 10;
}

int find_dominant_operator(int p, int q) {
	int i, op = p, flag = 0;
	int pr = 1;
	for(i=p;i<=q;i++) {
		if(tokens[i].type == NUM || tokens[i].type == HNUM || tokens[i].type == REG) 
			continue;
		if(tokens[i].type == '(') {
			flag++;
			i++;
			while(1) {
				if(tokens[i].type == '(')
					flag++;
				if(tokens[i].type == ')')
					flag--;
				i++;
				if(flag == 0)
					break;
			}
			if(i>q)
				break;
		}
		if(pre(tokens[i].type) >= pr) {
			pr = pre(tokens[i].type);
			op = i;
		}
	}
	return op;
}

uint32_t eval(int p, int q) {
	if(p > q) {
		printf("Something wrong\n");
		return 0;
	}
	else if(p == q) {
		uint32_t val=0;
		if(tokens[p].type == NUM)
			sscanf(tokens[p].str, "%d", &val);
		else if(tokens[p].type == HNUM)
			sscanf(tokens[p].str, "%x", &val);
		else if(tokens[p].type == REG) {
			if(strlen(tokens[p].str) == 3) {
				int i;
				for(i=R_EAX;i<=R_EDI;i++)
					if(strcmp(tokens[i].str, regsl[i]) == 0)
						break;
				if(i<=R_EDI)
					val = reg_l(i);
				else {
					if(strcmp(tokens[i].str, "eip") == 0||strcmp(tokens[i].str,"EIP") == 0)
						val = cpu.eip;
					else {
						printf("Invalid register\n");
						assert(0);
					}
				}
			}
			else if(strlen(tokens[p].str) == 2) {
				if(tokens[p].str[1] == 'h'||tokens[p].str[1] == 'H'||tokens[p].str[1] == 'l'||tokens[p].str[1] == 'L') {
					int i;
					for(i=R_AL;i<=R_BH;i++)
						if(strcmp(tokens[i].str, regsb[i]) == 0)
							break;
					if(i<=R_BH)
						val = reg_b(i);
					else {
						printf("Invalid register\n");
						assert(0);
					}
				}
				else if(tokens[p].str[1] == 'x'||tokens[p].str[1] == 'X'||tokens[p].str[1] == 'p'||tokens[p].str[1] == 'P'||tokens[p].str[1] == 'i'||tokens[p].str[1] == 'I') {
					int i;
					for(i=R_AX;i<=R_DI;i++)
						if(strcmp(tokens[i].str, regsw[i]) == 0)
							break;
					if(i<=R_DI)
						val = reg_w(i);
					else {
						printf("Invalid register\n");
						assert(0);
					}
				}
			}
		}
		return val;
	}

	else if(check_parentheses(p,q) == true) {
		//printf("%d %d\n",p,q); 
		return eval(p + 1, q - 1);
	}
	else {
		int op;
		op = find_dominant_operator(p,q);

		if(p == op||tokens[op].type == '!') {
			uint32_t num = eval(p+1,q);
			switch(tokens[p].type) {
				case '!':return !num;
				default: assert(0);
			}
		}
		
		uint32_t val1 = eval(p,op-1);
		uint32_t val2 = eval(op+1,q);

		switch(tokens[op].type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': return val1 / val2;
			case EQ: return val1 == val2;
			case NOTEQ: return val1 != val2;
			case AND: return val1 && val2;
			case OR: return val1 || val2;
			default: assert(0);
		}
	}
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	
	/* TODO: Insert codes to evaluate the expression. */

	else {
		*success = true;
		int val = eval(0,nr_token-1);
		return val;
	}	
	panic("please implement me");
	return 0;
}
