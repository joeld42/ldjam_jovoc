
fips_begin_app(hello-sokol windowed)
    fips_files(hello-sokol.cpp)
    fipsutil_copy(spritetest_gamedata.yml)
    fips_deps(ldjam stb)
fips_end_app()

fips_begin_app(ldjam_spritetest windowed)
    fips_files(ldjam_spritetest.cpp)
    fipsutil_copy(spritetest_gamedata.yml)
    fips_deps(ldjam stb)
fips_end_app()

