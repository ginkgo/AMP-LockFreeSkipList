
TEST_OBJS += lib/pheet_tests.o lib/Test.o
TEST_OBJS_MIC += lib_mic/pheet_tests.o lib_mic/Test.o

include test/ds_stress/sub.mk
include test/graph_bipartitioning/sub.mk
include test/sorting/sub.mk
include test/inarow/sub.mk
include test/n-queens/sub.mk
include test/lupiv/sub.mk
include test/uts/sub.mk
#include test/sor/sub.mk
include test/prefix_sum/sub.mk
include test/set_bench/sub.mk
include test/count_bench/sub.mk
include test/sssp/sub.mk
include test/tristrip/sub.mk
