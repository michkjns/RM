
#pragma once

#include <includes.h>
#include <bitstream.h>

class Serialize
{
public:
	static void serializePosition(BitStream* stream, Vector2 pos);
	static Vector2 deserializePosition(BitStream* stream);

	static void serializeAngle(BitStream* stream, float angle);
	static float deserializeAngle(BitStream* stream);

};