all:
	g++ -c -I../GcodeProcessorLib/ inverse_processor.cpp
	g++ -o ArcWelderInverseProcessor -I../GcodeProcessorLib/ ../GcodeProcessorLib/*.o inverse_processor.o ArcWelderInverseProcessor.cpp 

clean:
	rm -f *.o ArcWelderInverseProcessor
