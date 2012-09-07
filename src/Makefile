include ../Makefile.inc

DIRS	= PTU_forwarder_TCP_RS232
EXE	= PTU_forwarder_TCP_RS232

all : $(EXE)

PTU_forwarder_TCP_RS232 : force_look
		$(ECHO) looking into PTU_forwarder_TCP_RS232 : $(MAKE) $(MFLAGS)
		cd PTU_forwarder_TCP_RS232; $(MAKE) $(MFLAGS)

clean : force_look
		$(ECHO) cleaning up in PTU_forwarder_TCP_RS232: $(MAKE) $(MFLAGS) clean
		cd PTU_forwarder_TCP_RS232; $(MAKE) $(MFLAGS) clean
#		-$(RM) -f $(EXE) $(OBJS) $(OBJLIBS)
#		-for d in $(DIRS); do (cd $$d; $(MAKE) clean ); done

install : force_look
		$(ECHO) looking into PTU_forwarder_TCP_RS232 : $(MAKE) $(MFLAGS) install
		cd PTU_forwarder_TCP_RS232; $(MAKE) $(MFLAGS) install
		
force_look :
		true
.PHONY: all clean install
