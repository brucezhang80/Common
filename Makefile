PROGRAMS = steve_common \
       evan_common 

all:
	for PROGRAM in $(PROGRAMS); do { cd $$PROGRAM; $(MAKE); cd ..; } done   
clean:  
	for PROGRAM in $(PROGRAMS); do { cd $$PROGRAM; $(MAKE) clean; cd ..; } done
test:   
	for PROGRAM in $(PROGRAMS); do { cd $$PROGRAM; $(MAKE) test; cd ..; } done
