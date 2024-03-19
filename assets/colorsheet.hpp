COLOR texdata_colorsheet[] = {
0xb016, 0xf81f, 0xd81b, 
    0x72e7, 0xa42a, 0x8b88, 
    0x738e, 0xa514, 0x8c51, 
    0x4a49, 0x6b6d, 0x5acb, 
    0x528a, 0x73ae, 0x632c, 
    0xb016, 0xf81f, 0xd81b, 
    0xb016, 0xf81f, 0xd81b, 
    0xb016, 0xf81f, 0xd81b, 
    };

constexpr const TEXTURE tex_colorsheet = {
    .width = 3,
    .height = 8,
    .has_transparency = false,
    .transparent_color = 0x0000,
    .bitmap = texdata_colorsheet
};

