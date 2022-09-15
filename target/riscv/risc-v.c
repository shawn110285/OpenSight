#include "target.h"
#include "risc-v.h"


void riscv_info_init(struct target *target, struct riscv_info *r)
{
	memset(r, 0, sizeof(*r));

	r->common_magic = RISCV_COMMON_MAGIC;

	r->dtm_version = 1;
	r->current_hartid = target->coreid;
	r->version_specific = NULL;

	memset(r->trigger_unique_id, 0xff, sizeof(r->trigger_unique_id));

	r->xlen = -1;

	r->mem_access_methods[0] = RISCV_MEM_ACCESS_PROGBUF;
	r->mem_access_methods[1] = RISCV_MEM_ACCESS_SYSBUS;
	r->mem_access_methods[2] = RISCV_MEM_ACCESS_ABSTRACT;

	r->mem_access_progbuf_warn = true;
	r->mem_access_sysbus_warn = true;
	r->mem_access_abstract_warn = true;

	INIT_LIST_HEAD(&r->expose_csr);
	INIT_LIST_HEAD(&r->expose_custom);
}


static int riscv_create_target(struct target *target, Jim_Interp *interp)
{
	LOG_DEBUG("riscv_create_target()");
	target->arch_info = calloc(1, sizeof(struct riscv_info));
	if (!target->arch_info)
    {
		LOG_ERROR("Failed to allocate RISC-V target structure.");
		return ERROR_FAIL;
	}
	riscv_info_init(target, target->arch_info);
	return ERROR_OK;
}


static int riscv_init_target(struct command_context *cmd_ctx, struct target *target)
{
	LOG_DEBUG("riscv_init_target()");
	RISCV_INFO(info);
	info->cmd_ctx = cmd_ctx;

	select_dtmcontrol.num_bits = target->tap->ir_length;
	select_dbus.num_bits = target->tap->ir_length;
	select_idcode.num_bits = target->tap->ir_length;

	if (bscan_tunnel_ir_width != 0)
    {
		assert(target->tap->ir_length >= 6);
		uint32_t ir_user4_raw = 0x23 << (target->tap->ir_length - 6);
		ir_user4[0] = (uint8_t)ir_user4_raw;
		ir_user4[1] = (uint8_t)(ir_user4_raw >>= 8);
		ir_user4[2] = (uint8_t)(ir_user4_raw >>= 8);
		ir_user4[3] = (uint8_t)(ir_user4_raw >>= 8);
		select_user4.num_bits = target->tap->ir_length;
		bscan_tunneled_ir_width[0] = bscan_tunnel_ir_width;
		if (bscan_tunnel_type == BSCAN_TUNNEL_DATA_REGISTER)
			bscan_tunnel_data_register_select_dmi[1].num_bits = bscan_tunnel_ir_width;
		else /* BSCAN_TUNNEL_NESTED_TAP */
			bscan_tunnel_nested_tap_select_dmi[2].num_bits = bscan_tunnel_ir_width;
	}

	riscv_semihosting_init(target);
	target->debug_reason = DBG_REASON_DBGRQ;
	return ERROR_OK;
}


static void riscv_deinit_target(struct target *target)
{
	LOG_DEBUG("riscv_deinit_target()");

	struct riscv_info *info = target->arch_info;
	struct target_type *tt = get_target_type(target);

	if (tt && info && info->version_specific)
		tt->deinit_target(target);

	riscv_free_registers(target);

	if (!info)
		return;

	range_list_t * entry, * tmp;
	list_for_each_entry_safe(entry, tmp, &info->expose_csr, list)
    {
		free(entry->name);
		free(entry);
	}

	list_for_each_entry_safe(entry, tmp, &info->expose_custom, list)
    {
		free(entry->name);
		free(entry);
	}

	free(info->reg_names);
	free(target->arch_info);

	target->arch_info = NULL;
}


