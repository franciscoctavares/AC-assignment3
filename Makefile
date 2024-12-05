fractal: clean fractal.c fractalfuncs.c
	gcc -c fractalfuncs.c
	gcc -c fractal.c
	gcc -fopenmp -o fractal fractal.o fractalfuncs.o
	chmod 755 genmovie
convert:
	for file in imgs/normal/*.pgm; do \
		convert "$$file" "$${file%.pgm}.png"; \
		rm "$$file"; \
	done
	for file in imgs/difusion/*.pgm; do \
		convert "$$file" "$${file%.pgm}.png"; \
		rm "$$file"; \
	done
	convert julia.pgm julia_normal.png
	rm julia.pgm
clean:
	rm -f *.png fractal imgs/difusion/* imgs/normal/* julia*.p*