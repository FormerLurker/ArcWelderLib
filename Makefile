all:
	$(MAKE) -C GcodeProcessorLib all
	$(MAKE) -C ArcWelder all
	$(MAKE) -C ArcWelderConsole all
	$(MAKE) -C ArcWelderInverseProcessor all

clean:
	$(MAKE) -C GcodeProcessorLib clean
	$(MAKE) -C ArcWelder clean
	$(MAKE) -C ArcWelderConsole clean
	$(MAKE) -C ArcWelderInverseProcessor clean


cleanall: clean


.PHONY: all clean 