static int riscv_examine(struct target *target)
{
	LOG_DEBUG("riscv_examine()");
	if (target_was_examined(target)) {
		LOG_DEBUG("Target was already examined.");
		return ERROR_OK;
	}

	/* Don't need to select dbus, since the first thing we do is read dtmcontrol. */

	RISCV_INFO(info);
	uint32_t dtmcontrol = dtmcontrol_scan(target, 0);
	LOG_DEBUG("dtmcontrol=0x%x", dtmcontrol);
	info->dtm_version = get_field(dtmcontrol, DTMCONTROL_VERSION);
	LOG_DEBUG("  version=0x%x", info->dtm_version);

	struct target_type *tt = get_target_type(target);
	if (!tt)
		return ERROR_FAIL;

	int result = tt->init_target(info->cmd_ctx, target);
	if (result != ERROR_OK)
		return result;

	return tt->examine(target);
}


static int old_or_new_riscv_poll(struct target *target)
{
	RISCV_INFO(r);
	if (!r->is_halted)
		return oldriscv_poll(target);
	else
		return riscv_openocd_poll(target);
}


int riscv_halt(struct target *target)
{
	RISCV_INFO(r);

	if (!r->is_halted) {
		struct target_type *tt = get_target_type(target);
		return tt->halt(target);
	}

	LOG_DEBUG("[%d] halting all harts", target->coreid);

	int result = ERROR_OK;
	if (target->smp)
    {
		struct target_list *tlist;
		foreach_smp_target(tlist, target->smp_targets)
        {
			struct target *t = tlist->target;
			if (halt_prep(t) != ERROR_OK)
				result = ERROR_FAIL;
		}

		foreach_smp_target(tlist, target->smp_targets)
        {
			struct target *t = tlist->target;
			struct riscv_info *i = riscv_info(t);
			if (i->prepped)
            {
				if (halt_go(t) != ERROR_OK)
					result = ERROR_FAIL;
			}
		}

		foreach_smp_target(tlist, target->smp_targets)
        {
			struct target *t = tlist->target;
			if (halt_finish(t) != ERROR_OK)
				return ERROR_FAIL;
		}

	}
    else
    {
		if (halt_prep(target) != ERROR_OK)
			result = ERROR_FAIL;
		if (halt_go(target) != ERROR_OK)
			result = ERROR_FAIL;
		if (halt_finish(target) != ERROR_OK)
			return ERROR_FAIL;
	}

	return result;
}


static int riscv_target_resume(struct target *target, int current, target_addr_t address, int handle_breakpoints, int debug_execution)
{
	return riscv_resume(target, current, address, handle_breakpoints, debug_execution, false);
}


static int old_or_new_riscv_step(struct target *target, int current, target_addr_t address, int handle_breakpoints)
{
	RISCV_INFO(r);
	LOG_DEBUG("handle_breakpoints=%d", handle_breakpoints);
	if (!r->is_halted)
		return oldriscv_step(target, current, address, handle_breakpoints);
	else
		return riscv_openocd_step(target, current, address, handle_breakpoints);
}


static int riscv_assert_reset(struct target *target)
{
	LOG_DEBUG("[%d]", target->coreid);
	struct target_type *tt = get_target_type(target);
	riscv_invalidate_register_cache(target);
	return tt->assert_reset(target);
}


static int riscv_deassert_reset(struct target *target)
{
	LOG_DEBUG("[%d]", target->coreid);
	struct target_type *tt = get_target_type(target);
	return tt->deassert_reset(target);
}


static int riscv_read_phys_memory(struct target *target, target_addr_t phys_address, uint32_t size, uint32_t count, uint8_t *buffer)
{
	RISCV_INFO(r);
	if (riscv_select_current_hart(target) != ERROR_OK)
		return ERROR_FAIL;
	return r->read_memory(target, phys_address, size, count, buffer, size);
}


static int riscv_read_memory(struct target *target, target_addr_t address, uint32_t size, uint32_t count, uint8_t *buffer)
{
	if (count == 0) {
		LOG_WARNING("0-length read from 0x%" TARGET_PRIxADDR, address);
		return ERROR_OK;
	}

	if (riscv_select_current_hart(target) != ERROR_OK)
		return ERROR_FAIL;

	target_addr_t physical_addr;
	if (target->type->virt2phys(target, address, &physical_addr) == ERROR_OK)
		address = physical_addr;

	RISCV_INFO(r);
	return r->read_memory(target, address, size, count, buffer, size);
}


