410KLIB_MALLOC_OBJS:= \
                        calloc.o		\
                        free.o			\
                        malloc.o		\
                        malloc_lmm.o	\
                        memalign.o		\
                        realloc.o		\
                        sfree.o			\
                        smalloc.o		\
                        smemalign.o


410KLIB_MALLOC_OBJS:= $(410KLIB_MALLOC_OBJS:%=$(410KDIR)/malloc/%)

ALL_410KOBJS += $(410KLIB_MALLOC_OBJS)
410KCLEANS += $(410KDIR)/libmalloc.a

$(410KDIR)/libmalloc.a: $(410KLIB_MALLOC_OBJS)
