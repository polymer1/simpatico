BLD_DIR_REL =../../..
include $(BLD_DIR_REL)/config.mk
include $(BLD_DIR)/mcMd/config.mk
include $(BLD_DIR)/inter/config.mk
include $(BLD_DIR)/util/config.mk
include $(SRC_DIR)/inter/patterns.mk
include $(SRC_DIR)/mcMd/sources.mk
include $(SRC_DIR)/inter/sources.mk
include $(SRC_DIR)/util/sources.mk
include $(SRC_DIR)/mcMd/tests/mdIntegrators/sources.mk

all: $(mcMd_tests_mdIntegrators_OBJS) Test

clean:
	rm -f $(mcMd_tests_mdIntegrators_OBJS)
	rm -f $(mcMd_tests_mdIntegrators_OBJS:.o=.d)
	rm -f Test

Test: Test.o $(mcMd_LIB) $(inter_LIB) $(util_LIB)
	$(CXX) $(LDFLAGS) $(INCLUDES) $(DEFINES) -o Test Test.o \
	       $(mcMd_LIB) $(inter_LIB) $(util_LIB)

-include $(mcMd_tests_mdIntegrators_OBJS:.o=.d)