static int riscv_write_phys_memory(struct target *target, target_addr_t phys_address, uint32_t size, uint32_t count, const uint8_t *buffer)
{
	if (riscv_select_current_hart(target) != ERROR_OK)
		return ERROR_FAIL;
	struct target_type *tt = get_target_type(target);
	return tt->write_memory(target, phys_address, size, count, buffer);
}


static int riscv_write_memory(struct target *target, target_addr_t address, uint32_t size, uint32_t count, const uint8_t *buffer)
{
	if (count == 0)
    {
		LOG_WARNING("0-length write to 0x%" TARGET_PRIxADDR, address);
		return ERROR_OK;
	}

	if (riscv_select_current_hart(target) != ERROR_OK)
		return ERROR_FAIL;

	target_addr_t physical_addr;
	if (target->type->virt2phys(target, address, &physical_addr) == ERROR_OK)
		address = physical_addr;

	struct target_type *tt = get_target_type(target);
	return tt->write_memory(target, address, size, count, buffer);
}


static int riscv_checksum_memory(struct target *target, target_addr_t address, uint32_t count, uint32_t *checksum)
{
	struct working_area *crc_algorithm;
	struct reg_param reg_params[2];
	int retval;

	LOG_DEBUG("address=0x%" TARGET_PRIxADDR "; count=0x%" PRIx32, address, count);

	static const uint8_t riscv32_crc_code[] =
    {
#include "../../../contrib/loaders/checksum/riscv32_crc.inc"
	};

	static const uint8_t riscv64_crc_code[] =
    {
#include "../../../contrib/loaders/checksum/riscv64_crc.inc"
	};

	static const uint8_t *crc_code;

	unsigned xlen = riscv_xlen(target);
	unsigned crc_code_size;
	if (xlen == 32) {
		crc_code = riscv32_crc_code;
		crc_code_size = sizeof(riscv32_crc_code);
	} else {
		crc_code = riscv64_crc_code;
		crc_code_size = sizeof(riscv64_crc_code);
	}

	if (count < crc_code_size * 4) {
		/* Don't use the algorithm for relatively small buffers. It's faster
		 * just to read the memory.  target_checksum_memory() will take care of
		 * that if we fail. */
		return ERROR_FAIL;
	}

	retval = target_alloc_working_area(target, crc_code_size, &crc_algorithm);
	if (retval != ERROR_OK)
		return retval;

	if (crc_algorithm->address + crc_algorithm->size > address &&
			crc_algorithm->address < address + count) {
		/* Region to checksum overlaps with the work area we've been assigned.
		 * Bail. (Would be better to manually checksum what we read there, and
		 * use the algorithm for the rest.) */
		target_free_working_area(target, crc_algorithm);
		return ERROR_FAIL;
	}

	retval = target_write_buffer(target, crc_algorithm->address, crc_code_size,
			crc_code);
	if (retval != ERROR_OK) {
		LOG_ERROR("Failed to write code to " TARGET_ADDR_FMT ": %d",
				crc_algorithm->address, retval);
		target_free_working_area(target, crc_algorithm);
		return retval;
	}

	init_reg_param(&reg_params[0], "a0", xlen, PARAM_IN_OUT);
	init_reg_param(&reg_params[1], "a1", xlen, PARAM_OUT);
	buf_set_u64(reg_params[0].value, 0, xlen, address);
	buf_set_u64(reg_params[1].value, 0, xlen, count);

	/* 20 second timeout/megabyte */
	int timeout = 20000 * (1 + (count / (1024 * 1024)));

	retval = target_run_algorithm(target, 0, NULL, 2, reg_params,
			crc_algorithm->address,
			0,	/* Leave exit point unspecified because we don't know. */
			timeout, NULL);

	if (retval == ERROR_OK)
		*checksum = buf_get_u32(reg_params[0].value, 0, 32);
	else
		LOG_ERROR("error executing RISC-V CRC algorithm");

	destroy_reg_param(&reg_params[0]);
	destroy_reg_param(&reg_params[1]);

	target_free_working_area(target, crc_algorithm);

	LOG_DEBUG("checksum=0x%" PRIx32 ", result=%d", *checksum, retval);

	return retval;
}



