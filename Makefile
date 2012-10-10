include Makefile.inc

DIRS	= src
LIBS	= -L. -lsub -lsuba -lsubsub

all : src

boot.scr :
	cd conf; mkimage -T script -C none -n 'WPSS phase2 FireSTORM' -d WPSS-FireSTORM-bootcmds.txt boot.scr
src : force_look
		$(ECHO) looking into src: $(MAKE) $(MFLAGS)
		cd src; $(MAKE) $(MFLAGS)

clean : force_look
		$(ECHO) cleaning up in src : $(MAKE) $(MFLAGS) clean
		cd src; $(MAKE) $(MFLAGS) clean
#		-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
#		-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

install :
	$(ECHO) looking into src: $(MAKE) $(MFLAGS)
	cd src; $(MAKE) $(MFLAGS) install
		
force_look :
		true

.PHONY: all install src clean
