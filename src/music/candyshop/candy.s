ram{start=$000,end=$1C0}
ram{start=$200,end=$800}
output{file=candy.nes}
copy{file=candy.hdr}
# Bank 0
bank{size=$4000,origin=$8000}
link{file=candy.o}
# Bank 1
bank{size=$4000,origin=$C000}
link{file=bitmasktable.o}
link{file=periodtable.o}
link{file=volumetable.o}
link{file=envelope.o}
link{file=effect.o}
link{file=tonal.o}
link{file=dmc.o}
link{file=mixer.o}
link{file=sequencer.o}
link{file=sound.o}
link{file=sprite.o}
link{file=tablecall.o}
link{file=ppu.o}
link{file=ppuwrite.o}
link{file=ppubuffer.o}
link{file=joypad.o}
link{file=timer.o}
link{file=irq.o}
link{file=nmi.o}
link{file=reset.o}
link{file=main.o}
link{file=songtable.o}
pad{origin=$D000}
link{file=dmcdata.o}
pad{origin=$FFFA}
link{file=vectors.o}
# CHR banks
bank{size=$2000}
copy{file=candy.chr}
