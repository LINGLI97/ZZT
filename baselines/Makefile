MF=     Makefile

CC=     g++

CFLAGS=  -D_USE_64 -g -fopenmp -fomit-frame-pointer -funroll-loops -msse3 -O3
LFLAGS= -std=c++17 -I ./libsdsl/include/ -I /usr/include/boost -I ./include/ -L ./libsdsl/lib/ -lsdsl -ldivsufsort -ldivsufsort64 -Wl,-rpath=$(PWD)/libsdsl/lib
LCCS_LFLAGS= $(LFLAGS) -I ./librmqo/include/ -L ./librmqo/lib/ -lrmqo

.DEFAULT_GOAL := all

.PHONY: all LFCS_BA LCCS_BA_ST CC_BA baseline baseline_TP baseline_SP \
        clean_LFCS_BA clean_LCCS_BA_ST clean_CC_BA clean_baseline clean_baseline_TP clean_baseline_SP clean

.SUFFIXES:
.SUFFIXES: .cpp .o

.cpp.o:
	$(CC) $(CFLAGS) -c $(LFLAGS) $<


# ─── Shared object dependencies ──────────────────────────────────────────────
SA_LCP_LCE.o: SA_LCP_LCE.cpp SA_LCP_LCE.h $(MF)
suffixTree.o: suffixTree.cpp suffixTree.h stNode.h $(MF)
stNode.o:     stNode.cpp     stNode.h $(MF)


# ─── LFCS baseline ────────────────────────────────────────────────────────────
LFCS_BA_EXE= run_LFCS_BA
LFCS_BA_SRC= LFCS_BA.cpp SA_LCP_LCE.cpp suffixTree.cpp stNode.cpp
LFCS_BA_HD=  SA_LCP_LCE.h suffixTree.h cmdline.h stNode.h $(MF)
LFCS_BA_OBJ= $(LFCS_BA_SRC:.cpp=.o)

LFCS_BA: $(LFCS_BA_EXE)

$(LFCS_BA_EXE): $(LFCS_BA_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LFCS_BA_OBJ) $(LFLAGS)

LFCS_BA.o: $(LFCS_BA_HD)

clean_LFCS_BA:
	rm -f $(LFCS_BA_OBJ) $(LFCS_BA_EXE) *~


# ─── LCCS baseline ────────────────────────────────────────────────────────────
LCCS_BA_ST_EXE= run_LCCS_BA
LCCS_BA_ST_SRC= LCCS_BA_ST.cpp suffixTree.cpp stNode.cpp
LCCS_BA_ST_HD=  suffixTree.h stNode.h cmdline.h rmq-offline.h $(MF)
LCCS_BA_ST_OBJ= $(LCCS_BA_ST_SRC:.cpp=.o)

LCCS_BA_ST: $(LCCS_BA_ST_EXE)

$(LCCS_BA_ST_EXE): $(LCCS_BA_ST_OBJ)
	$(CC) $(CFLAGS) -o $@ $(LCCS_BA_ST_OBJ) $(LCCS_LFLAGS)

LCCS_BA_ST.o: $(LCCS_BA_ST_HD)

clean_LCCS_BA_ST:
	rm -f $(LCCS_BA_ST_OBJ) $(LCCS_BA_ST_EXE) *~


# ─── CC baseline ──────────────────────────────────────────────────────────────
CC_BA_EXE= run_CC_BA
CC_BA_SRC= CC_BA.cpp SA_LCP_LCE.cpp suffixTree.cpp stNode.cpp
CC_BA_HD=  SA_LCP_LCE.h suffixTree.h stNode.h cmdline.h $(MF)
CC_BA_OBJ= $(CC_BA_SRC:.cpp=.o)

CC_BA: $(CC_BA_EXE)

$(CC_BA_EXE): $(CC_BA_OBJ)
	$(CC) $(CFLAGS) -o $@ $(CC_BA_OBJ) $(LFLAGS)

CC_BA.o: $(CC_BA_HD)

clean_CC_BA:
	rm -f $(CC_BA_OBJ) $(CC_BA_EXE) *~


# ─── TCPR-TF baseline ─────────────────────────────────────────────────────────
baseline_EXE= run_baseline_topK
baseline_SRC= baseline.cpp
baseline_HD=  baseline.h cmdline.h $(MF)
baseline_OBJ= $(baseline_SRC:.cpp=.o)

baseline: $(baseline_EXE)

$(baseline_EXE): $(baseline_OBJ)
	$(CC) $(CFLAGS) -o $@ $(baseline_OBJ) $(LFLAGS)

baseline.o: $(baseline_HD)

clean_baseline:
	rm -f $(baseline_OBJ) $(baseline_EXE) *~


# ─── TCPR-TP baseline ─────────────────────────────────────────────────────────
baseline_TP_EXE= run_baseline_TP_topK
baseline_TP_SRC= baseline_TP.cpp
baseline_TP_HD=  baseline_TP.h cmdline.h $(MF)
baseline_TP_OBJ= $(baseline_TP_SRC:.cpp=.o)

baseline_TP: $(baseline_TP_EXE)

$(baseline_TP_EXE): $(baseline_TP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(baseline_TP_OBJ) $(LFLAGS)

baseline_TP.o: $(baseline_TP_HD)

clean_baseline_TP:
	rm -f $(baseline_TP_OBJ) $(baseline_TP_EXE) *~


# ─── TCPR-SP baseline ─────────────────────────────────────────────────────────
baseline_SP_EXE= run_baseline_SP_topK
baseline_SP_SRC= baseline_GAP.cpp
baseline_SP_HD=  baseline_GAP.h cmdline.h $(MF)
baseline_SP_OBJ= $(baseline_SP_SRC:.cpp=.o)

baseline_SP: $(baseline_SP_EXE)

$(baseline_SP_EXE): $(baseline_SP_OBJ)
	$(CC) $(CFLAGS) -o $@ $(baseline_SP_OBJ) $(LFLAGS)

baseline_GAP.o: $(baseline_SP_HD)

clean_baseline_SP:
	rm -f $(baseline_SP_OBJ) $(baseline_SP_EXE) run_baseline_GAP_topK *~


# ─── all / clean ──────────────────────────────────────────────────────────────
all: LFCS_BA LCCS_BA_ST CC_BA baseline baseline_TP baseline_SP

clean: clean_LFCS_BA clean_LCCS_BA_ST clean_CC_BA clean_baseline clean_baseline_TP clean_baseline_SP
	rm -f *.o *~
