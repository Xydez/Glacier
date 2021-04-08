#pragma once

namespace glacier
{
	namespace math
	{
		inline float clamp(float value, float min, float max)
		{
			if (value < min)
				return min;

			if (value > max)
				return max;

			return value;
		}

		inline float abs(float value)
		{
			if (value < 0)
				return -value;

			return value;
		}

		inline float lerp(float t, float start, float end)
		{
			return start + t * (end - start);
		}
	}
}
