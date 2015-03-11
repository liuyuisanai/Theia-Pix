BOOST_INCLUDE ?= $(BOOST_ROOT)/include
BOOST_LIB ?= $(BOOST_ROOT)/lib

CXXFLAGS = -std=c++11 -I$(BOOST_INCLUDE) -I../.. -DBOOST_TEST_DYN_LINK -flto -g -O0
LDFLAGS  = -L$(BOOST_LIB) -lboost_unit_test_framework -lboost_prg_exec_monitor -g

ifeq ($(shell uname),Darwin)
export DYLD_LIBRARY_PATH += $(BOOST_LIB)
endif

OBJ = $(subst .cpp,.o,main.cpp $(wildcard test_*.cpp))

run: test
	./test

test: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

test%.o: test%.cpp require_boost
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o test_bounce_filter test_bounce_filter.cpp

requre_boost:
	@test "$(BOOST_ROOT)" -a -d "$(BOOST_ROOT)" \
	|| echo "BOOST_ROOT environment variable is required."

clean:
	@rm -f test *.o
	@rm -rf *.dSYM
