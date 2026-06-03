MF= Makefile
CC= g++

CFLAGS= -D_USE_64 -O3 -fopenmp -fomit-frame-pointer -funroll-loops -msse3
LFLAGS= -std=c++17 \
        -I ./libsdsl/include/ -I /usr/include/boost \
        -L ./libsdsl/lib/ -lsdsl -ldivsufsort -ldivsufsort64

LCCS_BA_LFLAGS= $(LFLAGS) -I ./librmqo/include/ -L ./librmqo/lib/ -lrmqo

.PHONY: all CC_BA LCCS_BA LFCS_BA baseline_Freq baseline_TP baseline_GAP \
        clean_CC_BA clean_LCCS_BA clean_LFCS_BA \
        clean_baseline_Freq clean_baseline_TP clean_baseline_GAP clean

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CC) $(CFLAGS) -c $(LFLAGS) $<


# ─── Shared object dependencies ──────────────────────────────────────────────
SA_LCP_LCE.o:  SA_LCP_LCE.cpp  SA_LCP_LCE.h $(MF)
suffixTree.o:  suffixTree.cpp  suffixTree.h stNode.h $(MF)
stNode.o:      stNode.cpp      stNode.h $(MF)


# ─── CC_BA (Contextual Complexity baseline) ───────────────────────────────────
CC_BA_EXE= run_CC_BA
CC_BA_SRC= CC_BA.cpp SA_LCP_LCE.cpp suffixTree.cpp stNode.cpp
CC_BA_HD=  SA_LCP_LCE.h suffixTree.h stNode.h cmdline.h Makefile
CC_BA_OBJ= $(CC_BA_SRC:.cpp=.o)

CC_BA: $(CC_BA_EXE)

$(CC_BA_EXE): $(CC_BA_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CC_BA_OBJ) $(LFLAGS)

CC_BA.o: $(CC_BA_HD)

clean_CC_BA:
	rm -f $(CC_BA_OBJ) $(CC_BA_EXE)


# ─── LCCS_BA (LCCS baseline — suffix tree + RMQ) ─────────────────────────────
LCCS_BA_EXE= run_LCCS_BA
LCCS_BA_SRC= LCCS_BA_ST.cpp suffixTree.cpp stNode.cpp
LCCS_BA_HD=  suffixTree.h stNode.h cmdline.h rmq-offline.h Makefile
LCCS_BA_OBJ= $(LCCS_BA_SRC:.cpp=.o)

LCCS_BA: $(LCCS_BA_EXE)

$(LCCS_BA_EXE): $(LCCS_BA_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LCCS_BA_OBJ) $(LCCS_BA_LFLAGS)

LCCS_BA_ST.o: $(LCCS_BA_HD)

clean_LCCS_BA:
	rm -f $(LCCS_BA_OBJ) $(LCCS_BA_EXE)


# ─── LFCS_BA (LFCS baseline — suffix tree) ────────────────────────────────────
LFCS_BA_EXE= run_LFCS_BA
LFCS_BA_SRC= LFCS_BA.cpp SA_LCP_LCE.cpp suffixTree.cpp stNode.cpp
LFCS_BA_HD=  SA_LCP_LCE.h suffixTree.h stNode.h cmdline.h Makefile
LFCS_BA_OBJ= $(LFCS_BA_SRC:.cpp=.o)

LFCS_BA: $(LFCS_BA_EXE)

$(LFCS_BA_EXE): $(LFCS_BA_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LFCS_BA_OBJ) $(LFLAGS)

LFCS_BA.o: $(LFCS_BA_HD)

clean_LFCS_BA:
	rm -f $(LFCS_BA_OBJ) $(LFCS_BA_EXE)


# ─── baseline_Freq (TopK Frequency baseline) ──────────────────────────────────
baseline_Freq_EXE= run_baseline_topK
baseline_Freq_SRC= baseline.cpp
baseline_Freq_HD=  baseline.h cmdline.h Makefile
baseline_Freq_OBJ= $(baseline_Freq_SRC:.cpp=.o)

baseline_Freq: $(baseline_Freq_EXE)

$(baseline_Freq_EXE): $(baseline_Freq_OBJ)
	$(CC) $(CFLAGS) -o $@ $(baseline_Freq_OBJ) $(LFLAGS)

baseline.o: $(baseline_Freq_HD)

clean_baseline_Freq:
	rm -f $(baseline_Freq_OBJ) $(baseline_Freq_EXE)


# ─── baseline_TP (TopK Term Proximity baseline) ───────────────────────────────
baseline_TP_EXE= run_baseline_TP_topK
baseline_TP_SRC= baseline_TP.cpp
baseline_TP_HD=  baseline_TP.h cmdline.h Makefile
baseline_TP_OBJ= $(baseline_TP_SRC:.cpp=.o)

baseline_TP: $(baseline_TP_EXE)

$(baseline_TP_EXE): $(baseline_TP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(baseline_TP_OBJ) $(LFLAGS)

baseline_TP.o: $(baseline_TP_HD)

clean_baseline_TP:
	rm -f $(baseline_TP_OBJ) $(baseline_TP_EXE)


# ─── baseline_GAP (TopK Gap baseline) ─────────────────────────────────────────
baseline_GAP_EXE= run_baseline_GAP_topK
baseline_GAP_SRC= baseline_GAP.cpp
baseline_GAP_HD=  baseline_GAP.h cmdline.h Makefile
baseline_GAP_OBJ= $(baseline_GAP_SRC:.cpp=.o)

baseline_GAP: $(baseline_GAP_EXE)

$(baseline_GAP_EXE): $(baseline_GAP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(baseline_GAP_OBJ) $(LFLAGS)

baseline_GAP.o: $(baseline_GAP_HD)

clean_baseline_GAP:
	rm -f $(baseline_GAP_OBJ) $(baseline_GAP_EXE)


# ─── all / clean ──────────────────────────────────────────────────────────────
all: CC_BA LCCS_BA LFCS_BA baseline_Freq baseline_TP baseline_GAP

clean: clean_CC_BA clean_LCCS_BA clean_LFCS_BA \
       clean_baseline_Freq clean_baseline_TP clean_baseline_GAP
	rm -f *.o *~
