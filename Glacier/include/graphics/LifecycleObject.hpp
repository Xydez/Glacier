#pragma once

namespace glacier
{
	class LifecycleObject
	{
	public:
		virtual void create() = 0;
		virtual void destroy() = 0;
	};
}
