fractal: clean fractal.c fractalfuncs.c
	gcc -c fractalfuncs.c
	gcc -c -fopenmp fractal.c
	gcc -o fractal fractal.o fractalfuncs.o
	chmod 755 genmovie
convert:
	for file in imgs/*.pgm; do \
		convert "$$file" "$${file%.pgm}.png"; \
		rm "$$file"; \
	done
clean:
	rm -f *.png fractal imgs/*