
#version 330 core

in  vec2 TexCoords;
in  vec2 Position;
out vec4 color;

uniform sampler2D tile_image;
uniform sampler2D tile_map;
uniform vec4      map_info;

void main()
{    
    float tileSize      = map_info.x;
	float mapWidth      = map_info.y;
	vec2  tilePos       = TexCoords / tileSize;
	float index         = floor(texture2D(tile_map, tilePos).r * 255);
	vec2  baseTilePos   = 0.5 * floor(vec2(mod(index, 2), index / 2.0)); 
	vec2  tileTexCoords = 0.5 * mod(TexCoords * mapWidth, 1);
	
	color =  texture2D(tile_image, baseTilePos + tileTexCoords); 
}