static int riscv_mmu(struct target *target, int *enabled)
{
	if (!riscv_enable_virt2phys)
    {
		*enabled = 0;
		return ERROR_OK;
	}

	/* Don't use MMU in explicit or effective M (machine) mode */
	riscv_reg_t priv;
	if (riscv_get_register(target, &priv, GDB_REGNO_PRIV) != ERROR_OK)
    {
		LOG_ERROR("Failed to read priv register.");
		return ERROR_FAIL;
	}

	riscv_reg_t mstatus;
	if (riscv_get_register(target, &mstatus, GDB_REGNO_MSTATUS) != ERROR_OK)
    {
		LOG_ERROR("Failed to read mstatus register.");
		return ERROR_FAIL;
	}

	if ((get_field(mstatus, MSTATUS_MPRV) ? get_field(mstatus, MSTATUS_MPP) : priv) == PRV_M)
    {
		LOG_DEBUG("SATP/MMU ignored in Machine mode (mstatus=0x%" PRIx64 ").", mstatus);
		*enabled = 0;
		return ERROR_OK;
	}

	riscv_reg_t satp;
	if (riscv_get_register(target, &satp, GDB_REGNO_SATP) != ERROR_OK)
    {
		LOG_DEBUG("Couldn't read SATP.");
		/* If we can't read SATP, then there must not be an MMU. */
		*enabled = 0;
		return ERROR_OK;
	}

	if (get_field(satp, RISCV_SATP_MODE(riscv_xlen(target))) == SATP_MODE_OFF)
    {
		LOG_DEBUG("MMU is disabled.");
		*enabled = 0;
	}
    else
    {
		LOG_DEBUG("MMU is enabled.");
		*enabled = 1;
	}

	return ERROR_OK;
}


static int riscv_virt2phys(struct target *target, target_addr_t virtual, target_addr_t *physical)
{
	int enabled;
	if (riscv_mmu(target, &enabled) == ERROR_OK) {
		if (!enabled)
			return ERROR_FAIL;

		if (riscv_address_translate(target, virtual, physical) == ERROR_OK)
			return ERROR_OK;
	}

	return ERROR_FAIL;
}


const char *riscv_get_gdb_arch(struct target *target)
{
	switch (riscv_xlen(target))
    {
		case 32:
			return "riscv:rv32";

		case 64:
			return "riscv:rv64";
	}
	LOG_ERROR("Unsupported xlen: %d", riscv_xlen(target));
	return NULL;
}


static int riscv_get_gdb_reg_list_noread(struct target *target, struct reg **reg_list[], int *reg_list_size, enum target_register_class reg_class)
{
	return riscv_get_gdb_reg_list_internal(target, reg_list, reg_list_size, reg_class, false);
}


static int riscv_get_gdb_reg_list(struct target *target, struct reg **reg_list[], int *reg_list_size, enum target_register_class reg_class)
{
	return riscv_get_gdb_reg_list_internal(target, reg_list, reg_list_size, reg_class, true);
}



