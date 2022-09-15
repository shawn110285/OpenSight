
struct riscv_info
{
	unsigned int common_magic;

	unsigned dtm_version;

	struct command_context *cmd_ctx;
	void *version_specific;

	/* The hart that is currently being debugged.  Note that this is
	 * different than the hartid that the RTOS is expected to use.  This
	 * one will change all the time, it's more of a global argument to
	 * every function than an actual */
	int current_hartid;

	/* Single buffer that contains all register names, instead of calling
	 * malloc for each register. Needs to be freed when reg_list is freed. */
	char *reg_names;

	/* It's possible that each core has a different supported ISA set. */
	int xlen;
	riscv_reg_t misa;
	/* Cached value of vlenb. 0 if vlenb is not readable for some reason. */
	unsigned int vlenb;

	/* The number of triggers per hart. */
	unsigned int trigger_count;

	/* For each physical trigger, contains -1 if the hwbp is available, or the
	 * unique_id of the breakpoint/watchpoint that is using it.
	 * Note that in RTOS mode the triggers are the same across all harts the
	 * target controls, while otherwise only a single hart is controlled. */
	int trigger_unique_id[RISCV_MAX_HWBPS];

	/* The number of entries in the debug buffer. */
	int debug_buffer_size;

	/* This hart contains an implicit ebreak at the end of the program buffer. */
	bool impebreak;

	bool triggers_enumerated;

	/* Decremented every scan, and when it reaches 0 we clear the learned
	 * delays, causing them to be relearned. Used for testing. */
	int reset_delays_wait;

	/* This target has been prepped and is ready to step/resume. */
	bool prepped;
	/* This target was selected using hasel. */
	bool selected;

	/* Helper functions that target the various RISC-V debug spec
	 * implementations. */
	int (*get_register)(struct target *target, riscv_reg_t *value, int regid);
	int (*set_register)(struct target *target, int regid, uint64_t value);
	int (*get_register_buf)(struct target *target, uint8_t *buf, int regno);
	int (*set_register_buf)(struct target *target, int regno,
			const uint8_t *buf);
	int (*select_current_hart)(struct target *target);
	bool (*is_halted)(struct target *target);
	/* Resume this target, as well as every other prepped target that can be
	 * resumed near-simultaneously. Clear the prepped flag on any target that
	 * was resumed. */
	int (*resume_go)(struct target *target);
	int (*step_current_hart)(struct target *target);
	int (*on_halt)(struct target *target);
	/* Get this target as ready as possible to resume, without actually
	 * resuming. */
	int (*resume_prep)(struct target *target);
	int (*halt_prep)(struct target *target);
	int (*halt_go)(struct target *target);
	int (*on_step)(struct target *target);
	enum riscv_halt_reason (*halt_reason)(struct target *target);
	int (*write_debug_buffer)(struct target *target, unsigned index,
			riscv_insn_t d);
	riscv_insn_t (*read_debug_buffer)(struct target *target, unsigned index);
	int (*execute_debug_buffer)(struct target *target);
	int (*dmi_write_u64_bits)(struct target *target);
	void (*fill_dmi_write_u64)(struct target *target, char *buf, int a, uint64_t d);
	void (*fill_dmi_read_u64)(struct target *target, char *buf, int a);
	void (*fill_dmi_nop_u64)(struct target *target, char *buf);

	int (*authdata_read)(struct target *target, uint32_t *value, unsigned int index);
	int (*authdata_write)(struct target *target, uint32_t value, unsigned int index);

	int (*dmi_read)(struct target *target, uint32_t *value, uint32_t address);
	int (*dmi_write)(struct target *target, uint32_t address, uint32_t value);

	int (*test_sba_config_reg)(struct target *target, target_addr_t legal_address,
			uint32_t num_words, target_addr_t illegal_address, bool run_sbbusyerror_test);

	int (*sample_memory)(struct target *target,
						 struct riscv_sample_buf *buf,
						 riscv_sample_config_t *config,
						 int64_t until_ms);

	int (*read_memory)(struct target *target, target_addr_t address,
			uint32_t size, uint32_t count, uint8_t *buffer, uint32_t increment);

	/* How many harts are attached to the DM that this target is attached to? */
	int (*hart_count)(struct target *target);
	unsigned (*data_bits)(struct target *target);

	COMMAND_HELPER((*print_info), struct target *target);

	/* Storage for vector register types. */
	struct reg_data_type_vector vector_uint8;
	struct reg_data_type_vector vector_uint16;
	struct reg_data_type_vector vector_uint32;
	struct reg_data_type_vector vector_uint64;
	struct reg_data_type_vector vector_uint128;
	struct reg_data_type type_uint8_vector;
	struct reg_data_type type_uint16_vector;
	struct reg_data_type type_uint32_vector;
	struct reg_data_type type_uint64_vector;
	struct reg_data_type type_uint128_vector;
	struct reg_data_type_union_field vector_fields[5];
	struct reg_data_type_union vector_union;
	struct reg_data_type type_vector;

	/* Set when trigger registers are changed by the user. This indicates we eed
	 * to beware that we may hit a trigger that we didn't realize had been set. */
	bool manual_hwbp_set;

	/* Memory access methods to use, ordered by priority, highest to lowest. */
	int mem_access_methods[RISCV_NUM_MEM_ACCESS_METHODS];

	/* Different memory regions may need different methods but single configuration is applied
	 * for all. Following flags are used to warn only once about failing memory access method. */
	bool mem_access_progbuf_warn;
	bool mem_access_sysbus_warn;
	bool mem_access_abstract_warn;

	/* In addition to the ones in the standard spec, we'll also expose additional
	 * CSRs in this list. */
	struct list_head expose_csr;
	/* Same, but for custom registers.
	 * Custom registers are for non-standard extensions and use abstract register numbers
	 * from range 0xc000 ... 0xffff. */
	struct list_head expose_custom;

	riscv_sample_config_t sample_config;
	struct riscv_sample_buf sample_buf;
};
