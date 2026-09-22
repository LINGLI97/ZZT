MF= Makefile
CC= g++

CFLAGS= -D_USE_64 -O3 -fopenmp -fomit-frame-pointer -funroll-loops -msse3
OPT_CFLAGS= -O3 -fopenmp -fomit-frame-pointer -funroll-loops -msse3 -DUSE_LIBSAIS_ZZ
SDSL_ROOT ?= ../third_party/sdsl-install
LIBSAIS_ROOT ?= ../vendor/zigzag-libsais
LIBSAIS_OBJS= $(LIBSAIS_ROOT)/libsais.o $(LIBSAIS_ROOT)/libsais64.o
LFLAGS= -std=c++17 \
		-I $(SDSL_ROOT)/include \
		-L $(SDSL_ROOT)/lib -lsdsl -ldivsufsort -ldivsufsort64

LCCS_LFLAGS= $(LFLAGS) -I ./librmqo/include/ -L ./librmqo/lib/ -lrmqo

.DEFAULT_GOAL := all

.PHONY: all CC LCCS LFCS TCPR-TF TCPR-TP TCPR-SP TopK_Freq TopK_TP TopK_SP \
        optimized optimized-core optimized-topk test-optimized-backend \
        clean_CC clean_LCCS clean_LFCS clean_TCPR-TF clean_TCPR-TP clean_TCPR-SP \
        clean_TopK_Freq clean_TopK_TP clean_TopK_SP clean_optimized clean

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CC) $(CFLAGS) -c $(LFLAGS) $<


# ─── Shared object dependencies ──────────────────────────────────────────────
SA_LCP_LCE.o:          SA_LCP_LCE.cpp          SA_LCP_LCE.h $(MF)
compactTrie_CC.o:      compactTrie_CC.cpp      compactTrie_CC.h node_CC.h $(MF)
node_CC.o:             node_CC.cpp             node_CC.h $(MF)
compactTrie_LCCS.o:    compactTrie_LCCS.cpp    compactTrie_LCCS.h node_LCCS.h $(MF)
node_LCCS.o:           node_LCCS.cpp           node_LCCS.h $(MF)
compactTrie_LFCS.o:    compactTrie_LFCS.cpp    compactTrie_LFCS.h node_LFCS.h $(MF)
node_LFCS.o:           node_LFCS.cpp           node_LFCS.h $(MF)
compactTrieTruncated.o: compactTrieTruncated.cpp compactTrieTruncated.h $(MF)
esaTruncated.o:        esaTruncated.cpp        esaTruncated.h $(MF)
esaTruncated_TP.o:     esaTruncated_TP.cpp     esaTruncated_TP.h $(MF)
esaTruncated_GAP.o:    esaTruncated_GAP.cpp    esaTruncated_GAP.h $(MF)


# ─── Optimized libsais/radix backend, separate 32/64-bit executables ─────────
# Existing targets above/below remain unchanged and continue to use the original
# merge-sort constructor.  Optimized binaries always have an _opt32/_opt64 suffix.
OPT_BACKEND_SRC= zigzag_libsais_backend.cpp
OPT_BACKEND_HD=  zigzag_libsais_backend.h $(LIBSAIS_ROOT)/zigzag.cpp

$(LIBSAIS_OBJS):
	$(MAKE) -C $(LIBSAIS_ROOT) $(notdir $@)

define OPTIMIZED_TARGETS
$(1)_opt32: $(2) $(OPT_BACKEND_SRC) $(OPT_BACKEND_HD) $(LIBSAIS_OBJS)
	$(CC) $(OPT_CFLAGS) -D_USE_32 -o $$@ $(2) $(OPT_BACKEND_SRC) $(LFLAGS) $(LIBSAIS_OBJS)

$(1)_opt64: $(2) $(OPT_BACKEND_SRC) $(OPT_BACKEND_HD) $(LIBSAIS_OBJS)
	$(CC) $(OPT_CFLAGS) -D_USE_64 -o $$@ $(2) $(OPT_BACKEND_SRC) $(LFLAGS) $(LIBSAIS_OBJS)
endef