int riscv_add_breakpoint(struct target *target, struct breakpoint *breakpoint)
{
	LOG_DEBUG("[%d] @0x%" TARGET_PRIxADDR, target->coreid, breakpoint->address);
	assert(breakpoint);
	if (breakpoint->type == BKPT_SOFT)
    {
		/** @todo check RVC for size/alignment */
		if (!(breakpoint->length == 4 || breakpoint->length == 2))
        {
			LOG_ERROR("Invalid breakpoint length %d", breakpoint->length);
			return ERROR_FAIL;
		}

		if (0 != (breakpoint->address % 2))
        {
			LOG_ERROR("Invalid breakpoint alignment for address 0x%" TARGET_PRIxADDR, breakpoint->address);
			return ERROR_FAIL;
		}

		/* Read the original instruction. */
		if (riscv_read_by_any_size(target, breakpoint->address, breakpoint->length, breakpoint->orig_instr) != ERROR_OK)
        {
			LOG_ERROR("Failed to read original instruction at 0x%" TARGET_PRIxADDR, breakpoint->address);
			return ERROR_FAIL;
		}

		uint8_t buff[4] = { 0 };
		buf_set_u32(buff, 0, breakpoint->length * CHAR_BIT, breakpoint->length == 4 ? ebreak() : ebreak_c());

		/* Write the ebreak instruction. */
		if (riscv_write_by_any_size(target, breakpoint->address, breakpoint->length, buff) != ERROR_OK)
        {
			LOG_ERROR("Failed to write %d-byte breakpoint instruction at 0x%" TARGET_PRIxADDR, breakpoint->length, breakpoint->address);
			return ERROR_FAIL;
		}

	}
    else if (breakpoint->type == BKPT_HARD)
    {
		struct trigger trigger;
		trigger_from_breakpoint(&trigger, breakpoint);
		int const result = add_trigger(target, &trigger);
		if (result != ERROR_OK)
			return result;
	}
    else
    {
		LOG_INFO("OpenOCD only supports hardware and software breakpoints.");
		return ERROR_TARGET_RESOURCE_NOT_AVAILABLE;
	}

	breakpoint->is_set = true;
	return ERROR_OK;
}


int riscv_remove_breakpoint(struct target *target, struct breakpoint *breakpoint)
{
	if (breakpoint->type == BKPT_SOFT)
    {
		/* Write the original instruction. */
		if (riscv_write_by_any_size(target, breakpoint->address, breakpoint->length, breakpoint->orig_instr) != ERROR_OK)
        {
			LOG_ERROR("Failed to restore instruction for %d-byte breakpoint at 0x%" TARGET_PRIxADDR, breakpoint->length, breakpoint->address);
			return ERROR_FAIL;
		}

	}
    else if (breakpoint->type == BKPT_HARD)
    {
		struct trigger trigger;
		trigger_from_breakpoint(&trigger, breakpoint);
		int result = remove_trigger(target, &trigger);
		if (result != ERROR_OK)
			return result;

	}
    else
    {
		LOG_INFO("OpenOCD only supports hardware and software breakpoints.");
		return ERROR_TARGET_RESOURCE_NOT_AVAILABLE;
	}

	breakpoint->is_set = false;

	return ERROR_OK;
}


int riscv_add_watchpoint(struct target *target, struct watchpoint *watchpoint)
{
	struct trigger trigger;
	trigger_from_watchpoint(&trigger, watchpoint);

	int result = add_trigger(target, &trigger);
	if (result != ERROR_OK)
		return result;
	watchpoint->is_set = true;

	return ERROR_OK;
}


int riscv_remove_watchpoint(struct target *target, struct watchpoint *watchpoint)
{
	LOG_DEBUG("[%d] @0x%" TARGET_PRIxADDR, target->coreid, watchpoint->address);

	struct trigger trigger;
	trigger_from_watchpoint(&trigger, watchpoint);

	int result = remove_trigger(target, &trigger);
	if (result != ERROR_OK)
		return result;
	watchpoint->is_set = false;

	return ERROR_OK;
}



/* Sets *hit_watchpoint to the first watchpoint identified as causing the
 * current halt.
 *
 * The GDB server uses this information to tell GDB what data address has
 * been hit, which enables GDB to print the hit variable along with its old
 * and new value. */
