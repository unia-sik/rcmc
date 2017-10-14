regfile_ram_inst : regfile_ram PORT MAP (
		data	 => data_sig,
		rdaddress	 => rdaddress_sig,
		rdclock	 => rdclock_sig,
		rdclocken	 => rdclocken_sig,
		wraddress	 => wraddress_sig,
		wrclock	 => wrclock_sig,
		wrclocken	 => wrclocken_sig,
		wren	 => wren_sig,
		q	 => q_sig
	);
