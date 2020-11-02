#include "cpu/exec/template-start.h"

#define instr scas

make_helper(concat(scas_, SUFFIX)) {
	uint32_t t2 = MEM_R(reg_l(R_EDI));
	uint32_t t1 = REG(R_EAX);
	uint32_t ret = t1-t2;
	if(cpu.DF == 0)
		reg_l(R_EDI) += DATA_BYTE;
	else
		reg_l(R_EDI) -= DATA_BYTE;
	cpu.ZF = !ret;
	int len = (DATA_BYTE << 3) - 1;
	cpu.SF = ret >> len;
	cpu.CF = (t1<t2);
	int s1=t1 >> len;
	int s2=t2 >> len;
	cpu.OF = (s1!=s2&&s2 == cpu.SF);
	ret ^= ret >> 4;
	ret ^= ret >> 2;
	ret ^= ret >> 1;
	cpu.PF = !(ret&1);
	print_asm("scas");
	return 1;
}

#include "cpu/exec/template-end.h"
