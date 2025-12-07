
all: demo_application

libparallel_scheduler.so: parallel_scheduler.cpp parallel_scheduler.h
	g++ -std=c++11 -Wall -fPIC -pthread -shared -o $@ parallel_scheduler.cpp

demo_application: demo_application.cpp libparallel_scheduler.so
	g++ -std=c++11 -Wall -pthread -o demo_application demo_application.cpp \
		-lparallel_scheduler -L. -Wl,-rpath,.

run: demo_application
	./demo_application

clean:
	rm -f demo_application libparallel_scheduler.so 

.PHONY: all clean run

