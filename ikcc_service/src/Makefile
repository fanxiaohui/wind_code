#############################################################
# root Makefile
#############################################################

SUBDIRS = build

SDKDIR = sdk
 
all:
	@for subdir in $(SUBDIRS);	\
	do \
		echo "Making $@ in $$subdir"; \
		(cd $$subdir && make -f Makefile $@); \
	done
ikcc:
	echo "Making $@ in build";
	cd build && make -f Makefile;
	
 
.PHONY: clean
 
clean:
	@for subdir in $(SUBDIRS);	\
	do \
		echo "Clean in $$subdir"; \
		(cd $$subdir && make clean); \
	done