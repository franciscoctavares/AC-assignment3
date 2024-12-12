fractal_omp: clean fractal_omp.c fractalfuncs.c
	gcc -fopenmp -g fractalfuncs.c fractal_omp.c -o fractal_omp
	chmod 755 genmovie
fractal: clean fractal.c fractalfuncs.c 
	gcc -g fractalfuncs.c fractal.c -o fractal
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
	rm -f fractal fractal_omp imgs/difusion/* imgs/normal/* imgs/*.pgm imgs/*.png