int riscv_hit_watchpoint(struct target *target, struct watchpoint **hit_watchpoint)
{
	struct watchpoint *wp = target->watchpoints;

	LOG_DEBUG("Current hartid = %d", riscv_current_hartid(target));

	/*TODO instead of disassembling the instruction that we think caused the
	 * trigger, check the hit bit of each watchpoint first. The hit bit is
	 * simpler and more reliable to check but as it is optional and relatively
	 * new, not all hardware will implement it  */
	riscv_reg_t dpc;
	riscv_get_register(target, &dpc, GDB_REGNO_DPC);
	const uint8_t length = 4;
	LOG_DEBUG("dpc is 0x%" PRIx64, dpc);

	/* fetch the instruction at dpc */
	uint8_t buffer[length];
	if (target_read_buffer(target, dpc, length, buffer) != ERROR_OK)
    {
		LOG_ERROR("Failed to read instruction at dpc 0x%" PRIx64, dpc);
		return ERROR_FAIL;
	}

	uint32_t instruction = 0;

	for (int i = 0; i < length; i++)
    {
		LOG_DEBUG("Next byte is %x", buffer[i]);
		instruction += (buffer[i] << 8 * i);
	}

	LOG_DEBUG("Full instruction is %x", instruction);

	/* find out which memory address is accessed by the instruction at dpc */
	/* opcode is first 7 bits of the instruction */
	uint8_t opcode = instruction & 0x7F;
	uint32_t rs1;
	int16_t imm;
	riscv_reg_t mem_addr;

	if (opcode == MATCH_LB || opcode == MATCH_SB)
    {
		rs1 = (instruction & 0xf8000) >> 15;
		riscv_get_register(target, &mem_addr, rs1);

		if (opcode == MATCH_SB)
        {
			LOG_DEBUG("%x is store instruction", instruction);
			imm = ((instruction & 0xf80) >> 7) | ((instruction & 0xfe000000) >> 20);
		}
        else
        {
			LOG_DEBUG("%x is load instruction", instruction);
			imm = (instruction & 0xfff00000) >> 20;
		}
		/* sign extend 12-bit imm to 16-bits */
		if (imm & (1 << 11))
			imm |= 0xf000;
		mem_addr += imm;
		LOG_DEBUG("memory address=0x%" PRIx64, mem_addr);
	}
    else
    {
		LOG_DEBUG("%x is not a RV32I load or store", instruction);
		return ERROR_FAIL;
	}

	while (wp)
    {
		/*TODO support length/mask */
		if (wp->address == mem_addr)
        {
			*hit_watchpoint = wp;
			LOG_DEBUG("Hit address=%" TARGET_PRIxADDR, wp->address);
			return ERROR_OK;
		}
		wp = wp->next;
	}

	/* No match found - either we hit a watchpoint caused by an instruction that
	 * this function does not yet disassemble, or we hit a breakpoint.
	 *
	 * OpenOCD will behave as if this function had never been implemented i.e.
	 * report the halt to GDB with no address information. */
	return ERROR_FAIL;
}


static int riscv_arch_state(struct target *target)
{
	struct target_type *tt = get_target_type(target);
	return tt->arch_state(target);
}


