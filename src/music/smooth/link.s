ram{start=$000,end=$1C0}
ram{start=$200,end=$800}
copy{file=ines.hdr}
# Bank 0
bank{size=$4000,origin=$8000}
link{file=build/song.o}
# Bank 1
bank{size=$4000,origin=$C000}
link{file=build/bitmasktable.o}
link{file=build/periodtable.o}
link{file=build/volumetable.o}
link{file=build/envelope.o}
link{file=build/effect.o}
link{file=build/tonal.o}
link{file=build/dmc.o}
link{file=build/mixer.o}
link{file=build/sequencer.o}
link{file=build/sound.o}
link{file=build/sprite.o}
link{file=build/tablecall.o}
link{file=build/ppu.o}
link{file=build/ppuwrite.o}
link{file=build/ppubuffer.o}
link{file=build/joypad.o}
link{file=build/timer.o}
link{file=build/irq.o}
link{file=build/nmi.o}
link{file=build/reset.o}
link{file=build/main.o}
link{file=build/songtable.o}
pad{origin=$FB00}
link{file=build/dmcdata.o}
pad{origin=$FFFA}
link{file=build/vectors.o}
# CHR banks
bank{size=$2000}
copy{file=graphics.chr}
