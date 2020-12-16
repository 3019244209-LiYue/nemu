#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
	cpu.eip += op_src->val;
	print_asm(str(instr) " %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)
#if DATA_BYTE == 4
make_helper(jmp_rm_l) {
	int len = decode_rm_l(eip + 1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}

extern Sreg_Descriptor *sreg_desc;
Sreg_Descriptor new_sreg_desc;
make_helper(ljmp){
	sreg_desc = &new_sreg_desc;

	uint32_t op1 = instr_fetch(cpu.eip + 1,4);
	uint32_t op2 = instr_fetch(cpu.eip + 1 + 4,2);
	
	cpu.eip = op1 - 7;
	cpu.cs.selector = op2;

	uint16_t idx = cpu.cs.selector >> 3;
	Assert((idx << 3) <= cpu.gdtr.limit,"Segement Selector Is Out Of The Limit!");
	uint32_t chart_addr = cpu.gdtr.base + (idx << 3);
	sreg_desc -> part1 = lnaddr_read(chart_addr,4);
	sreg_desc -> part2 = lnaddr_read(chart_addr + 4,4);
	
	Assert(sreg_desc -> p ==1,"Segement Not Exist!");
	uint32_t bases = 0;
	
	bases |= ((uint32_t)sreg_desc -> base1);
	bases |= ((uint32_t)sreg_desc -> base2)<<16;
	bases |= ((uint32_t)sreg_desc -> base3)<<24;
	cpu.cs.base = bases;

	uint32_t limits = 0;
	limits |= ((uint32_t)sreg_desc -> limit1);
	limits |= ((uint32_t)sreg_desc -> limit2)<<16;
	limits |= ((uint32_t)0xfff)<<24;
	cpu.cs.limit = limits;
	
	if(sreg_desc -> g == 1)
		cpu.cs.limit <<= 12;
	print_asm("ljmp %x %x",op2,op1);
	return 1 + 6;
}
#endif
#include "cpu/exec/template-end.h"
