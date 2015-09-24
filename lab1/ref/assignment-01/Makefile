CFLAGS?=-std=c99 -D_GNU_SOURCE -Wall -O2

default: all

all: bin bin/montecarlo bin/montecarlo_parallel

report: report.pdf

bin:
	mkdir bin

bin/montecarlo bin/montecarlo_parallel: src/montecarlo.c
	$(CC) $(CFLAGS) -o bin/montecarlo $<
	$(CC) $(CFLAGS) -DPARALLEL -pthread -o bin/montecarlo_parallel $<

report.pdf: report/report.tex
	cd report && pdflatex report.tex && pdflatex report.tex
	mv report/report.pdf report.pdf

clean:
	rm -r bin
