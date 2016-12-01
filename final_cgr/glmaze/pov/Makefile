# create 3D player views

PLAYER_ICONS = \
	../data/smiley_r_icon.bmp \
	../data/smiley_g_icon.bmp \
	../data/smiley_b_icon.bmp \
	../data/smiley_c_icon.bmp \
	../data/smiley_y_icon.bmp \
	../data/smiley_m_icon.bmp

all: $(PLAYER_ICONS) ../data/radar.bmp

%_icon.bmp: %_icon.tga
	convert $< -compress none $@

%.png: %.bmp
	convert $< $@

%_icon.tga: %.png
	cp $< ./smiley.png
	povray ./smiley.ini -I smiley.pov +O $@
	rm -f ./smiley.png

../data/radar.bmp: radar.ini radar.pov
	povray ./radar.ini -I radar.pov +O ./radar.tga
	convert ./radar.tga -compress none $@
	rm -f radar.tga


