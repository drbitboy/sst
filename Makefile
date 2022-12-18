all: sst

sst: sst.c sst.h stty_info.h raw_settings.h
	$(CC) $(CPPFLAGS) -o sst -Wall sst.c

clean:
	$(RM) sst