$(eval $(call OPTIMIZED_TARGETS,run_CC,CC.cpp compactTrie_CC.cpp node_CC.cpp SA_LCP_LCE.cpp))
$(eval $(call OPTIMIZED_TARGETS,run_LFCS,LFCS.cpp compactTrie_LFCS.cpp node_LFCS.cpp SA_LCP_LCE.cpp))
$(eval $(call OPTIMIZED_TARGETS,run_LCCS,LCCS.cpp compactTrie_LCCS.cpp node_LCCS.cpp SA_LCP_LCE.cpp rmq-offline-local.cpp))
$(eval $(call OPTIMIZED_TARGETS,run_TopK_SA_Truncated_Freq,topK_SA_Truncated.cpp compactTrieTruncated.cpp esaTruncated.cpp SA_LCP_LCE.cpp))
$(eval $(call OPTIMIZED_TARGETS,run_TopK_SA_Truncated_TP,topK_SA_Truncated_TP.cpp compactTrieTruncated.cpp esaTruncated_TP.cpp SA_LCP_LCE.cpp))
$(eval $(call OPTIMIZED_TARGETS,run_TopK_SA_Truncated_SP,topK_SA_Truncated_GAP.cpp compactTrieTruncated.cpp esaTruncated_GAP.cpp SA_LCP_LCE.cpp))

optimized-core: run_CC_opt32 run_CC_opt64 run_LFCS_opt32 run_LFCS_opt64 \
		run_LCCS_opt32 run_LCCS_opt64

optimized-topk: run_TopK_SA_Truncated_Freq_opt32 run_TopK_SA_Truncated_Freq_opt64 \
		run_TopK_SA_Truncated_TP_opt32 run_TopK_SA_Truncated_TP_opt64 \
		run_TopK_SA_Truncated_SP_opt32 run_TopK_SA_Truncated_SP_opt64

optimized: optimized-core optimized-topk

test-optimized-backend: zigzag_libsais_backend_test
	OMP_NUM_THREADS=2 ./zigzag_libsais_backend_test

zigzag_libsais_backend_test: zigzag_libsais_backend_test.cpp $(OPT_BACKEND_SRC) $(OPT_BACKEND_HD) $(LIBSAIS_OBJS)
	$(CC) $(OPT_CFLAGS) -o $@ zigzag_libsais_backend_test.cpp $(OPT_BACKEND_SRC) \
		-std=c++17 $(LIBSAIS_OBJS)

clean_optimized:
	rm -f run_CC_opt32 run_CC_opt64 run_LFCS_opt32 run_LFCS_opt64 \
		run_LCCS_opt32 run_LCCS_opt64 \
		run_TopK_SA_Truncated_Freq_opt32 run_TopK_SA_Truncated_Freq_opt64 \
		run_TopK_SA_Truncated_TP_opt32 run_TopK_SA_Truncated_TP_opt64 \
		run_TopK_SA_Truncated_SP_opt32 run_TopK_SA_Truncated_SP_opt64 \
		zigzag_libsais_backend_test


# ─── CC (Contextual Complexity) ───────────────────────────────────────────────
CC_EXE= run_CC
CC_SRC= CC.cpp SA_LCP_LCE.cpp compactTrie_CC.cpp node_CC.cpp
CC_HD=  SA_LCP_LCE.h compactTrie_CC.h node_CC.h cmdline.h Makefile
CC_OBJ= $(CC_SRC:.cpp=.o)

CC: $(CC_EXE)

$(CC_EXE): $(CC_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CC_OBJ) $(LFLAGS)

CC.o: $(CC_HD)

clean_CC:
	rm -f $(CC_OBJ) $(CC_EXE)


# ─── LCCS (Longest Common Contextual Superstring) ─────────────────────────────
LCCS_EXE= run_LCCS
LCCS_SRC= LCCS.cpp SA_LCP_LCE.cpp compactTrie_LCCS.cpp node_LCCS.cpp
LCCS_HD=  SA_LCP_LCE.h compactTrie_LCCS.h node_LCCS.h cmdline.h rmq-offline.h Makefile
LCCS_OBJ= $(LCCS_SRC:.cpp=.o)

LCCS: $(LCCS_EXE)

$(LCCS_EXE): $(LCCS_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LCCS_OBJ) $(LCCS_LFLAGS)

LCCS.o: $(LCCS_HD)

