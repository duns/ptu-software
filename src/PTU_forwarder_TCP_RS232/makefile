################################################################################
# Automatically-generated file. Do not edit!
################################################################################

INSTALL?=install
TARGET_CC_ARCH += "${LDFLAGS}" 

RM := rm -rf

C_SRCS += \
./PTU_forwarder.c \
./json.c \
./rs232.c \
./timer.c 

OBJS += \
./PTU_forwarder.o \
./json.o \
./rs232.o \
./timer.o 

C_DEPS += \
./PTU_forwarder.d \
./json.d \
./rs232.d \
./timer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	${CC} -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# All of the sources participating in the build are defined here

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif


# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: PTU_forwarder_TCP_RS232

# Tool invocations
PTU_forwarder_TCP_RS232: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	${CC} -o "PTU_forwarder_TCP_RS232" $(OBJS) $(USER_OBJS) $(LIBS) ${LDFLAGS}
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) PTU_forwarder_TCP_RS232
	-@echo ' '


install: PTU_forwarder_TCP_RS232
	$(INSTALL) -d ${DESTDIR}/bin/
	$(INSTALL) -d ${DESTDIR}/etc/init.d/
   
	$(INSTALL) -m 755 PTU_forwarder_TCP_RS232 ${DESTDIR}/bin/
	$(INSTALL) -m 755 ptu_forwarder ${DESTDIR}/etc/init.d/
	#$(INSTALL) -m 755 ptu_forwarder.conf ${DESTDIR}/etc/ptu_forwarder.conf

	

.PHONY: all clean dependents install
.SECONDARY:

