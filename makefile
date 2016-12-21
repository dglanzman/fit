fit: fit.c
	cc -O2 -o fit `pkg-config --libs gsl` fit.c

debug: fit.c
	cc -g -O1 -o fit `pkg-config --libs gsl` fit.c

clean:
	rm -f fit