clean_LCCS:
	rm -f $(LCCS_OBJ) $(LCCS_EXE)


# ─── LFCS (Longest Frequent Contextual String) ────────────────────────────────
LFCS_EXE= run_LFCS
LFCS_SRC= LFCS.cpp SA_LCP_LCE.cpp compactTrie_LFCS.cpp node_LFCS.cpp
LFCS_HD=  SA_LCP_LCE.h compactTrie_LFCS.h node_LFCS.h cmdline.h Makefile
LFCS_OBJ= $(LFCS_SRC:.cpp=.o)

LFCS: $(LFCS_EXE)

$(LFCS_EXE): $(LFCS_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LFCS_OBJ) $(LFLAGS)

LFCS.o: $(LFCS_HD)

clean_LFCS:
	rm -f $(LFCS_OBJ) $(LFCS_EXE)


# ─── TCPR-TF ──────────────────────────────────────────────────────────────────
TopK_Freq_EXE= run_TopK_SA_Truncated_Freq
TopK_Freq_SRC= topK_SA_Truncated.cpp SA_LCP_LCE.cpp compactTrieTruncated.cpp esaTruncated.cpp
TopK_Freq_HD=  SA_LCP_LCE.h compactTrieTruncated.h esaTruncated.h cmdline.h Makefile
TopK_Freq_OBJ= $(TopK_Freq_SRC:.cpp=.o)

TCPR-TF: $(TopK_Freq_EXE)
TopK_Freq: $(TopK_Freq_EXE)

$(TopK_Freq_EXE): $(TopK_Freq_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TopK_Freq_OBJ) $(LFLAGS)

topK_SA_Truncated.o: $(TopK_Freq_HD)

clean_TopK_Freq:
	rm -f $(TopK_Freq_OBJ) $(TopK_Freq_EXE)

clean_TCPR-TF: clean_TopK_Freq


# ─── TCPR-TP ──────────────────────────────────────────────────────────────────
TopK_TP_EXE= run_TopK_SA_Truncated_TP
TopK_TP_SRC= topK_SA_Truncated_TP.cpp SA_LCP_LCE.cpp compactTrieTruncated.cpp esaTruncated_TP.cpp
TopK_TP_HD=  SA_LCP_LCE.h compactTrieTruncated.h esaTruncated_TP.h cmdline.h Makefile
TopK_TP_OBJ= $(TopK_TP_SRC:.cpp=.o)

TCPR-TP: $(TopK_TP_EXE)
TopK_TP: $(TopK_TP_EXE)

$(TopK_TP_EXE): $(TopK_TP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TopK_TP_OBJ) $(LFLAGS)

topK_SA_Truncated_TP.o: $(TopK_TP_HD)

clean_TopK_TP:
	rm -f $(TopK_TP_OBJ) $(TopK_TP_EXE)

clean_TCPR-TP: clean_TopK_TP


# ─── TCPR-SP ──────────────────────────────────────────────────────────────────
TopK_SP_EXE= run_TopK_SA_Truncated_SP
TopK_SP_SRC= topK_SA_Truncated_GAP.cpp SA_LCP_LCE.cpp compactTrieTruncated.cpp esaTruncated_GAP.cpp
TopK_SP_HD=  SA_LCP_LCE.h compactTrieTruncated.h esaTruncated_GAP.h cmdline.h Makefile
TopK_SP_OBJ= $(TopK_SP_SRC:.cpp=.o)

TCPR-SP: $(TopK_SP_EXE)
TopK_SP: $(TopK_SP_EXE)

$(TopK_SP_EXE): $(TopK_SP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TopK_SP_OBJ) $(LFLAGS)

topK_SA_Truncated_GAP.o: $(TopK_SP_HD)

clean_TopK_SP:
	rm -f $(TopK_SP_OBJ) $(TopK_SP_EXE) run_TopK_SA_Truncated_GAP

clean_TCPR-SP: clean_TopK_SP


# ─── all / clean ──────────────────────────────────────────────────────────────
all: CC LCCS LFCS TopK_Freq TopK_TP TopK_SP

clean: clean_CC clean_LCCS clean_LFCS clean_TopK_Freq clean_TopK_TP clean_TopK_SP
	rm -f *.o *~
