410K_LMM_OBJS := \
                 lmm_add_free.o \
                 lmm_add_region.o \
                 lmm_alloc.o \
                 lmm_alloc_aligned.o \
                 lmm_alloc_gen.o \
                 lmm_alloc_page.o \
                 lmm_avail.o \
                 lmm_dump.o \
                 lmm_find_free.o \
                 lmm_free.o \
                 lmm_free_page.o \
                 lmm_init.o \
                 lmm_remove_free.o \

410K_LMM_OBJS := $(410K_LMM_OBJS:%=$(410KDIR)/lmm/%)

ALL_410KOBJS += $(410K_LMM_OBJS)
410KCLEANS += $(410KDIR)/liblmm.a

$(410KDIR)/liblmm.a: $(410K_LMM_OBJS)
