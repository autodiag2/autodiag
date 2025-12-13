define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
endef
define filterout-multi
    $(filter-out $(strip $(1)), $(2))
endef