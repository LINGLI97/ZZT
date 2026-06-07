MF= Makefile
CC= g++

CFLAGS= -D_USE_64 -O3 -fopenmp -fomit-frame-pointer -funroll-loops -msse3
LFLAGS= -std=c++17 \
        -I ./libsdsl/include/ -I /usr/include/boost \
        -L ./libsdsl/lib/ -lsdsl -ldivsufsort -ldivsufsort64

LCCS_LFLAGS= $(LFLAGS) -I ./librmqo/include/ -L ./librmqo/lib/ -lrmqo

.DEFAULT_GOAL := all

.PHONY: all CC LCCS LFCS TCPR-TF TCPR-TP TCPR-SP TopK_Freq TopK_TP TopK_SP \
        clean_CC clean_LCCS clean_LFCS clean_TCPR-TF clean_TCPR-TP clean_TCPR-SP \
        clean_TopK_Freq clean_TopK_TP clean_TopK_SP clean

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
