CXXFLAGS = -std=c++11 -I$(BOOST_ROOT)/include -I..
LDFLAGS  = -L$(BOOST_ROOT)/lib -lboost_unit_test_framework

run: run_test_kbd_reader

run_test_kbd_reader: test_kbd_reader
	./test_kbd_reader

test_kbd_reader: test_kbd_reader.cpp ../kbd_reader.hpp requre_boost
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $<

requre_boost:
	@test "$(BOOST_ROOT)" -a -d "$(BOOST_ROOT)" || \
	    echo "BOOST_ROOT environment variable is required."

clean:
	rm -f test_kbd_reader
