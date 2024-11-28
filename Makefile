fractal: clean fractal.c fractalfuncs.c
	gcc -g fractalfuncs.c fractal.c -o fractal
	chmod 755 genmovie
convert:
	for file in imgs/*.pgm; do \
		convert "$$file" "$${file%.pgm}.png"; \
		rm "$$file"; \
	done
clean:
	rm -f *.png fractal imgs/*