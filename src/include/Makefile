T=$(shell bash -c 'printf "%(%Y%m%d-%H)T" -1')

all:

zip:
	@mkdir -p 1tmp
	zip 1tmp/$(shell bash -c 'printf "%(%Y%m%d-%H)T" -1').zip $(shell find . -maxdepth 1 -a -type f)

back:
	@mkdir -p $T
	cp * $T

