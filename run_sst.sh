########################################################################
### run_sst
### - script to run Serial Stress Test (sst)
###
###     Usage:  source run_sst
### Alternate: . run_sst
###
### The last --speed= and --send-count= command-line options below are
### the ones that will be used; the duplicate settings here are for
### convenience i.e. by reordering those lines with a simple Yank-Paste
### (vi) the speed and character count settings can be easily changed.

./sst --tty=/dev/ttyTHS0 \
      --speed=115200 --send-count=115200 \
      --speed=12.5M --send-count=12500000 \
      --speed=4M --send-count=4000000 \
      --speed=8M --send-count=8000000 \
      --open-non-blocking \
      --do-raw-config \
      --raw-config-debug-speed \
      --debug \

########################################################################
### List of various command-line options, for copy-paste to above
### [cat ... /dev/null] is to make script ignore this

cat << EoF > /dev/null

      --open-non-blocking \
      --fork-reader \

      --speed=12.5M --send-count=12500000 \
      --speed=12M --send-count=12000000 \
      --speed=8M --send-count=8000000 \
      --speed=4M --send-count=4000000 \
      --speed=115200 --send-count=115200 \

      --do-raw-config \

      --raw-config-debug \
      --raw-config-debug-speed \
      --debug \

      --dump-to-send=dump-to-send.txt \
      --dump-raw-settings=dump-raw-settings.txt \

EoF
