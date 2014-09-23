CXXFLAGS = -std=c++11 -I$(BOOST_ROOT)/include -I..
LDFLAGS  = -L$(BOOST_ROOT)/lib -lboost_unit_test_framework

run: run_test_bounce_filter run_test_buffer


run_test_bounce_filter: test_bounce_filter
	./test_bounce_filter

test_bounce_filter: requre_boost test_bounce_filter.cpp ../bounce_filter.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o test_bounce_filter test_bounce_filter.cpp


run_test_buffer: test_buffer
	./test_buffer

test_buffer: requre_boost test_buffer.cpp ../buffer.hpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o test_buffer test_buffer.cpp


requre_boost:
	@test "$(BOOST_ROOT)" -a -d "$(BOOST_ROOT)" || \
	    echo "BOOST_ROOT environment variable is required."

clean:
	rm -f test_bounce_filter test_buffer