/* Algorithm must end with a software breakpoint instruction. */
static int riscv_run_algorithm(struct target *target, int num_mem_params, struct mem_param *mem_params, int num_reg_params, struct reg_param *reg_params, target_addr_t entry_point, target_addr_t exit_point, int timeout_ms, void *arch_info)
{
	RISCV_INFO(info);

	if (num_mem_params > 0)
    {
		LOG_ERROR("Memory parameters are not supported for RISC-V algorithms.");
		return ERROR_FAIL;
	}

	if (target->state != TARGET_HALTED)
    {
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* Save registers */
	struct reg *reg_pc = register_get_by_name(target->reg_cache, "pc", true);
	if (!reg_pc || reg_pc->type->get(reg_pc) != ERROR_OK)
		return ERROR_FAIL;

	uint64_t saved_pc = buf_get_u64(reg_pc->value, 0, reg_pc->size);
	LOG_DEBUG("saved_pc=0x%" PRIx64, saved_pc);

	uint64_t saved_regs[32];
	for (int i = 0; i < num_reg_params; i++)
    {
		LOG_DEBUG("save %s", reg_params[i].reg_name);
		struct reg *r = register_get_by_name(target->reg_cache, reg_params[i].reg_name, false);
		if (!r)
        {
			LOG_ERROR("Couldn't find register named '%s'", reg_params[i].reg_name);
			return ERROR_FAIL;
		}

		if (r->size != reg_params[i].size)
        {
			LOG_ERROR("Register %s is %d bits instead of %d bits.", reg_params[i].reg_name, r->size, reg_params[i].size);
			return ERROR_FAIL;
		}

		if (r->number > GDB_REGNO_XPR31)
        {
			LOG_ERROR("Only GPRs can be use as argument registers.");
			return ERROR_FAIL;
		}

		if (r->type->get(r) != ERROR_OK)
			return ERROR_FAIL;
		saved_regs[r->number] = buf_get_u64(r->value, 0, r->size);

		if (reg_params[i].direction == PARAM_OUT || reg_params[i].direction == PARAM_IN_OUT)
        {
			if (r->type->set(r, reg_params[i].value) != ERROR_OK)
				return ERROR_FAIL;
		}
	}

	/* Disable Interrupts before attempting to run the algorithm. */
	uint64_t current_mstatus;
	uint8_t mstatus_bytes[8] = { 0 };

	LOG_DEBUG("Disabling Interrupts");
	struct reg *reg_mstatus = register_get_by_name(target->reg_cache, "mstatus", true);
	if (!reg_mstatus)
    {
		LOG_ERROR("Couldn't find mstatus!");
		return ERROR_FAIL;
	}

	reg_mstatus->type->get(reg_mstatus);
	current_mstatus = buf_get_u64(reg_mstatus->value, 0, reg_mstatus->size);
	uint64_t ie_mask = MSTATUS_MIE | MSTATUS_HIE | MSTATUS_SIE | MSTATUS_UIE;
	buf_set_u64(mstatus_bytes, 0, info->xlen, set_field(current_mstatus, ie_mask, 0));

	reg_mstatus->type->set(reg_mstatus, mstatus_bytes);

	/* Run algorithm */
	LOG_DEBUG("resume at 0x%" TARGET_PRIxADDR, entry_point);
	if (riscv_resume(target, 0, entry_point, 0, 0, true) != ERROR_OK)
		return ERROR_FAIL;

	int64_t start = timeval_ms();
	while (target->state != TARGET_HALTED)
    {
		LOG_DEBUG("poll()");
		int64_t now = timeval_ms();
		if (now - start > timeout_ms)
        {
			LOG_ERROR("Algorithm timed out after %" PRId64 " ms.", now - start);
			riscv_halt(target);
			old_or_new_riscv_poll(target);
			enum gdb_regno regnums[] =
            {
				GDB_REGNO_RA, GDB_REGNO_SP, GDB_REGNO_GP, GDB_REGNO_TP,
				GDB_REGNO_T0, GDB_REGNO_T1, GDB_REGNO_T2, GDB_REGNO_FP,
				GDB_REGNO_S1, GDB_REGNO_A0, GDB_REGNO_A1, GDB_REGNO_A2,
				GDB_REGNO_A3, GDB_REGNO_A4, GDB_REGNO_A5, GDB_REGNO_A6,
				GDB_REGNO_A7, GDB_REGNO_S2, GDB_REGNO_S3, GDB_REGNO_S4,
				GDB_REGNO_S5, GDB_REGNO_S6, GDB_REGNO_S7, GDB_REGNO_S8,
				GDB_REGNO_S9, GDB_REGNO_S10, GDB_REGNO_S11, GDB_REGNO_T3,
				GDB_REGNO_T4, GDB_REGNO_T5, GDB_REGNO_T6,
				GDB_REGNO_PC,
				GDB_REGNO_MSTATUS, GDB_REGNO_MEPC, GDB_REGNO_MCAUSE,
			};

			for (unsigned i = 0; i < ARRAY_SIZE(regnums); i++)
            {
				enum gdb_regno regno = regnums[i];
				riscv_reg_t reg_value;
				if (riscv_get_register(target, &reg_value, regno) != ERROR_OK)
					break;
				LOG_ERROR("%s = 0x%" PRIx64, gdb_regno_name(regno), reg_value);
			}
			return ERROR_TARGET_TIMEOUT;
		}

		int result = old_or_new_riscv_poll(target);
		if (result != ERROR_OK)
			return result;
	}

	/* The current hart id might have been changed in poll(). */
	if (riscv_select_current_hart(target) != ERROR_OK)
		return ERROR_FAIL;

	if (reg_pc->type->get(reg_pc) != ERROR_OK)
		return ERROR_FAIL;

	uint64_t final_pc = buf_get_u64(reg_pc->value, 0, reg_pc->size);
	if (exit_point && final_pc != exit_point)
    {
		LOG_ERROR("PC ended up at 0x%" PRIx64 " instead of 0x%" TARGET_PRIxADDR, final_pc, exit_point);
		return ERROR_FAIL;
	}

	/* Restore Interrupts */
	LOG_DEBUG("Restoring Interrupts");
	buf_set_u64(mstatus_bytes, 0, info->xlen, current_mstatus);
	reg_mstatus->type->set(reg_mstatus, mstatus_bytes);

	/* Restore registers */
	uint8_t buf[8] = { 0 };
	buf_set_u64(buf, 0, info->xlen, saved_pc);
	if (reg_pc->type->set(reg_pc, buf) != ERROR_OK)
		return ERROR_FAIL;

	for (int i = 0; i < num_reg_params; i++)
    {
		if (reg_params[i].direction == PARAM_IN || reg_params[i].direction == PARAM_IN_OUT)
        {
			struct reg *r = register_get_by_name(target->reg_cache, reg_params[i].reg_name, false);
			if (r->type->get(r) != ERROR_OK)
            {
				LOG_ERROR("get(%s) failed", r->name);
				return ERROR_FAIL;
			}
			buf_cpy(r->value, reg_params[i].value, reg_params[i].size);
		}

		LOG_DEBUG("restore %s", reg_params[i].reg_name);
		struct reg *r = register_get_by_name(target->reg_cache, reg_params[i].reg_name, false);
		buf_set_u64(buf, 0, info->xlen, saved_regs[r->number]);
		if (r->type->set(r, buf) != ERROR_OK)
        {
			LOG_ERROR("set(%s) failed", r->name);
			return ERROR_FAIL;
		}
	}

	return ERROR_OK;
}


const struct command_registration riscv_command_handlers[] =
{
	{
		.name = "riscv",
		.mode = COMMAND_ANY,
		.help = "RISC-V Command Group",
		.usage = "",
		.chain = riscv_exec_command_handlers
	},

	{
		.name = "arm",
		.mode = COMMAND_ANY,
		.help = "ARM Command Group",
		.usage = "",
		.chain = semihosting_common_handlers
	},
	COMMAND_REGISTRATION_DONE
};


static unsigned riscv_xlen_nonconst(struct target *target)
{
	return riscv_xlen(target);
}


static unsigned int riscv_data_bits(struct target *target)
{
	RISCV_INFO(r);
	if (r->data_bits)
		return r->data_bits(target);
	return riscv_xlen(target);
}


struct target_type riscv_target =
{
	.name = "riscv",

	.target_create = riscv_create_target,
	.init_target = riscv_init_target,
	.deinit_target = riscv_deinit_target,
	.examine = riscv_examine,

	/* poll current target status */
	.poll = old_or_new_riscv_poll,
	.halt = riscv_halt,
	.resume = riscv_target_resume,
	.step = old_or_new_riscv_step,

	.assert_reset = riscv_assert_reset,
	.deassert_reset = riscv_deassert_reset,

	.read_memory = riscv_read_memory,
	.write_memory = riscv_write_memory,
	.read_phys_memory = riscv_read_phys_memory,
	.write_phys_memory = riscv_write_phys_memory,

	.checksum_memory = riscv_checksum_memory,

	.mmu = riscv_mmu,
	.virt2phys = riscv_virt2phys,

	.get_gdb_arch = riscv_get_gdb_arch,
	.get_gdb_reg_list = riscv_get_gdb_reg_list,
	.get_gdb_reg_list_noread = riscv_get_gdb_reg_list_noread,

	.add_breakpoint = riscv_add_breakpoint,
	.remove_breakpoint = riscv_remove_breakpoint,
	.add_watchpoint = riscv_add_watchpoint,
	.remove_watchpoint = riscv_remove_watchpoint,
	.hit_watchpoint = riscv_hit_watchpoint,

	.arch_state = riscv_arch_state,

	.run_algorithm = riscv_run_algorithm,

	.commands = riscv_command_handlers,

	.address_bits = riscv_xlen_nonconst,
	.data_bits = riscv_data_bits
};