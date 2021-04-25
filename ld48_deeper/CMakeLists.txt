

fips_begin_app(ld48_deeper windowed)
    fips_files(
		../basecode/emsc.h 
    	../basecode/sokol_debugtext.h
    	../basecode/sokol_fetch.h
    	../basecode/sokol_gfx.h
    	../basecode/sokol_gl.h
    	../basecode/sokol_time.h
    	
    	tk_sprite.cpp
    	tk_sprite.h

    	tk_tilemap.h

    	ld48_deeper.cpp
    	)
    fipsutil_copy(ld48deeper_gamedata.yml)
    fips_deps(stb)
fips_end_app()