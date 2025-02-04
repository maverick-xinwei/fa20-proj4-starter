CC = gcc
CFLAGS = -m64 -g -Wall -std=c99 -fopenmp -mavx -mfma -pthread
LDFLAGS = -fopenmp
#CUNIT = -L/home/ff/cs61c/cunit/install/lib -I/home/ff/cs61c/cunit/install/include -lcunit

#PYTHON = -I/usr/include/python3.6 -lpython3.6m
PYTHON = -I/arm/tools/python/python/3.6.8/rhe7-x86_64/include/python3.6m -L/arm/tools/python/python/3.6.8/rhe7-x86_64/lib -lpython3.6m
CUNIT = -I/usr/local/CUnit/include -L/usr/local/lib -lcunit

install:
	if [ ! -f files.txt ]; then touch files.txt; fi
	rm -rf build
	xargs rm -rf < files.txt
	python3 setup.py install --record files.txt

uninstall:
	if [ ! -f files.txt ]; then touch files.txt; fi
	rm -rf build
	xargs rm -rf < files.txt

clean:
	rm -f *.o
	rm -f test
	rm -rf build
	rm -rf __pycache__

test:
	rm -f test
	$(CC) $(CFLAGS) mat_test.c matrix.c -o test $(LDFLAGS) $(CUNIT) $(PYTHON)
	./test

.PHONY: test
