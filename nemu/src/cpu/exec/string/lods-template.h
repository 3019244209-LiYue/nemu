#include "cpu/exec/template-start.h"

#define instr lods

static void do_execute() {
	if(ops_decoded.is_operand_size_16)
	{
		if(DATA_BYTE == 2)
		{
			swaddr_write(reg_w(R_AX),2,swaddr_read(reg_w(R_SI),2));
			if(cpu.DF == 0)
				reg_w(R_SI) += DATA_BYTE;
			else
				reg_w(R_SI) -= DATA_BYTE;
		}
		else
		{
			swaddr_write(reg_b(R_AL),1,swaddr_read(reg_w(R_SI),1));
			if(cpu.DF == 0)
				reg_w(R_SI) += DATA_BYTE;
			else
				reg_w(R_SI) -= DATA_BYTE;
		}
	}
	else
	{
		swaddr_write(reg_l(R_EAX),4,swaddr_read(reg_l(R_ESI),4));
		if(cpu.DF == 0)
			reg_l(R_ESI) += DATA_BYTE;
		else
			reg_l(R_ESI) -= DATA_BYTE;
	}
	print_asm("lods");
}

make_instr_helper(r)

#include "cpu/exec/template-end.h"
