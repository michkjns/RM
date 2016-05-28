
#include <core/serialize.h>

static const bool s_enableCompression = false;

void Serialize::serializePosition(BitStream* stream, Vector2 pos)
{
	if (s_enableCompression)
	{
	}
	else
	{
		stream->writeFloat(pos.x);
		stream->writeFloat(pos.y);
	}
}

Vector2 Serialize::deserializePosition(BitStream* stream)
{
	Vector2 pos;
	if (s_enableCompression)
	{
	}
	else
	{
		pos.x = stream->readFloat();
		pos.y = stream->readFloat();
	}

	return pos;
}

void Serialize::serializeAngle(BitStream* stream, float angle)
{
	if (s_enableCompression)
	{
	}
	else
	{
		stream->writeFloat(angle);
	}
}

float Serialize::deserializeAngle(BitStream* stream)
{
	if (s_enableCompression)
	{
	}
	else
	{
		return stream->readFloat();
	}
}
