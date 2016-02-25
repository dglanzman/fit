fit: fit.c
	cc -o fit `pkg-config --libs gsl` fit.c

clean:
	rm -f fit
