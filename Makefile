include settings.mk

TEST_OBJS =
TEST_OBJS_MIC =  

TEST_TARGET = bin/pheet_test
TEST_TARGET_MIC = bin_mic/pheet_test

include test/sub.mk

lib/%.o : test/%.cpp test/settings.h
	$(CXX) -c $(CXXFLAGS) $(INCLUDE_PATH) -o $@ $<
 
lib_mic/%.o : test/%.cpp test/settings.h
	$(CXX) -c -mmic $(CXXFLAGS_MIC) $(INCLUDE_PATH_MIC) -o $@ $<


$(TEST_TARGET):	$(TEST_OBJS)
	$(CXX) -o $(TEST_TARGET) $(TEST_OBJS) $(LIB_PATH) $(LIBS)

$(TEST_TARGET_MIC):	$(TEST_OBJS_MIC)
	$(CXX) -mmic -o $(TEST_TARGET_MIC) $(TEST_OBJS_MIC) $(LIB_PATH_MIC) $(LIBS)
  	
all:		$(TEST_TARGET)
all_mic:	$(TEST_TARGET) $(TEST_TARGET_MIC)

mic: $(TEST_TARGET_MIC)

test:	$(TEST_TARGET)

clean:
	rm -f $(OBJS) $(TARGET) $(TEST_OBJS) $(TEST_OBJS_MIC) $(TEST_TARGET) $(TEST_TARGET_MIC)
	rm -f -R inc/pheet/*
