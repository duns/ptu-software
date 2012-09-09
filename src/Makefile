include ../Makefile.inc

SUBDIRS	= PTU_forwarder_TCP_RS232 commandserver
.PHONY: all clean install subdirs $(SUBDIRS)
TARGET=asd

all : subdirs

subdirs : $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) $(MFLAGS) -C $@ 


clean : 
	for dir in $(SUBDIRS); \
	do \
	$(MAKE) $(MFLAGS) -C $$dir clean; \
	done

install : 
	for dir in $(SUBDIRS); \
	do \
	$(MAKE) $(MFLAGS) -C $$dir install; \
	done
		
