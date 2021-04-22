
include_directories(stb)
add_subdirectory( stb )

fips_begin_lib(ldjam)
    fips_files(
    	emsc.h 
    	sokol_debugtext.h
    	sokol_fetch.h
    	sokol_gfx.h
    	sokol_gl.h
    	sokol_time.h
    	tk_sprite.cpp
    	tk_sprite.h
		)

fips_end_lib(ldjam)
