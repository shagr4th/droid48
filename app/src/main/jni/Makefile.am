
## Process this file with automake to produce Makefile.in

bin_PROGRAMS = x48 dump2rom mkcard checkrom

BITMAPS = bitmaps/A.h bitmaps/B.h bitmaps/C.h bitmaps/D.h bitmaps/E.h bitmaps/F.h \
          bitmaps/G.h bitmaps/H.h bitmaps/I.h bitmaps/J.h bitmaps/K.h \
          bitmaps/L.h bitmaps/M.h bitmaps/N.h bitmaps/O.h bitmaps/P.h \
          bitmaps/Q.h bitmaps/R.h bitmaps/S.h bitmaps/T.h bitmaps/U.h \
          bitmaps/V.h bitmaps/W.h bitmaps/X.h bitmaps/Y.h bitmaps/Z.h \
          bitmaps/alpha.h bitmaps/angle.h bitmaps/angle_gx.h \
          bitmaps/ann_alpha.h bitmaps/ann_battery.h bitmaps/ann_busy.h \
          bitmaps/ann_io.h bitmaps/ann_left.h bitmaps/ann_right.h \
          bitmaps/arrow.h bitmaps/arrow_gx.h bitmaps/blank.h \
          bitmaps/bracket_gx.h bitmaps/bs.h bitmaps/colon.h \
          bitmaps/colon_gx.h bitmaps/comma.h bitmaps/comma_gx.h \
          bitmaps/curly_gx.h bitmaps/diff.h bitmaps/div.h bitmaps/down.h \
          bitmaps/equal.h bitmaps/equal_gx.h bitmaps/exp.h \
          bitmaps/exp_gx.h bitmaps/gx_128K_ram.h bitmaps/gx_green.h \
          bitmaps/gx_silver.h bitmaps/hash.h bitmaps/hash_gx.h \
          bitmaps/hp.h bitmaps/hp48_blue.h bitmaps/hp48_bottom.h \
          bitmaps/hp48_disp.h bitmaps/hp48_green_gx.h \
          bitmaps/hp48_icon.h bitmaps/hp48_keys.h bitmaps/hp48_logo.h \
          bitmaps/hp48_logo_gx.h bitmaps/hp48_on.h bitmaps/hp48_orange.h \
          bitmaps/hp48_text.h bitmaps/hp48_text_gx.h bitmaps/hp48_top.h \
          bitmaps/hp48_top_gx.h bitmaps/hp48gx.h bitmaps/hp48sx.h \
          bitmaps/integral.h bitmaps/inv.h bitmaps/last.h \
          bitmaps/lbrace.h bitmaps/lbracket.h bitmaps/lcurly.h \
          bitmaps/left.h bitmaps/menu_label.h bitmaps/minus.h \
          bitmaps/mul.h bitmaps/neg.h bitmaps/nl.h bitmaps/nl_gx.h \
          bitmaps/parens_gx.h bitmaps/pi.h bitmaps/pi_gx.h \
          bitmaps/plus.h bitmaps/pow10.h bitmaps/pow10_gx.h \
          bitmaps/power.h bitmaps/prog.h bitmaps/prog_gx.h \
          bitmaps/quote_gx.h bitmaps/rbrace.h bitmaps/rbracket.h \
          bitmaps/rcurly.h bitmaps/right.h bitmaps/root.h \
          bitmaps/root_gx.h bitmaps/science.h bitmaps/shl.h \
          bitmaps/shr.h bitmaps/sigma.h bitmaps/slash.h \
          bitmaps/small_colon.h bitmaps/sqr.h bitmaps/sqr_gx.h \
          bitmaps/sqrt.h bitmaps/string.h bitmaps/three.h bitmaps/two.h \
          bitmaps/under.h bitmaps/under_gx.h bitmaps/up.h

x48_SOURCES = actions.c debugger.c device.c \
              disasm.c emulate.c errors.c init.c \
              lcd.c main.c memory.c options.c register.c \
              resources.c romio.c rpl.c serial.c timer.c \
              x48_x11.c \
              annunc.h append.h buttons.h constants.h debugger.h \
              device.h disasm.h errors.h global.h hp48char.h hp48_emu.h \
              hp48.h hp.h icon.h mmu.h options.h resources.h romio.h rpl.h \
              small.h timer.h version.h X48.ad.h x48_x11.h \
	      $(BITMAPS)

dump2rom_SOURCES = dump2rom.c

mkcard_SOURCES = mkcard.c

checkrom_SOURCES = checkrom.c romio.c

man1_MANS = x48.man

EXTRA_DIST = $(man1_MANS)

INCLUDES = $(X_CFLAGS)

x48_LDADD = $(X_LIBS)
