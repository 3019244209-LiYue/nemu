#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_, SUFFIX)) {
	current_sreg = R_DS;
	uint32_t tmp = MEM_R(reg_l(R_ESI));
	current_sreg = R_ES;
	swaddr_write(reg_l(R_EDI),DATA_BYTE,tmp);
	reg_l(R_ESI) += (cpu.eflags.DF ? -DATA_BYTE : DATA_BYTE);
	reg_l(R_EDI) += (cpu.eflags.DF ? -DATA_BYTE : DATA_BYTE);

	print_asm("movs" str(SUFFIX) " %%ds:(%%esi),%%es:(%%edi)");
	return 1;
}

#include "cpu/exec/template-end.h"
