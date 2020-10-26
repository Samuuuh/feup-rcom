compile:
	gcc alarm.h const_defines.h llfunctions.h messages.h state_machines.h alarm.c llfunctions.c messages.c writenoncanonical.c state_machines.c -o writenoncanonical -lm
	gcc alarm.h const_defines.h llfunctions.h messages.h state_machines.h alarm.c llfunctions.c messages.c noncanonical.c state_machines.c -o noncanonical -lm

clean:
	rm -f writenoncanonical
	rm -f noncanonical