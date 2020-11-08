compile:
	gcc const_defines.h llfunctions.h messages.h state_machines.h llfunctions.c messages.c writenoncanonical.c state_machines.c -o writenoncanonical -lm
	gcc const_defines.h llfunctions.h messages.h state_machines.h llfunctions.c messages.c noncanonical.c state_machines.c -o noncanonical -lm

clean:
	rm -f writenoncanonical
	rm -f noncanonical