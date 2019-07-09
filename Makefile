export ROOTDIR=$(CURDIR)
export ARCH_BITS=$(shell getconf LONG_BIT)

SOURCE_DIRS = common pb lbs lbs_client mds mds_client
BUILD_DIRS = bin lib

SOURCE_DIRS_CLEAN = $(addsuffix .clean,$(SOURCE_DIRS))
BUILD_DIRS_CLEAN = $(addsuffix .clean,$(BUILD_DIRS))

.PHONY: all check debug clean $(BUILD_DIRS) $(BUILD_DIRS_CLEAN) $(SOURCE_DIRS) $(SOURCE_DIRS_CLEAN)

all: export EXTRA_CFLAGS = -D__RELEASE__ -O3 -g3 -ggdb3 -DNDEBUG -Ofast
all: check $(BUILD_DIRS) $(SOURCE_DIRS)

asan: export EXTRA_CFLAGS = -D__DEBUG__ -DDEBUG -O0 -g3 -ggdb3 -fno-inline -fsanitize=address
asan: check $(BUILD_DIRS) $(SOURCE_DIRS)

debug: export EXTRA_CFLAGS = -D__DEBUG__ -DDEBUG -O0 -g3 -ggdb3 -fno-inline
debug: check $(BUILD_DIRS) $(SOURCE_DIRS)

clean: $(BUILD_DIRS_CLEAN) $(SOURCE_DIRS_CLEAN)

check:
	cppcheck --error-exitcode=0 -q . || exit 1

$(SOURCE_DIRS):
	$(MAKE) -C $@

$(BUILD_DIRS):
	mkdir -p $@

$(SOURCE_DIRS_CLEAN): %.clean:
	$(MAKE) -C $* clean

$(BUILD_DIRS_CLEAN): %.clean:
	rm -rf $*

common: $(BUILD_DIRS)

pb: common $(BUILD_DIRS)

lbs: common pb $(BUILD_DIRS)

lbs_client: common pb lbs $(BUILD_DIRS)

mds: common pb lbs lbs_client $(BUILD_DIRS)

mds_client: common pb lbs lbs_client mds $(BUILD_DIRS)
