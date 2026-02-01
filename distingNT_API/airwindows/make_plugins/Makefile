
inputs := $(wildcard int/*cpp)
bin := $(patsubst int/%.cpp,bin/%,$(inputs))
reports := $(patsubst int/%.cpp,reports/%.txt,$(inputs))

bin: $(bin)

reports: $(reports)

clean:
	rm -rf bin int reports

bin/%: int/%.cpp
	mkdir -p $(@D)
	g++ -I. -o $@ $^

reports/%.txt: bin/%
	mkdir -p $(@D)
	$^ > $@
